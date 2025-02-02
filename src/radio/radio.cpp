#include <cstring>

#include "radio.h"
#include "sys.h"
#include "gpio.h"
#include "system.h"


using namespace RadioNS;

uint8_t Radio::convertRSSIToSLevel(int16_t rssi_dBm) {
    if (rssi_dBm <= -121) {
        return 0; // S0
    }
    else if (rssi_dBm <= -115) {
        return 1; // S1
    }
    else if (rssi_dBm <= -109) {
        return 2; // S2
    }
    else if (rssi_dBm <= -103) {
        return 3; // S3
    }
    else if (rssi_dBm <= -97) {
        return 4; // S4
    }
    else if (rssi_dBm <= -91) {
        return 5; // S5
    }
    else if (rssi_dBm <= -85) {
        return 6; // S6
    }
    else if (rssi_dBm <= -79) {
        return 7; // S7
    }
    else if (rssi_dBm <= -73) {
        return 8; // S8
    }
    else if (rssi_dBm <= -67) {
        return 9; // S9
    }
    else {
        return 10; // Greater than S9
    }
}

int16_t Radio::convertRSSIToPlusDB(int16_t rssi_dBm) {
    if (rssi_dBm > -67) {
        return (rssi_dBm + 67); // Convert to +dB value
    }
    return 0; // Return 0 if not greater than S9
}

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

void Radio::setVFO(Settings::VFOAB vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation) {
    uint8_t vfoIndex = (uint8_t)vfo;
    radioVFO[vfoIndex].rx.frequency = (uint32_t)(rx & 0x07FFFFFF);
    radioVFO[vfoIndex].tx.frequency = (uint32_t)(tx & 0x07FFFFFF);
    radioVFO[vfoIndex].channel = channel;
    radioVFO[vfoIndex].squelch = 1;
    radioVFO[vfoIndex].step = Settings::Step::STEP_12_5kHz;
    radioVFO[vfoIndex].modulation = modulation;
    radioVFO[vfoIndex].bw = BK4819_Filter_Bandwidth::BK4819_FILTER_BW_20k;
    radioVFO[vfoIndex].power = Settings::TXOutputPower::TX_POWER_LOW;
    radioVFO[vfoIndex].shift = Settings::OffsetDirection::OFFSET_NONE;
    radioVFO[vfoIndex].repeaterSte = Settings::ONOFF::OFF;
    radioVFO[vfoIndex].ste = Settings::ONOFF::OFF;
    radioVFO[vfoIndex].compander = Settings::TXRX::OFF;
    radioVFO[vfoIndex].pttid = 0;
    radioVFO[vfoIndex].rxagc = 18;
    radioVFO[vfoIndex].rx.codeType = Settings::CodeType::NONE;
    radioVFO[vfoIndex].rx.code = 0;
    radioVFO[vfoIndex].tx.codeType = Settings::CodeType::NONE;
    radioVFO[vfoIndex].tx.code = 0;

    if (channel > 0) {
        snprintf(radioVFO[vfoIndex].name, sizeof(radioVFO[vfoIndex].name), "CH-%03d", channel);
    }
    else {
        strncpy(radioVFO[vfoIndex].name, getBandName(rx), sizeof(radioVFO[vfoIndex].name) - 1);
        radioVFO[vfoIndex].name[sizeof(radioVFO[vfoIndex].name) - 1] = '\0'; // Ensure null termination
    }
}

void Radio::setupToVFO(Settings::VFOAB vfo) {
    uint8_t vfoIndex = (uint8_t)vfo;
    bk4819.squelchType(SquelchType::SQUELCH_RSSI_NOISE_GLITCH);
    setSquelch(radioVFO[vfoIndex].rx.frequency, 4);

    bk4819.setModulation(radioVFO[vfoIndex].modulation);

    // RX expander
    bk4819.setCompander((radioVFO[vfoIndex].modulation == ModType::MOD_FM && (uint8_t)(radioVFO[vfoIndex].compander) >= 2) ? (uint8_t)(radioVFO[vfoIndex].compander) : 0);

    bk4819.setAGC(radioVFO[vfoIndex].modulation != ModType::MOD_AM, radioVFO[vfoIndex].rxagc);
    bk4819.setFilterBandwidth(radioVFO[vfoIndex].bw);

    bk4819.rxTurnOn();

    setupToneDetection(vfo);

    bk4819.tuneTo(radioVFO[vfoIndex].rx.frequency, true);

}

