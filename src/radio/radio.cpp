#include <cstring>
#include <algorithm>

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

void Radio::setPowerSaveMode() {
    if (inPowerSaveMode) {
        return;
    }

    inPowerSaveMode = true;
    toggleSpeaker(false);
    bk4819.setSleepMode();
}

void Radio::setNormalPowerMode() {
    if (!inPowerSaveMode) {
        return;
    }

    bk4819.rxTurnOn();
    inPowerSaveMode = false;
    delayMs(5); // Give the audio path a moment to wake up
}

void Radio::setActiveVFO(Settings::VFOAB vfo) {
    activeVFO = vfo;
    rxVFO = vfo;
    setupToVFO(vfo);
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
    radioVFO[vfoIndex].rx.frequency = rx;
    radioVFO[vfoIndex].tx.frequency = tx;
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
    bk4819.setCompander((radioVFO[vfoIndex].modulation == ModType::MOD_FM && static_cast<uint8_t>(radioVFO[vfoIndex].compander) >= 2) ? static_cast<uint8_t>(radioVFO[vfoIndex].compander) : 0);

    const bool isAM = radioVFO[vfoIndex].modulation == ModType::MOD_AM;
    if (isAM && static_cast<uint8_t>(radioVFO[vfoIndex].bw) > static_cast<uint8_t>(BK4819_Filter_Bandwidth::BK4819_FILTER_BW_14k)) {
        radioVFO[vfoIndex].bw = BK4819_Filter_Bandwidth::BK4819_FILTER_BW_14k; // tighten AM passband to cut noise
    }

    bk4819.setAGC(true, isAM, radioVFO[vfoIndex].rxagc);
    bk4819.setFilterBandwidth(radioVFO[vfoIndex].bw, isAM); // keep weak-signal bandwidth the same on AM

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
        if (inPowerSaveMode) {
            setNormalPowerMode();
        }

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
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_IDLE, 0);
        }
    }
}

