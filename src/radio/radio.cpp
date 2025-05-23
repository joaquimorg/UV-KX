// -----------------------------------------------------------------------------------------
// This file implements the Radio class methods, providing high-level control
// over radio operations. It includes functionality for RSSI processing,
// speaker and audio path management, VFO configuration, squelch and filter settings,
// beep tone generation, dual watch, power saving modes, and handling of BK4819
// radio chip interrupts for events like CTCSS/DCS detection and squelch status.
// -----------------------------------------------------------------------------------------
#include <cstring> // For C-style string functions like strncpy, snprintf

#include "radio.h"  // Header for this implementation
#include "sys.h"      // For delayMs and potentially other system utilities
#include "gpio.h"     // For GPIO control (e.g., speaker path)
#include "system.h"   // For SystemTask message pushing

using namespace RadioNS;

/**
 * @brief Converts an RSSI value in dBm to an S-meter level.
 * The S-meter scale is typically S0 to S9, with values above S9 also possible.
 * This function maps dBm ranges to S-levels 0-9, and 10 for anything above S9.
 * @param rssi_dBm The RSSI value in dBm.
 * @return The corresponding S-meter level (0-10).
 */
uint8_t Radio::convertRSSIToSLevel(int16_t rssi_dBm) {
    // Standard S-meter calibration points (approximate)
    if (rssi_dBm <= -121) return 0; // S0 or below
    if (rssi_dBm <= -115) return 1; // S1
    if (rssi_dBm <= -109) return 2; // S2
    if (rssi_dBm <= -103) return 3; // S3
    if (rssi_dBm <= -97)  return 4; // S4
    if (rssi_dBm <= -91)  return 5; // S5
    if (rssi_dBm <= -85)  return 6; // S6
    if (rssi_dBm <= -79)  return 7; // S7
    if (rssi_dBm <= -73)  return 8; // S8
    if (rssi_dBm <= -67)  return 9; // S9
    return 10; // Stronger than S9 (S9+dB)
}

/**
 * @brief Converts an RSSI value (stronger than S9) to a "dB over S9" value.
 * For example, if S9 is -67 dBm, an RSSI of -57 dBm would be S9+10dB.
 * @param rssi_dBm The RSSI value in dBm.
 * @return The number of dB over S9. Returns 0 if the signal is S9 or weaker.
 */
int16_t Radio::convertRSSIToPlusDB(int16_t rssi_dBm) {
    if (rssi_dBm > -67) { // Assuming -67 dBm is the S9 threshold
        return (rssi_dBm + 67); // Calculate dB over S9
    }
    return 0; // Not stronger than S9
}

/**
 * @brief Toggles the external audio path (speaker).
 * @param on True to enable the speaker path, false to disable it.
 */
void Radio::toggleSpeaker(bool on) {
    speakerOn = on;
    if (on) {
        // Set a GPIO pin high to enable the audio amplifier or path.
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH); 
    }
    else {
        // Clear the GPIO pin to disable the audio path.
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    }
}

/**
 * @brief Sets the squelch level on the BK4819 chip.
 * @param f The current operating frequency (can be used for band-specific squelch tables, though currently fixed).
 * @param sql The desired squelch level (abstract value, e.g., 0-9).
 * The open and close delay parameters are currently hardcoded to 1.
 */
void Radio::setSquelch(uint32_t f, uint8_t sql) {
    // Calls the BK4819 driver's squelch configuration function.
    // `gSettings.sqlOpenTime` and `gSettings.sqlCloseTime` are commented out, using fixed delays.
    bk4819.squelch(sql, f, 1, 1); // Hardcoded open/close delay of 1.
}

/**
 * @brief Initializes a VFO structure with specified parameters.
 * This function populates a `Settings::VFO` object with default values for many parameters,
 * along with the provided RX/TX frequencies, channel number, and modulation type.
 * It also generates a name for the VFO based on channel or frequency band.
 * @param vfo Identifier of the VFO to set (VFOA or VFOB).
 * @param rx RX frequency in Hz * 10.
 * @param tx TX frequency in Hz * 10.
 * @param channel Channel number (0 if not a channel-based VFO).
 * @param modulation Modulation type (e.g., MOD_FM).
 */