void Radio::toggleBK4819(bool on) {
    //bk4819.rxTurnOn();

    if (on) {
        bk4819.toggleAFDAC(true);
        bk4819.toggleAFBit(true);
        //delayMs(5);
        //toggleSpeaker(true);
    }
    else {
        //toggleSpeaker(false);
        //delayMs(5);
        bk4819.toggleAFDAC(false);
        bk4819.toggleAFBit(false);
    }
}

void Radio::toggleRX(bool on, Settings::CodeType codeType = Settings::CodeType::NONE) {
    uint8_t vfoIndex = (uint8_t)getRXVFO();

    if (on) {
        if (state != Settings::RadioState::RX_ON) {
            bk4819.toggleGreen(true);
            toggleBK4819(true);
            state = Settings::RadioState::RX_ON;
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_RX, 0);
        }

        if (radioVFO[vfoIndex].modulation == ModType::MOD_FM &&
            (radioVFO[vfoIndex].rx.codeType == Settings::CodeType::DCS ||
                radioVFO[vfoIndex].rx.codeType == Settings::CodeType::NDCS ||
                radioVFO[vfoIndex].rx.codeType == Settings::CodeType::CT) && codeType == Settings::CodeType::NONE) {
            return;
        }
        toggleSpeaker(true);
    }
    else {
        if (state != Settings::RadioState::IDLE) {
            toggleSpeaker(false);
            bk4819.toggleGreen(false);
            toggleBK4819(false);
            state = Settings::RadioState::IDLE;
        }
    }
}

void Radio::playBeep(Settings::BEEPType beep) {
    bool isSpeakerWasOn = speakerOn;
    uint16_t toneConfig = bk4819.getToneRegister();

    // validate Radio state
    if (state != Settings::RadioState::IDLE) {
        return;
    }

    toggleSpeaker(false);
    delayMs(20);

    uint16_t toneFrequency;
    switch (beep)
    {
    default:
    case Settings::BEEPType::BEEP_NONE:
        toneFrequency = 220;
        break;
    case Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL:
        toneFrequency = 1000;
        break;
    case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
    case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP:
        toneFrequency = 500;
        break;
    case Settings::BEEPType::BEEP_440HZ_40MS_OPTIONAL:
    case Settings::BEEPType::BEEP_440HZ_500MS:
        toneFrequency = 440;
        break;
    case Settings::BEEPType::BEEP_880HZ_40MS_OPTIONAL:
    case Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP:
    case Settings::BEEPType::BEEP_880HZ_200MS:
    case Settings::BEEPType::BEEP_880HZ_500MS:
        toneFrequency = 880;
        break;
    }

    bk4819.playTone(toneFrequency, true);
    delayMs(2);
    toggleSpeaker(true);
    delayMs(60);

    uint16_t duration;
    switch (beep)
    {
    case Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP:
        bk4819.exitTxMute();
        delayMs(60);
        bk4819.enterTxMute();
        delayMs(20);
        [[fallthrough]];
    case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
    case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP:
        bk4819.exitTxMute();
        delayMs(60);
        bk4819.enterTxMute();
        delayMs(20);
        [[fallthrough]];
    case Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL:
        bk4819.exitTxMute();
        duration = 60;
        break;
    case Settings::BEEPType::BEEP_880HZ_40MS_OPTIONAL:
    case Settings::BEEPType::BEEP_440HZ_40MS_OPTIONAL:
        bk4819.exitTxMute();
        duration = 40;
        break;
    case Settings::BEEPType::BEEP_880HZ_200MS:
        bk4819.exitTxMute();
        duration = 200;
        break;
    case Settings::BEEPType::BEEP_440HZ_500MS:
    case Settings::BEEPType::BEEP_880HZ_500MS:
    default:
        bk4819.exitTxMute();
        duration = 500;
        break;
    }

    delayMs(duration);
    bk4819.enterTxMute();
    delayMs(20);
    toggleSpeaker(false);    
    delayMs(5);
    bk4819.turnsOffTonesTurnsOnRX();
    delayMs(5);
    bk4819.setToneRegister(toneConfig);

    toggleSpeaker(isSpeakerWasOn);
}