bool Radio::sendFSKMessage(const char* msg) {
    if (!msg || !msg[0]) {
        return false;
    }

    // Use a fixed packet length (64 bytes) like the reference FFSK modem for better stability
    static constexpr size_t PACKET_LEN = 64;
    const size_t maxPayload = PACKET_LEN - 2; // reserve 2 bytes for a simple "MS" header
    size_t payloadLen = std::min<size_t>(std::strlen(msg), maxPayload);

    // Determine TX frequency (use TX if set, otherwise RX)
    uint8_t vfoIndex = static_cast<uint8_t>(getCurrentVFO());
    uint32_t txFreq = radioVFO[vfoIndex].tx.frequency ? radioVFO[vfoIndex].tx.frequency : radioVFO[vfoIndex].rx.frequency;

    // Save state to restore later
    uint16_t css_val = bk4819.readRaw(BK4819_REG_51);
    uint16_t dev_val = bk4819.readRaw(BK4819_REG_40);
    uint16_t filt_val = bk4819.readRaw(BK4819_REG_2B);

    // Disable CTCSS/CDCSS during FSK
    bk4819.writeRaw(BK4819_REG_51, 0);

    // Adjust deviation based on bandwidth
    uint16_t deviation = 850;
    switch (radioVFO[vfoIndex].bw) {
    case BK4819_Filter_Bandwidth::BK4819_FILTER_BW_26k: deviation = 1050; break;
    case BK4819_Filter_Bandwidth::BK4819_FILTER_BW_23k: deviation = 1050; break;
    case BK4819_Filter_Bandwidth::BK4819_FILTER_BW_20k: deviation = 950; break;
    case BK4819_Filter_Bandwidth::BK4819_FILTER_BW_17k: deviation = 900; break;
    default: deviation = 850; break;
    }
    bk4819.writeRaw(BK4819_REG_40, static_cast<uint16_t>((dev_val & 0xF000u) | (deviation & 0x0FFFu)));

    // Disable TX HPF / preemphasis
    bk4819.writeRaw(BK4819_REG_2B, static_cast<uint16_t>((1u << 2) | (1u << 0)));

    // Configure FFSK 1200/1800 TX
    bk4819.writeRaw(BK4819_REG_58, 		
        (1u << 13) |		// 1 FSK TX mode selection
							//   0 = FSK 1.2K and FSK 2.4K TX .. no tones, direct FM
							//   1 = FFSK 1200/1800 TX
							//   2 = ???
							//   3 = FFSK 1200/2400 TX
							//   4 = ???
							//   5 = NOAA SAME TX
							//   6 = ???
							//   7 = ???
							//
		(7u << 10) |		// 0 FSK RX mode selection
							//   0 = FSK 1.2K, FSK 2.4K RX and NOAA SAME RX .. no tones, direct FM
							//   1 = ???
							//   2 = ???
							//   3 = ???
							//   4 = FFSK 1200/2400 RX
							//   5 = ???
							//   6 = ???
							//   7 = FFSK 1200/1800 RX
							//
		(0u << 8) |			// 0 FSK RX gain
							//   0 ~ 3
							//
		(0u << 6) |			// 0 ???
							//   0 ~ 3
							//
		(0u << 4) |			// 0 FSK preamble type selection
							//   0 = 0xAA or 0x55 due to the MSB of FSK sync byte 0
							//   1 = ???
							//   2 = 0x55
							//   3 = 0xAA
							//
		(1u << 1) |			// 1 FSK RX bandwidth setting
							//   0 = FSK 1.2K .. no tones, direct FM
							//   1 = FFSK 1200/1800
							//   2 = NOAA SAME RX
							//   3 = ???
							//   4 = FSK 2.4K and FFSK 1200/2400
							//   5 = ???
							//   6 = ???
							//   7 = ???
							//
		(1u << 0));			// 1 FSK enable
							//   0 = disable
							//   1 = enable

    // Tone2 frequency = 1200Hz and enable tone2 with gain
    bk4819.setTone2Frequency(1200);
    bk4819.writeRaw(BK4819_REG_70, static_cast<uint16_t>((1u << 7) | (96u << 0)));

    // Packet length (bytes) fixed to PACKET_LEN
    bk4819.writeRaw(BK4819_REG_5D, static_cast<uint16_t>(PACKET_LEN << 8));

    // Sync/preamble config
    bk4819.writeRaw(BK4819_REG_5A, 0x5555);
    bk4819.writeRaw(BK4819_REG_5B, 0x55AA);
    bk4819.writeRaw(BK4819_REG_5C, 0x5625); // CRC off

    // FSK control
    uint16_t fsk_reg59 = static_cast<uint16_t>((0u << 15) | (0u << 14) | (0u << 13) | (0u << 12) |
        (0u << 11) | (0u << 10) | (0u << 9) | (0u << 8) | (15u << 4) | (1u << 3) | (0u << 0));
    // clear FIFOs
    bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 15) | (1u << 14) | fsk_reg59));
    bk4819.writeRaw(BK4819_REG_59, fsk_reg59);

    delayMs(100); // let things settle
    // Build and load a padded packet into FIFO (little-endian words)
    uint8_t packet[PACKET_LEN] = { 0 };
    packet[0] = 'M';
    packet[1] = 'S';
    memcpy(&packet[2], msg, payloadLen);

    for (size_t i = 0; i < PACKET_LEN; i += 2) {
        uint16_t w = static_cast<uint16_t>(static_cast<uint8_t>(packet[i]) |
            (static_cast<uint16_t>(packet[i + 1]) << 8));
        bk4819.writeRaw(BK4819_REG_5F, w);
    }

    // Go to TX
    bk4819.enableTxPath();
    bk4819.tuneTo(txFreq, true);
    toggleSpeaker(false);

    // Enable FSK TX
    bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 11) | fsk_reg59));

    // Wait for TX finish (timeout ~ 1000ms)
    uint16_t timeout = 500;
    bool done = false;
    while (timeout-- > 0) {
        delayMs(5);
        if (bk4819.getInterruptRequest() & 1u) {
            bk4819.clearInterrupt();
            uint16_t flags = bk4819.readInterrupt();
            if (flags & BK4819_REG_3F_FSK_TX_FINISHED) {
                done = true;
                break;
            }
        }
    }

    delayMs(100); // let things settle
    // Disable FSK TX
    bk4819.writeRaw(BK4819_REG_59, fsk_reg59);
    bk4819.disableTxPath();

    // Restore registers
    bk4819.writeRaw(BK4819_REG_40, dev_val);
    bk4819.writeRaw(BK4819_REG_2B, filt_val);
    bk4819.writeRaw(BK4819_REG_51, css_val);

    return done;
}

