// -----------------------------------------------------------------------------------------
// This file defines the Radio class, which serves as a high-level abstraction
// for controlling and managing radio functionalities. It encapsulates interactions
// with the BK4819 radio chip driver, manages VFO (Variable Frequency Oscillator) settings,
// handles audio (speaker, beeps), squelch, bandwidth, dual watch, power saving,
// and provides an interface to query radio status like RSSI and operational state.
// -----------------------------------------------------------------------------------------
#pragma once

#include "bk4819.h"   // BK4819 chip driver
#include "uart_hal.h" // UART HAL for debugging/logging
#include "misc.h"     // Miscellaneous utilities
#include "settings.h" // Settings structure definitions (VFO, RadioState, etc.)

// Forward declaration for SystemTask to avoid circular dependencies.
namespace System {
    class SystemTask;
}

namespace RadioNS // Using a namespace to encapsulate radio-specific functionalities.
{
    /**
     * @brief Manages high-level radio operations and state.
     * This class acts as an orchestrator for radio functions, interacting with the
     * BK4819 chip driver, managing VFO settings, and handling features like
     * dual watch, power saving, and audio control.
     */
    class Radio {
    public:
        /**
         * @brief Structure to define a frequency band with its properties.
         */
        struct FrequencyBand {
            char name[11];          ///< Name of the frequency band (e.g., "HAM 2m").
            uint32_t lower_freq;    ///< Lower frequency limit of the band in Hz * 10 (e.g., 14400000 for 144.0 MHz).
            uint32_t upper_freq;    ///< Upper frequency limit of the band in Hz * 10.
            bool txEnable;          ///< True if transmission is allowed in this band, false otherwise.
        };
        
        Settings::VFO radioVFO[2]; ///< Array holding the settings for VFO A and VFO B.

        /**
         * @brief Constructor for the Radio class.
         * @param systask Reference to the SystemTask for inter-task communication or system events.
         * @param uart Reference to the UART HAL for debugging or logging.
         * @param bk4819 Reference to the BK4819 chip driver instance.
         * @param settings Reference to the global Settings instance.
         */
        Radio(System::SystemTask& systask, UART& uart, BK4819& bk4819, Settings& settings) 
            : systask{ systask }, uart{ uart }, bk4819{ bk4819 }, settings{ settings } {};

        /**
         * @brief Toggles the speaker audio path.
         * @param on True to turn the speaker on, false to turn it off.
         */
        void toggleSpeaker(bool on);

        /**
         * @brief Sets the squelch level for the radio.
         * @param f The current frequency (used for band-dependent squelch parameters).
         * @param sql The desired squelch level (0-9 or similar abstract scale).
         */
        void setSquelch(uint32_t f, uint8_t sql);

        /**
         * @brief Sets the IF filter bandwidth on the BK4819 chip.
         * @param bw The desired filter bandwidth from the BK4819_Filter_Bandwidth enum.
         */
        void setFilterBandwidth(BK4819_Filter_Bandwidth bw) { bk4819.setFilterBandwidth(bw); }

        /**
         * @brief Configures a specific VFO with new RX/TX frequencies, channel, and modulation.
         * @param vfo The VFO to configure (VFOA or VFOB).
         * @param rx RX frequency in Hz * 10.
         * @param tx TX frequency in Hz * 10.
         * @param channel Channel number (0 if not a channel).
         * @param modulation Modulation type (FM, AM, etc.).
         */
        void setVFO(Settings::VFOAB vfo, uint32_t rx, uint32_t tx, int16_t channel, ModType modulation);

        /**
         * @brief Applies the settings of the specified VFO to the BK4819 chip for active operation.
         * @param vfo The VFO whose settings are to be applied (VFOA or VFOB).
         */
        void setupToVFO(Settings::VFOAB vfo);

        /**
         * @brief Plays a beep tone.
         * @param beep The type of beep to play (from Settings::BEEPType enum).
         */
        void playBeep(Settings::BEEPType beep);

