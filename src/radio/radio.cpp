#include "radio.h"
#include "sys.h"
#include "gpio.h"
#include "system.h"

using namespace RadioNS;

void Radio::toggleSpeaker(bool on) {
    speakerOn = on;
    if (on) {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    }
    else {
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    }
}

void Radio::setSquelch(uint32_t f, uint8_t sql) {
    //bk4819.squelch(sql, f, gSettings.sqlOpenTime, gSettings.sqlCloseTime);
    bk4819.squelch(sql, f, 1, 1);
}

void Radio::setVFO(uint8_t vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation) {
    radioVFO[vfo].rx.frequency = rx;
    radioVFO[vfo].tx.frequency = tx;
    radioVFO[vfo].channel = channel;
    radioVFO[vfo].modulation = modulation;
}

void Radio::setupToVFO(uint8_t vfo) {
    
    bk4819.squelchType(SquelchType::SQUELCH_RSSI_NOISE_GLITCH);
    setSquelch(radioVFO[vfo].rx.frequency, 4);

    bk4819.setModulation(radioVFO[vfo].modulation);
    
    bk4819.setAGC(radioVFO[vfo].modulation != ModType::MOD_AM, 18);
    bk4819.setFilterBandwidth(BK4819_Filter_Bandwidth::BK4819_FILTER_BW_12k);    

    bk4819.rxTurnOn();

    bk4819.tuneTo(radioVFO[vfo].rx.frequency, false);
    
}

void Radio::toggleBK4819(bool on) {
    if (on) {
        bk4819.toggleAFDAC(true);
        bk4819.toggleAFBit(true);
        delayMs(8);
        toggleSpeaker(true);
    }
    else {
        toggleSpeaker(false);
        delayMs(8);
        bk4819.toggleAFDAC(false);
        bk4819.toggleAFBit(false);
    }
}

void Radio::toggleRX(bool on) {
    bk4819.toggleGreen(on);
    toggleBK4819(on);
}

void Radio::checkRadioInterrupts(void) {

    while (bk4819.getInterruptRequest() & 1u) { // BK chip interrupt request
        // clear interrupts
        bk4819.clearInterrupt();
        // fetch interrupt status bits

        union {
            struct InterruptFlags {
                uint16_t __UNUSED : 1;
                uint16_t fskRxSync : 1;
                uint16_t sqlLost : 1;
                uint16_t sqlFound : 1;
                uint16_t voxLost : 1;
                uint16_t voxFound : 1;
                uint16_t ctcssLost : 1;
                uint16_t ctcssFound : 1;
                uint16_t cdcssLost : 1;
                uint16_t cdcssFound : 1;
                uint16_t cssTailFound : 1;
                uint16_t dtmf5ToneFound : 1;
                uint16_t fskFifoAlmostFull : 1;
                uint16_t fskRxFinied : 1;
                uint16_t fskFifoAlmostEmpty : 1;
                uint16_t fskTxFinied : 1;
            } flags;
            uint16_t __raw;
        } interrupts;

        interrupts.__raw = bk4819.readInterrupt();

        //uart.print("interrupts %0.16b \r\n", interrupts);

        if (interrupts.flags.sqlLost) {
            toggleRX(true);
            state = RadioState::RX_ON;
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_RX, 0);
        }

        if (interrupts.flags.sqlFound) {
            toggleRX(false);
            state = RadioState::IDLE;
        }

    }
}