bool Radio::startTX() {
    uint8_t vfoIndex = (uint8_t)getCurrentVFO();
    uint32_t txFrequency = radioVFO[vfoIndex].tx.frequency ? radioVFO[vfoIndex].tx.frequency : radioVFO[vfoIndex].rx.frequency;

    if (systask.isUARTBusy()) {
        return false;
    }

    if (!radioReady) {
        return false;
    }

    if (radioVFO[vfoIndex].modulation != ModType::MOD_FM && radioVFO[vfoIndex].modulation != ModType::MOD_WFM) {
        return false;
    }

    if (!bk4819.canTransmit(txFrequency)) {
        return false;
    }

    if (state == Settings::RadioState::TX_ON) {
        return true;
    }

    setNormalPowerMode();
    toggleSpeaker(false);
    bk4819.toggleGreen(false);
    bk4819.toggleRed(true);

    uint8_t powerIndex = static_cast<uint8_t>(radioVFO[vfoIndex].power);
    if (powerIndex >= 3) {
        powerIndex = 0;
    }

    loadTxCalibrationFromEEPROM();
    uint8_t paBias = selectBias(static_cast<Settings::TXOutputPower>(powerIndex), txFrequency);
    bk4819.setTxPowerLevel(paBias, txFrequency);

    if (!bk4819.switchToTx(txFrequency)) {
        bk4819.toggleRed(false);
        return false;
    }

    applyTxCode(vfoIndex);

    state = Settings::RadioState::TX_ON;
    return true;
}

void Radio::stopTX() {
    if (state != Settings::RadioState::TX_ON) {
        return;
    }

    uint8_t vfoIndex = (uint8_t)getCurrentVFO();
    uint32_t rxFrequency = radioVFO[vfoIndex].rx.frequency;

    uint8_t rogerSetting = radioVFO[vfoIndex].roger;
    if (rogerSetting) {
        sendRogerTone(rogerSetting);
    }

    bk4819.switchToRx(rxFrequency);
    bk4819.toggleRed(false);

    // Stay idle and keep audio muted until a real RX event occurs
    toggleBK4819(false);
    toggleSpeaker(false);
    bk4819.toggleGreen(false);

    state = Settings::RadioState::IDLE;
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_IDLE, 0);
}

void Radio::sendRogerTone(uint8_t rogerSetting) {
    if (!bk4819.isTxActive() || rogerSetting == 0) {
        return;
    }

    uint16_t toneHz = 1000;
    uint16_t durationMs = 120;
    if (rogerSetting == 2) { // MOTO TPT style
        toneHz = 1240;
        durationMs = 240;
    }

    bk4819.setAF(BK4819_AF::BEEP);
    bk4819.enableTone1(96);
    bk4819.setToneFrequency(toneHz);
    delayMs(durationMs);
    bk4819.disableTones();
    bk4819.setAF(BK4819_AF::MUTE);
}

void Radio::applyTxCode(uint8_t vfoIndex) {
    auto& vfo = radioVFO[vfoIndex];

    switch (vfo.tx.codeType) {
    case Settings::CodeType::CT:
        bk4819.setAF(BK4819_AF::CTCO);
        bk4819.setCTCSSFrequency(Settings::CTCSSOptions[vfo.tx.code]);
        break;
    case Settings::CodeType::DCS:
    case Settings::CodeType::NDCS:
        bk4819.setAF(BK4819_AF::CTCO);
        bk4819.setCDCSSCodeWord(DCSGetGolayCodeWord(vfo.tx.codeType, vfo.tx.code));
        break;
    default:
        bk4819.disableTones();
        bk4819.setAF(BK4819_AF::FM);
        break;
    }
}