        /**
         * @brief Gets the settings of the currently active VFO.
         * @return A Settings::VFO structure containing the active VFO's parameters.
         */
        Settings::VFO getActiveVFO() { return radioVFO[(uint8_t)activeVFO]; };

        /**
         * @brief Gets the settings of a specified VFO (A or B).
         * @param vfo The VFO (VFOA or VFOB) whose settings are requested.
         * @return A Settings::VFO structure.
         */
        Settings::VFO getVFO(Settings::VFOAB vfo) { return radioVFO[(uint8_t)vfo]; };

        /**
         * @brief Updates the settings for a specified VFO and applies them.
         * Also updates the VFO name based on channel or frequency band.
         * @param vfoab The VFO to update (VFOA or VFOB).
         * @param vfoData A Settings::VFO structure containing the new settings.
         */
        void setVFO(Settings::VFOAB vfoab, Settings::VFO vfoData) {
            uint8_t vfoIndex = (uint8_t)vfoab;
            radioVFO[vfoIndex] = vfoData; // Update local VFO store.

            // Update VFO name: "CH-XXX" if a channel, or band name if frequency mode.
            if (radioVFO[vfoIndex].channel > 0) {
                snprintf(radioVFO[vfoIndex].name, sizeof(radioVFO[vfoIndex].name), "CH-%03d", radioVFO[vfoIndex].channel);
            }
            else {
                strncpy(radioVFO[vfoIndex].name, getBandName(radioVFO[vfoIndex].rx.frequency), sizeof(radioVFO[vfoIndex].name) - 1);
                radioVFO[vfoIndex].name[sizeof(radioVFO[vfoIndex].name) - 1] = '\0'; // Ensure null termination.
            }
            setupToVFO(vfoab); // Apply the new settings to the hardware.
        };
        
        /**
         * @brief Gets the identifier of the currently active VFO (A or B).
         * @return The active VFO (Settings::VFOAB::VFOA or Settings::VFOAB::VFOB).
         */
        Settings::VFOAB getCurrentVFO(void) { return activeVFO; };

        /**
         * @brief Toggles the active VFO between VFO A and VFO B and applies its settings.
         */
        void changeActiveVFO(void) {
            activeVFO = (activeVFO == Settings::VFOAB::VFOA) ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA;
            setupToVFO(activeVFO); // Apply the new active VFO's settings.
        }

        /**
         * @brief Gets the identifier of the VFO currently configured for reception.
         * @return The RX VFO (Settings::VFOAB::VFOA or Settings::VFOAB::VFOB).
         */
        Settings::VFOAB getRXVFO(void) { return rxVFO; };

        /**
         * @brief Sets the VFO to be used for reception and applies its settings.
         * @param vfo The VFO to set as the RX VFO.
         */
        void setRXVFO(Settings::VFOAB vfo) {
            rxVFO = vfo;
            setupToVFO(vfo); // Apply settings of the new RX VFO.
        }

        /**
         * @brief Toggles the RX state of the radio.
         * @param on True to enable RX, false to disable.
         * @param codeType The type of squelch code (CTCSS/DCS) to use for RX.
         */
        void toggleRX(bool on, Settings::CodeType codeType);

        /**
         * @brief Checks for and processes interrupts from the BK4819 chip.
         */
        void checkRadioInterrupts(void);

        /**
         * @brief Gets the current operational state of the radio (IDLE, RX_ON, TX_ON).
         * @return The current radio state (Settings::RadioState).
         */
        Settings::RadioState getState() { return state; }

        /**
         * @brief Gets the raw RSSI value from the BK4819 chip.
         * @return Raw RSSI value (0-511).
         */
        uint16_t getRSSI() { return bk4819.getRSSI(); }