void Radio::setVFO(Settings::VFOAB vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation) {
    uint8_t vfoIndex = (uint8_t)vfo;
    // Apply basic frequency and channel info.
    radioVFO[vfoIndex].rx.frequency = (uint32_t)(rx & 0x07FFFFFF); // Mask to ensure valid range (example).
    radioVFO[vfoIndex].tx.frequency = (uint32_t)(tx & 0x07FFFFFF); // Mask to ensure valid range.
    radioVFO[vfoIndex].channel = channel;
    radioVFO[vfoIndex].modulation = modulation;

    // Set default values for other VFO parameters.
    radioVFO[vfoIndex].squelch = 1; // Default squelch level.
    radioVFO[vfoIndex].step = Settings::Step::STEP_12_5kHz; // Default frequency step.
    radioVFO[vfoIndex].bw = BK4819_Filter_Bandwidth::BK4819_FILTER_BW_20k; // Default bandwidth.
    radioVFO[vfoIndex].power = Settings::TXOutputPower::TX_POWER_LOW; // Default TX power.
    radioVFO[vfoIndex].shift = Settings::OffsetDirection::OFFSET_NONE; // Default repeater offset.
    radioVFO[vfoIndex].repeaterSte = Settings::ONOFF::OFF; // Default repeater STE.
    radioVFO[vfoIndex].ste = Settings::ONOFF::OFF; // Default STE.
    radioVFO[vfoIndex].compander = Settings::TXRX::OFF; // Default compander state.
    radioVFO[vfoIndex].pttid = 0; // Default PTT ID.
    radioVFO[vfoIndex].rxagc = 18; // Default RX AGC setting (auto).
    radioVFO[vfoIndex].rx.codeType = Settings::CodeType::NONE; // Default RX CTCSS/DCS type.
    radioVFO[vfoIndex].rx.code = 0; // Default RX code.
    radioVFO[vfoIndex].tx.codeType = Settings::CodeType::NONE; // Default TX CTCSS/DCS type.
    radioVFO[vfoIndex].tx.code = 0; // Default TX code.

    // Generate VFO name.
    if (channel > 0) {
        snprintf(radioVFO[vfoIndex].name, sizeof(radioVFO[vfoIndex].name), "CH-%03d", channel);
    }
    else {
        strncpy(radioVFO[vfoIndex].name, getBandName(rx), sizeof(radioVFO[vfoIndex].name) - 1);
        radioVFO[vfoIndex].name[sizeof(radioVFO[vfoIndex].name) - 1] = '\0'; // Ensure null termination.
    }
}

/**
 * @brief Configures the BK4819 radio chip according to the settings of the specified VFO.
 * This includes setting squelch, modulation, compander, AGC, filter bandwidth,
 * tone detection (CTCSS/DCS), and tuning to the VFO's RX frequency.
 * @param vfo The VFO (VFOA or VFOB) whose settings are to be applied to the hardware.
 */
void Radio::setupToVFO(Settings::VFOAB vfo) {
    uint8_t vfoIndex = (uint8_t)vfo;
    Settings::VFO currentVfoSettings = radioVFO[vfoIndex]; // Use a copy for clarity.

    // Configure squelch type and level.
    bk4819.squelchType(SquelchType::SQUELCH_RSSI_NOISE_GLITCH); // Default squelch logic.
    setSquelch(currentVfoSettings.rx.frequency, 4); // Example fixed squelch level 4.

    bk4819.setModulation(currentVfoSettings.modulation);

    // Configure RX compander based on VFO settings (only if FM and compander is set for RX or RX/TX).
    bool companderRxActive = (currentVfoSettings.modulation == ModType::MOD_FM && 
                              static_cast<uint8_t>(currentVfoSettings.compander) >= 2);
    bk4819.setCompander(companderRxActive ? static_cast<uint8_t>(currentVfoSettings.compander) : 0);

    // Configure AGC (disable for AM, use VFO setting for others).
    bk4819.setAGC(currentVfoSettings.modulation != ModType::MOD_AM, currentVfoSettings.rxagc);
    bk4819.setFilterBandwidth(currentVfoSettings.bw);

    bk4819.rxTurnOn(); // Ensure RX path is enabled.

    setupToneDetection(vfo); // Configure CTCSS/DCS detection.

    // Tune the radio to the VFO's RX frequency with precise calibration.
    bk4819.tuneTo(currentVfoSettings.rx.frequency, true);
}

/**
 * @brief Internal helper function to toggle core BK4819 AF DAC and AF bit.
 * This is likely used to enable/disable the audio output path in the chip.
 * @param on True to enable, false to disable.
 */