void Radio::playBeep(Settings::BEEPType beep) {
    bool isSpeakerWasOn = speakerOn;
    uint16_t toneConfig = bk4819.getToneRegister();

    if (inPowerSaveMode) {
        setNormalPowerMode();
    }

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
        if (inPowerSaveMode) {
            if (timeoutPSDualWatch == 10) {
                bk4819.setNormalMode();
                timeoutPSDualWatch = 9;           
            }
            else if (timeoutPSDualWatch >= 1) {            
                if (timeoutPSDualWatch > 1) {
                    timeoutPSDualWatch--;
                } else {
                    bk4819.setSleepMode();
                    timeoutPSDualWatch = 0;
                }
            }
        }

        if (dualWatchTimer > 0) {
            dualWatchTimer--;
        }
        else {
            dualWatchTimer = dualWatchTime;
            setRXVFO((rxVFO == Settings::VFOAB::VFOA) ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA);
            timeoutPSDualWatch = 10;
        }
    }
    else if (dualWatch && state == Settings::RadioState::RX_ON) {
        dualWatchTimer = dualWatchTime;
        timeoutPSDualWatch = 10;
        if (inPowerSaveMode) {
            bk4819.setNormalMode();
            inPowerSaveMode = false;
        }
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
    interruptMask |= BK4819_REG_3F_FSK_RX_SYNC | BK4819_REG_3F_FSK_RX_FINISHED | BK4819_REG_3F_FSK_FIFO_ALMOST_FULL | BK4819_REG_3F_FSK_TX_FINISHED;

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

        bk4819.clearInterrupt();                       // then acknowledge/clear latch

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

        interrupts.__raw = bk4819.readInterrupt();   // read latched flags first

        uart.print("%0.16b\n", interrupts);        

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

        if (fskRxEnabled && (interrupts.flags.fskRxSync || interrupts.flags.fskRxFinied || interrupts.flags.fskFifoAlmostFull || interrupts.flags.fskTxFinied)) {
            handleFSKInterrupts(interrupts.__raw);
            uart.print("FSK : %0.16b\n", interrupts);
        }


    }
}

void Radio::loadTxCalibrationFromEEPROM() {
    if (txCalLoaded) {
        return;
    }

    struct Slot {
        uint32_t freq;
        uint16_t addr;
    };

    // Calibration markers in EEPROM (start addresses of 16-byte blocks)
    static constexpr Slot slots[] = {
        { 5000000u,  0x1ED0 },  // 50 MHz
        { 10800000u, 0x1EE0 },  // 108 MHz
        { 13700000u, 0x1EF0 },  // 137 MHz
        { 17400000u, 0x1F00 },  // 174 MHz
        { 35000000u, 0x1F10 },  // 350 MHz
        { 40000000u, 0x1F20 },  // 400 MHz
        { 47000000u, 0x1F30 },  // 470 MHz
    };

    bool anyValid = false;
    for (size_t i = 0; i < txCalibration.size(); ++i) {
        uint8_t buf[9] = { 0xFF };
        settings.getEEPROM().readBuffer(slots[i].addr, buf, sizeof(buf));

        TxCalPoint pt{};
        pt.freq = slots[i].freq;
        pt.low = buf[0];
        pt.mid = buf[3];
        pt.high = buf[6];
        pt.valid = (pt.low != 0xFF && pt.mid != 0xFF && pt.high != 0xFF);
        txCalibration[i] = pt;
        anyValid |= pt.valid;
    }

    txCalHasValid = anyValid;
    txCalLoaded = true;
}

uint8_t Radio::pickBiasForLevel(const TxCalPoint& pt, Settings::TXOutputPower level) {
    switch (level) {
    case Settings::TXOutputPower::TX_POWER_LOW:
        return pt.low;
    case Settings::TXOutputPower::TX_POWER_MID:
        return pt.mid;
    case Settings::TXOutputPower::TX_POWER_HIGH:
        return pt.high;
    default:
        return pt.low;
    }
}

uint8_t Radio::interpolateBias(uint8_t a, uint8_t b, uint32_t fa, uint32_t fb, uint32_t f) {
    if (fa == fb || f <= fa) {
        return a;
    }
    if (f >= fb) {
        return b;
    }
    int32_t delta = static_cast<int32_t>(b) - static_cast<int32_t>(a);
    uint32_t span = fb - fa;
    return static_cast<uint8_t>(a + (delta * static_cast<int32_t>(f - fa)) / static_cast<int32_t>(span));
}