void Radio::runDualWatch(void) {
    if (dualWatch && state == Settings::RadioState::IDLE) {
        if (dualWatchTimer == 0) {
            dualWatchTimer = dualWatchTime;
        }
        else {
            dualWatchTimer--;
        }
        if (dualWatchTimer == 0) {
            setRXVFO((rxVFO == Settings::VFOAB::VFOA) ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA);
        }
    }
    else if (dualWatch && state == Settings::RadioState::RX_ON) {
        dualWatchTimer = dualWatchTime;
    }
}


void Radio::setupToneDetection(Settings::VFOAB vfo) {
    uint8_t vfoIndex = (uint8_t)vfo;

    uint16_t interruptMask = 0;

    /*if (dtmfdecode) {
      bk4819.enableDTMF();
    } else {
      bk4819.disableDTMF();
    }*/

    interruptMask |= BK4819_REG_3F_SQUELCH_FOUND | BK4819_REG_3F_SQUELCH_LOST | BK4819_REG_3F_DTMF_5TONE_FOUND;

    if (radioVFO[vfoIndex].modulation == ModType::MOD_FM) {

        switch (radioVFO[vfoIndex].rx.codeType) {
        case Settings::CodeType::DCS:
        case Settings::CodeType::NDCS:
            // Log("DCS on");
            bk4819.setCDCSSCodeWord(DCSGetGolayCodeWord(radioVFO[vfoIndex].rx.codeType, radioVFO[vfoIndex].rx.code));
            interruptMask |= BK4819_REG_3F_CDCSS_FOUND | BK4819_REG_3F_CDCSS_LOST;
            break;
        case Settings::CodeType::CT:
            // Log("CTCSS on");
            bk4819.setCTCSSFrequency(Settings::CTCSSOptions[radioVFO[vfoIndex].rx.code]);
            interruptMask |= BK4819_REG_3F_CTCSS_FOUND | BK4819_REG_3F_CTCSS_LOST;
            break;
        default:

            // Log("STE on"); ????
            if (radioVFO[vfoIndex].ste == Settings::ONOFF::ON) {
                bk4819.setCTCSSFrequency(550);
                bk4819.setTailDetection(550);
                interruptMask |= BK4819_REG_3F_CxCSS_TAIL;
            }
            
            break;
        }
    }

    bk4819.setInterrupt(interruptMask);
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

        //uart.print("%0.16b\n", interrupts);        

        /* if (interrupts.flags.fskRxFinied) {
             uart.sendLog("FSK RX Finished");
         }

         if (interrupts.flags.fskTxFinied) {
             uart.sendLog("FSK TX Finished");
         }

         if (interrupts.flags.fskFifoAlmostFull) {
             uart.sendLog("FSK FIFO Almost Full");
         }

         if (interrupts.flags.fskFifoAlmostEmpty) {
             uart.sendLog("FSK FIFO Almost Empty");
         }

         if (interrupts.flags.fskRxSync) {
             uart.sendLog("FSK RX Sync");
         }

         if (interrupts.flags.voxLost) {
             uart.sendLog("VOX Lost");
         }

         if (interrupts.flags.voxFound) {
             uart.sendLog("VOX Found");
         }

         if (interrupts.flags.dtmf5ToneFound) {
             uart.sendLog("DTMF 5 Tone Found");
         }
         */

        if (interrupts.flags.cssTailFound) {
            //uart.sendLog("CSS Tail Found");
            toggleRX(false);
        }

        if (interrupts.flags.ctcssLost) {
            //uart.sendLog("CTCSS Lost");
            rxToneDetected = true;
            toggleRX(true, Settings::CodeType::CT);
        }

        if (interrupts.flags.ctcssFound) {
            //uart.sendLog("CTCSS Found");
            rxToneDetected = false;
            toggleRX(false, Settings::CodeType::CT);
        }

        if (interrupts.flags.cdcssLost) {
            //uart.sendLog("CDCSS Lost");
            rxToneDetected = true;
            toggleRX(true, Settings::CodeType::DCS);
        }

        if (interrupts.flags.cdcssFound) {
            //uart.sendLog("CDCSS Found");
            rxToneDetected = false;
            toggleRX(false, Settings::CodeType::CT);
        }

        if (interrupts.flags.sqlLost) {
            //uart.sendLog("SQL Lost");
            toggleRX(true);
        }

        if (interrupts.flags.sqlFound) {
            //uart.sendLog("SQL Found");
            rxToneDetected = false;
            toggleRX(false);
        }


    }
}