        /**
         * @brief Gets the RSSI value converted to dBm, with band-specific correction.
         * @return RSSI value in dBm as an int16_t.
         * TODO: The band selection for dBm correction needs to use the current RX VFO's band.
         */
        int16_t getRSSIdBm(void) {
            uint16_t rssi = bk4819.getRSSI();
            int16_t rssidbm = (int16_t)((rssi / 2) - 160); // Basic RSSI to dBm conversion.
            // TODO: RSSI gRxVfo->Band - The band index for dBmCorrTable should be determined by the active RX VFO's frequency band.
            return rssidbm + dBmCorrTable[6]; // Currently uses a fixed index (band 7).
        }

        /**
         * @brief Converts RSSI in dBm to an S-meter level (0-9, or 10 for S9+).
         * @param rssi_dBm RSSI value in dBm.
         * @return S-meter level as a uint8_t.
         */
        uint8_t convertRSSIToSLevel(int16_t rssi_dBm);

        /**
         * @brief Converts RSSI in dBm to a "plus dB" value over S9 (e.g., S9+10dB).
         * @param rssi_dBm RSSI value in dBm.
         * @return dB value over S9. Returns 0 if signal is S9 or weaker.
         */
        int16_t convertRSSIToPlusDB(int16_t rssi_dBm);

        /**
         * @brief Manages the dual watch functionality, periodically switching between VFOs.
         */
        void runDualWatch(void);

        /**
         * @brief Gets the name of the frequency band for a given frequency.
         * @param frequency The frequency in Hz * 10.
         * @return A const char* to the band name, or an empty string if not found.
         */
        const char* getBandName(uint32_t frequency) {
            int num_bands = sizeof(radioBands) / sizeof(radioBands[0]);

            for (int i = 0; i < num_bands; ++i) {
                if (frequency >= radioBands[i].lower_freq && frequency <= radioBands[i].upper_freq) {
                    return radioBands[i].name;
                }
            }
            return ""; // Not found in defined bands.
        }

        /**
         * @brief Configures CTCSS/DCS tone detection for the specified VFO.
         * @param vfo The VFO (VFOA or VFOB) for which to set up tone detection.
         */
        void setupToneDetection(Settings::VFOAB vfo);

        /**
         * @brief Checks if a CTCSS/DCS tone is currently detected on the RX path.
         * @return True if a tone is detected, false otherwise.
         */
        bool isRXToneDetected(void) { return rxToneDetected; }

        /**
         * @brief Checks if the radio hardware (BK4819) has been initialized and is ready.
         * @return True if the radio is ready, false otherwise.
         */
        bool isRadioReady(void) { return radioReady; }

        /**
         * @brief Sets the radio ready state.
         * @param ready True to mark radio as ready, false otherwise.
         */
        void setRadioReady(bool ready) { radioReady = ready; }
        
        /**
         * @brief Puts the radio into power save mode by instructing the BK4819 chip.
         */
        void setPowerSaveMode() {
            inPowerSaveMode = true;            
            bk4819.setSleepMode(); // Tell BK4819 to enter low-power mode.
        }

        /**
         * @brief Checks if the radio is currently in power save mode.
         * @return True if in power save mode, false otherwise.
         */
        bool isPowerSaveMode(void) { return inPowerSaveMode; }

        /**
         * @brief Takes the radio out of power save mode.
         */
        void setNormalPowerMode() {
            if (!inPowerSaveMode) {
                return; // Already in normal mode.
            }
            inPowerSaveMode = false;
            bk4819.setNormalMode(); // Tell BK4819 to resume normal operation.
        }

    private:
        // References to other system components.
        System::SystemTask& systask; ///< Reference to the main system task.
        UART& uart;                  ///< Reference to the UART for logging/debug.
        BK4819& bk4819;              ///< Reference to the BK4819 chip driver.
        Settings& settings;          ///< Reference to the global settings.

        bool inPowerSaveMode = false; ///< Flag indicating if power save mode is active.