uint8_t Radio::selectBias(Settings::TXOutputPower level, uint32_t freq) const {
    // Fallback if no calibration present
    if (!txCalLoaded || !txCalHasValid) {
        return defaultPaBias[static_cast<uint8_t>(level) % 3];
    }

    const TxCalPoint* lower = nullptr;
    const TxCalPoint* upper = nullptr;

    const TxCalPoint* lastValid = nullptr;
    for (const auto& pt : txCalibration) {
        if (!pt.valid) {
            continue;
        }

        if (freq <= pt.freq) {
            upper = &pt;
            break;
        }

        lastValid = &pt;
    }

    lower = lastValid;
    if (!lower) {
        // All valid points are above the frequency
        for (const auto& pt : txCalibration) {
            if (pt.valid) {
                lower = &pt;
                break;
            }
        }
    }

    if (!upper) {
        // No higher point found; use the last valid as upper too
        for (auto it = txCalibration.rbegin(); it != txCalibration.rend(); ++it) {
            if (it->valid) {
                upper = &(*it);
                break;
            }
        }
    }

    if (!lower || !upper) {
        return defaultPaBias[static_cast<uint8_t>(level) % 3];
    }

    uint8_t biasLow = pickBiasForLevel(*lower, level);
    uint8_t biasHigh = pickBiasForLevel(*upper, level);
    return interpolateBias(biasLow, biasHigh, lower->freq, upper->freq, freq);
}

void Radio::handleFSKInterrupts(uint16_t flags) {
    // Reset capture on fresh sync/
    if (flags & BK4819_REG_3F_FSK_RX_SYNC) {
        // Clear FIFOs to avoid stale data
        uint16_t base = bk4819.readRaw(BK4819_REG_59) & ~static_cast<uint16_t>((1u << 15) | (1u << 14) | (1u << 12) | (1u << 11));
        bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 15) | (1u << 14) | base));
        bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 12) | base));
    }

    if ((flags & BK4819_REG_3F_FSK_RX_FINISHED) == 0) {
        return;
    }

    char buf[64] = { 0 };
    uint8_t len = 0;

    // Read expected payload from FIFO words
    uint16_t pktLenBytes = static_cast<uint16_t>(bk4819.readRaw(BK4819_REG_5D) >> 8);
    uint16_t wordsToRead = static_cast<uint16_t>((pktLenBytes + 1u) / 2u);

    for (uint16_t i = 0; i < wordsToRead && (static_cast<size_t>(len) + 1) < sizeof(buf); ++i) {
        uint16_t word = bk4819.readRaw(BK4819_REG_5F);
        buf[len++] = static_cast<char>(word & 0xFF);
        if (len < sizeof(buf)) {
            buf[len++] = static_cast<char>((word >> 8) & 0xFF);
        }
    }

    if (len >= sizeof(buf)) {
        len = static_cast<uint8_t>(sizeof(buf) - 1);
    }
    buf[len] = '\0';

    // Strip padding and optional "MS" header so the app sees just the payload
    while (len > 0 && buf[len - 1] == '\0') {
        --len;
    }
    buf[len] = '\0';
    const char* payload = buf;
    if (len >= 2 && buf[0] == 'M' && buf[1] == 'S') {
        payload = &buf[2];
        len = static_cast<uint8_t>(len - 2);
    }

    // Clear FIFOs and re-enable RX
    uint16_t base = bk4819.readRaw(BK4819_REG_59) & ~static_cast<uint16_t>((1u << 15) | (1u << 14) | (1u << 12) | (1u << 11));
    bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 15) | (1u << 14) | base));
    bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 12) | base));

    uint8_t nextTail = static_cast<uint8_t>((fskRxTail + 1) % fskRxQueue.size());
    if (nextTail != fskRxHead && len > 0) {
        strncpy(fskRxQueue[fskRxTail].data(), payload, fskRxQueue[fskRxTail].size() - 1);
        fskRxQueue[fskRxTail][fskRxQueue[fskRxTail].size() - 1] = '\0';
        fskRxTail = nextTail;
    }
}