void Radio::toggleBK4819(bool on) {
    if (on) {
        bk4819.toggleAFDAC(true); // Enable Audio Frequency DAC.
        bk4819.toggleAFBit(true); // Enable a specific AF path bit (details in BK4819 driver).
    }
    else {
        bk4819.toggleAFDAC(false);
        bk4819.toggleAFBit(false);
    }
}

/**
 * @brief Manages the radio's receive state (turning RX on or off).
 * It handles BK4819 chip state, speaker, LED indicators, and system messages.
 * @param on True to enable reception, false to disable.
 * @param codeType The type of CTCSS/DCS code detected (or NONE). Used to decide if speaker should unmute.
 */
void Radio::toggleRX(bool on, Settings::CodeType codeType = Settings::CodeType::NONE) {
    uint8_t vfoIndex = (uint8_t)getRXVFO();
    Settings::VFO currentRxVfoSettings = radioVFO[vfoIndex];

    if (on) { // Turning RX ON
        if (state != Settings::RadioState::RX_ON) { // If not already in RX state
            bk4819.toggleGreen(true); // Turn on Green LED for RX indication.
            toggleBK4819(true);      // Enable BK4819 AF path.
            state = Settings::RadioState::RX_ON;
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_RX, 0); // Notify system.
        }

        // Logic to unmute speaker:
        // If FM mode and a CTCSS/DCS code is configured for RX,
        // only unmute if the detected `codeType` is NOT NONE (i.e., a valid carrier or matching tone was found).
        // If no code is configured (CodeType::NONE), unmute directly on signal.
        if (currentRxVfoSettings.modulation == ModType::MOD_FM &&
            currentRxVfoSettings.rx.codeType != Settings::CodeType::NONE &&
            codeType == Settings::CodeType::NONE) { // FM with code, but no matching code detected by interrupt
            return; // Keep speaker muted.
        }
        toggleSpeaker(true); // Unmute speaker.
    }
    else { // Turning RX OFF
        if (state != Settings::RadioState::IDLE) { // If not already IDLE
            toggleSpeaker(false);     // Mute speaker.
            bk4819.toggleGreen(false); // Turn off Green LED.
            toggleBK4819(false);      // Disable BK4819 AF path.
            state = Settings::RadioState::IDLE;
            systask.pushMessage(System::SystemTask::SystemMSG::MSG_RADIO_IDLE, 0); // Notify system.
        }
    }
}

/**
 * @brief Plays a beep tone with specified characteristics.
 * Manages speaker state, BK4819 tone generation, and delays for beep duration.
 * @param beep The type of beep to play, defining frequency and duration patterns.
 */
void Radio::playBeep(Settings::BEEPType beep) {
    bool isSpeakerWasOn = speakerOn; // Store current speaker state.
    uint16_t originalToneConfig = bk4819.getToneRegister(); // Store original tone config.

    // Beep can only play if radio is IDLE.
    if (state != Settings::RadioState::IDLE) {
        return;
    }

    toggleSpeaker(false); // Temporarily mute speaker.
    delayMs(20);          // Short delay.

    // Determine beep frequency based on BEEPType.
    uint16_t toneFrequency;
    switch (beep) {
        default:
        case Settings::BEEPType::BEEP_NONE: // Should ideally not play anything or a very short click.
            toneFrequency = 220; // Low frequency for "NONE" or default.
            break;
        // ... (other cases for different beep types) ...
        case Settings::BEEPType::BEEP_880HZ_40MS_OPTIONAL: // Fallthrough example
        case Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP:
        case Settings::BEEPType::BEEP_880HZ_200MS:
        case Settings::BEEPType::BEEP_880HZ_500MS:
            toneFrequency = 880;
            break;
    }

    bk4819.playTone(toneFrequency, true); // Start playing the tone on BK4819.
    delayMs(2);                          // Short delay for tone to stabilize.
    toggleSpeaker(true);                 // Unmute speaker to output the tone.
    // delayMs(60); // Initial part of beep, common to many types. (This was removed, logic changed below)

    // Determine duration and pattern for multi-beep types.
    uint16_t duration;
    switch (beep) {
        case Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP:
            bk4819.exitTxMute(); delayMs(60); bk4819.enterTxMute(); delayMs(20); // 1st beep part
            [[fallthrough]]; // Fall through for second part
        case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL:
        case Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP:
            bk4819.exitTxMute(); delayMs(60); bk4819.enterTxMute(); delayMs(20); // 2nd beep part (or 1st for double)
            [[fallthrough]]; // Fall through for final part
        case Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL:
             // bk4819.exitTxMute(); // Already called if fallen through, or call here if single
            duration = 60;
            break;
        // ... (other duration cases) ...
        default: // Includes BEEP_NONE, BEEP_440HZ_500MS, BEEP_880HZ_500MS
            // bk4819.exitTxMute(); // If single beep, ensure TX mute is exited for duration.
            duration = (beep == Settings::BEEPType::BEEP_NONE) ? 10 : 500; // Short for NONE, else 500ms
            break;
    }
    if(beep != Settings::BEEPType::BEEP_NONE) bk4819.exitTxMute(); // Ensure TX Mute is exited for the main duration part
    delayMs(duration); // Hold tone for calculated duration.

    // Cleanup after beep.
    bk4819.enterTxMute(); // Mute BK4819 tone path.
    delayMs(20);
    toggleSpeaker(false); // Mute speaker.
    delayMs(5);
    bk4819.turnsOffTonesTurnsOnRX(); // Restore radio to RX mode, disable tones.
    delayMs(5);
    bk4819.setToneRegister(originalToneConfig); // Restore original tone config.

    toggleSpeaker(isSpeakerWasOn); // Restore original speaker state.
}