        // Dual Watch related members.
        bool dualWatch = true;              ///< Flag to enable/disable dual watch.
        uint8_t dualWatchTimer = 0;         ///< Timer counter for dual watch switching.
        uint8_t timeoutPSDualWatch = 10;    ///< Timeout for power save during dual watch.
        static constexpr uint8_t dualWatchTime = 50; ///< Interval for dual watch VFO switching (e.g., in 10ms units).

        bool rxToneDetected = false; ///< True if a CTCSS/DCS tone matching the VFO's setting is detected.
        bool radioReady = false;     ///< True if the BK4819 chip is initialized and ready.
        bool speakerOn = false;      ///< Current state of the speaker (on/off).

        Settings::RadioState state = Settings::RadioState::IDLE; ///< Current operational state of the radio.
        Settings::VFOAB activeVFO = Settings::VFOAB::VFOA;       ///< The VFO currently selected by the user for TX/main interaction.
        Settings::VFOAB rxVFO = Settings::VFOAB::VFOA;           ///< The VFO currently configured for reception.
        Settings::VFOAB lastRXVFO = Settings::VFOAB::NONE;       ///< The VFO that last had RX activity (used for dual watch logic).

        // RSSI to dBm correction table. Index corresponds to a band.
        static constexpr int8_t dBmCorrTable[7] = {
            -15, // band 1
            -25, // band 2
            -20, // band 3
            -4,  // band 4
            -7,  // band 5
            -6,  // band 6
            -1   // band 7
        };

        // Table defining known frequency bands and their properties.
        static constexpr FrequencyBand radioBands[] = {
            {"HAM 17m",   1806800,   1816800, 0}, // 17-meter Ham band
            {"HAM 15m",   2100000,   2145000, 0}, // 15-meter Ham band
            // ... (other band definitions) ...
            {"MARINE VHF",15600000, 17400000, 0}, // Marine VHF band
        };

        /**
         * @brief Internal helper to toggle BK4819 power/enable state (implementation details not shown).
         * @param on True to turn on, false to turn off.
         */
        void toggleBK4819(bool on);

        /**
         * @brief Calculates the Golay (23,12) code word used in DCS.
         * @param codeWord The 11-bit DCS code (without parity bits).
         * @return The 23-bit Golay encoded word.
         */
        uint32_t DCSCalculateGolay(uint32_t codeWord) {
            unsigned int i;
            uint32_t word = codeWord;
            for (i = 0; i < 12; i++) { // Iterate 12 times for Golay encoding.
                word <<= 1; // Shift left.
                if (word & 0x1000) // If bit 12 (0x1000) is 1 after shift.
                    word ^= 0x08EA; // XOR with Golay generator polynomial (0x8EA or G(x)=x^11+x^9+x^7+x^6+x^5+x+1).
            }
            // Combine original code (shifted for parity) with calculated parity bits.
            // The original code is effectively the first 11 bits, parity is next 11, last bit is often fixed or also parity.
            // This specific implementation places parity bits after the original 11/12 bits.
            return codeWord | ((word & 0x0FFE) << 11); // `word` now contains parity in lower bits.
        }

        /**
         * @brief Gets the full 23-bit Golay encoded DCS code word for a given DCS option.
         * @param codeType Specifies if the code is normal (NDCS) or inverted (DCS).
         * @param option Index into the `Settings::DCSOptions` array.
         * @return The 23-bit Golay encoded word. If `codeType` is NDCS, the result is inverted.
         */
        uint32_t DCSGetGolayCodeWord(Settings::CodeType codeType, uint8_t option) {
            // Add 0x800U to the DCS option before Golay calculation (specific to this system's DCS table).
            uint32_t code = DCSCalculateGolay(Settings::DCSOptions[option] + 0x800U);
            if (codeType == Settings::CodeType::NDCS) // If inverted DCS is required.
                code ^= 0x7FFFFF; // Invert all 23 bits.
            return code;
        }
    };

} // namespace Radio