void Radio::setFSKRxEnabled(bool enable) {
    if (enable == fskRxEnabled) {
        return;
    }

    static uint16_t savedIntMask = 0;
    static uint16_t saved58 = 0;
    static uint16_t saved70 = 0;
    static uint16_t saved5C = 0;
    static uint16_t saved5D = 0;
    static uint16_t saved72 = 0;

    fskRxEnabled = enable;
    if (enable) {
        fskRxHead = fskRxTail = 0;
        savedIntMask = bk4819.readRaw(BK4819_REG_3F);
        saved58 = bk4819.readRaw(BK4819_REG_58);
        saved70 = bk4819.readRaw(BK4819_REG_70);
        saved5C = bk4819.readRaw(BK4819_REG_5C);
        saved5D = bk4819.readRaw(BK4819_REG_5D);
        saved72 = bk4819.readRaw(BK4819_REG_72);

        uint16_t fsk_reg59 = static_cast<uint16_t>(
            (0u << 15) | (0u << 14) | (0u << 13) | (0u << 12) | (0u << 11) |
            (0u << 10) | (0u << 9) | (0u << 8) | (15u << 4) | (1u << 3) | (0u << 0));

        // Tone2 / FSK setup for 1200/1800 FFSK
        bk4819.writeRaw(BK4819_REG_70, static_cast<uint16_t>((1u << 7) | (96u << 0)));
        bk4819.setTone2Frequency(1200);

        bk4819.writeRaw(BK4819_REG_58, static_cast<uint16_t>(
            (1u << 13) |  // FFSK TX (mode select)
            (7u << 10) |  // FFSK 1200/1800 RX
            (3u << 8)  |  // RX gain
            (0u << 4)  |  // preamble type
            (1u << 1)  |  // RX bandwidth for 1200/1800
            (1u << 0)));  // FSK enable

        bk4819.writeRaw(BK4819_REG_5A, 0x5555);
        bk4819.writeRaw(BK4819_REG_5B, 0x55AA);
        bk4819.writeRaw(BK4819_REG_5C, 0x5625); // CRC off

        // FIFO almost-full threshold: 8 bytes, interrupt enable
        bk4819.writeRaw(BK4819_REG_5E, static_cast<uint16_t>((64u << 3) | (1u << 0)));

        // Match TX packet length (64 bytes)
        uint16_t size = 64;
        bk4819.writeRaw(BK4819_REG_5D, static_cast<uint16_t>(size << 8));

        bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 15) | (1u << 14) | fsk_reg59));
        bk4819.writeRaw(BK4819_REG_59, static_cast<uint16_t>((1u << 12) | fsk_reg59));

        // Clear any pending interrupt latches
        bk4819.clearInterrupt();

        uint16_t mask = static_cast<uint16_t>(savedIntMask |
            BK4819_REG_3F_FSK_RX_SYNC |
            BK4819_REG_3F_FSK_RX_FINISHED |
            BK4819_REG_3F_FSK_FIFO_ALMOST_FULL |
            BK4819_REG_3F_FSK_TX_FINISHED);
        bk4819.setInterrupt(mask);

    } else {
        bk4819.writeRaw(BK4819_REG_59, 0);
        bk4819.writeRaw(BK4819_REG_58, saved58);
        bk4819.writeRaw(BK4819_REG_70, saved70);
        bk4819.writeRaw(BK4819_REG_5C, saved5C);
        bk4819.writeRaw(BK4819_REG_5D, saved5D);
        bk4819.writeRaw(BK4819_REG_72, saved72);
        bk4819.setInterrupt(savedIntMask);
    }
}

bool Radio::popFSKMessage(char* out, uint8_t maxLen) {
    if (fskRxHead == fskRxTail) return false;
    strncpy(out, fskRxQueue[fskRxHead].data(), maxLen - 1);
    out[maxLen - 1] = '\0';
    fskRxHead = static_cast<uint8_t>((fskRxHead + 1) % fskRxQueue.size());
    return true;
}