/**
 * @brief Manages dual watch functionality.
 * If dual watch is enabled and the radio is idle, it periodically switches
 * the RX VFO between VFO A and VFO B. It also handles power saving during dual watch.
 */
void Radio::runDualWatch(void) {
    if (dualWatch && state == Settings::RadioState::IDLE) {
        // Handle power saving during dual watch.
        if (inPowerSaveMode) {
            if (timeoutPSDualWatch == 10) { // Initial state or after RX activity
                bk4819.setNormalMode(); // Wake up chip to check other VFO.
                timeoutPSDualWatch = 9;           
            }
            else if (timeoutPSDualWatch >= 1) { // Countdown to re-enter sleep.
                if (timeoutPSDualWatch > 1) {
                    timeoutPSDualWatch--;
                } else { // Timeout expired.
                    bk4819.setSleepMode(); // Put chip back to sleep.
                    timeoutPSDualWatch = 0;
                }
            }
        }

        // Handle VFO switching timer.
        if (dualWatchTimer > 0) {
            dualWatchTimer--;
        }
        else { // Dual watch timer expired.
            dualWatchTimer = dualWatchTime; // Reset timer.
            // Switch to the other VFO for listening.
            setRXVFO((rxVFO == Settings::VFOAB::VFOA) ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA);
            timeoutPSDualWatch = 10; // Reset power save timeout for the new VFO check.
        }
    }
    else if (dualWatch && state == Settings::RadioState::RX_ON) {
        // If RX becomes active, reset dual watch timers and ensure radio is not in power save.
        dualWatchTimer = dualWatchTime;
        timeoutPSDualWatch = 10;
        if (inPowerSaveMode) {
            bk4819.setNormalMode();
            inPowerSaveMode = false;
        }
    }
}

/**
 * @brief Configures CTCSS/DCS tone detection settings on the BK4819 for a given VFO.
 * Sets up the appropriate interrupt mask based on the VFO's configured code type.
 * @param vfo The VFO (VFOA or VFOB) for which to configure tone detection.
 */
void Radio::setupToneDetection(Settings::VFOAB vfo) {
    uint8_t vfoIndex = (uint8_t)vfo;
    Settings::VFO currentVfoSettings = radioVFO[vfoIndex];

    uint16_t interruptMask = BK4819_REG_3F_SQUELCH_FOUND | BK4819_REG_3F_SQUELCH_LOST | BK4819_REG_3F_DTMF_5TONE_FOUND;

    if (currentVfoSettings.modulation == ModType::MOD_FM) { // Tone squelch usually for FM.
        switch (currentVfoSettings.rx.codeType) {
        case Settings::CodeType::DCS:
        case Settings::CodeType::NDCS:
            bk4819.setCDCSSCodeWord(DCSGetGolayCodeWord(currentVfoSettings.rx.codeType, currentVfoSettings.rx.code));
            interruptMask |= BK4819_REG_3F_CDCSS_FOUND | BK4819_REG_3F_CDCSS_LOST;
            break;
        case Settings::CodeType::CT:
            bk4819.setCTCSSFrequency(Settings::CTCSSOptions[currentVfoSettings.rx.code]);
            interruptMask |= BK4819_REG_3F_CTCSS_FOUND | BK4819_REG_3F_CTCSS_LOST;
            break;
        default: // Settings::CodeType::NONE
            // If STE (Squelch Tail Elimination) is enabled, configure for tail detection.
            if (currentVfoSettings.ste == Settings::ONOFF::ON) {
                bk4819.setCTCSSFrequency(550); // Use a common tail frequency (e.g., 55Hz).
                bk4819.setTailDetection(550);  // Configure BK4819 for this tail.
                interruptMask |= BK4819_REG_3F_CxCSS_TAIL;
            }
            break;
        }
    }
    bk4819.setInterrupt(interruptMask); // Apply the configured interrupt mask.
}

/**
 * @brief Checks and processes pending interrupts from the BK4819 radio chip.
 * This function reads the interrupt status register and handles events like
 * squelch open/close, CTCSS/DCS found/lost, and CTCSS tail detection.
 * It then calls `toggleRX` to update the radio's receive state accordingly.
 */
void Radio::checkRadioInterrupts(void) {
    // Loop while there are pending interrupt requests from BK4819.
    while (bk4819.getInterruptRequest() & 1u) { // Bit 0 of REG_0C indicates general interrupt.
        bk4819.clearInterrupt(); // Clear the main interrupt flag in REG_02.
        
        // Define a union to easily access individual interrupt flags from the raw register value.
        union {
            struct InterruptFlags { // Matches bit positions in BK4819_REG_02.
                uint16_t __UNUSED          : 1; // Bit 0 (often unused or general flag already checked)
                uint16_t fskRxSync         : 1; // Bit 1
                uint16_t sqlLost           : 1; // Bit 2: Squelch Lost (closed)
                uint16_t sqlFound          : 1; // Bit 3: Squelch Found (opened)
                uint16_t voxLost           : 1; // Bit 4
                uint16_t voxFound          : 1; // Bit 5
                uint16_t ctcssLost         : 1; // Bit 6
                uint16_t ctcssFound        : 1; // Bit 7
                uint16_t cdcssLost         : 1; // Bit 8
                uint16_t cdcssFound        : 1; // Bit 9
                uint16_t cssTailFound      : 1; // Bit 10: CTCSS/DCS Tail detected
                uint16_t dtmf5ToneFound    : 1; // Bit 11
                uint16_t fskFifoAlmostFull : 1; // Bit 12
                uint16_t fskRxFinied       : 1; // Bit 13 (Typo in original: Finished)
                uint16_t fskFifoAlmostEmpty: 1; // Bit 14
                uint16_t fskTxFinied       : 1; // Bit 15 (Typo in original: Finished)
            } flags;
            uint16_t __raw; // To read the entire register value at once.
        } interrupts;

        interrupts.__raw = bk4819.readInterrupt(); // Read the detailed interrupt status from REG_02.

        // --- Handle specific interrupt flags ---
        // Note: Some logging (uart.sendLog) is commented out.

        if (interrupts.flags.cssTailFound) { // CTCSS/DCS tail detected.
            toggleRX(false); // Close RX path.
        }

        if (interrupts.flags.ctcssFound) { // Matching CTCSS tone found.
            rxToneDetected = true;
            toggleRX(true, Settings::CodeType::CT); // Open RX path, indicate CTCSS was reason.
        }
        if (interrupts.flags.ctcssLost) {  // CTCSS tone lost.
            rxToneDetected = false;
            toggleRX(false, Settings::CodeType::CT); // Close RX path.
        }

        if (interrupts.flags.cdcssFound) { // Matching CDCSS code found.
            rxToneDetected = true;
            toggleRX(true, Settings::CodeType::DCS); // Open RX path, indicate DCS was reason.
        }
        if (interrupts.flags.cdcssLost) {  // CDCSS code lost.
            rxToneDetected = false;
            toggleRX(false, Settings::CodeType::DCS); // Close RX path (using DCS to indicate it was a code loss).
                                                     // Original code used CT here, might be a typo or specific logic.
        }
        
        if (interrupts.flags.sqlFound) { // Squelch opened (signal detected without specific tone match needed or after tone match).
            //uart.sendLog("SQL Found"); // Original log
            toggleRX(true); // Open RX path (CodeType::NONE implies general signal).
        }
        if (interrupts.flags.sqlLost) {  // Squelch closed (signal lost).
            //uart.sendLog("SQL Lost"); // Original log
            rxToneDetected = false; // No tone can be detected if squelch is closed.
            toggleRX(false); // Close RX path.
        }
        // Other interrupt flags (FSK, VOX, DTMF) are not handled here in the provided snippet.
    }
}