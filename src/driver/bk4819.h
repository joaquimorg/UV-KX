// -----------------------------------------------------------------------------------------
// This file defines the BK4819 class, a driver for the BK4819 radio transceiver chip.
// It provides an interface to initialize, configure, and control various radio functions
// such as frequency tuning, modulation, filter bandwidth, squelch, AGC, CTCSS/CDCSS,
// GPIO control, and interrupt handling. Communication with the chip is performed via SPI.
// -----------------------------------------------------------------------------------------
#pragma once

#include <array>       // For std::array
#include "sys.h"        // System-level definitions, possibly for delayMs
#include "spi_sw_hal.h" // Software SPI HAL for communication with BK4819
#include "bk4819-regs.h"// BK4819 register definitions and helper structures (like RegisterSpec)

/**
 * @brief Enumerates the modulation types supported by the BK4819 chip.
 */
enum class ModType : uint8_t {
    MOD_FM = 0,   ///< Frequency Modulation
    MOD_AM = 1,   ///< Amplitude Modulation
    MOD_LSB = 2,  ///< Lower Sideband
    MOD_USB = 3,  ///< Upper Sideband
    MOD_BYP = 4,  ///< Bypass mode (details specific to chip)
    MOD_RAW = 5,  ///< Raw mode (details specific to chip)
    MOD_WFM = 6,  ///< Wideband FM
    MOD_PRST = 7  ///< Preset mode (details specific to chip)
};

/**
 * @brief Enumerates the IF filter bandwidth settings for the BK4819 chip.
 */
enum class BK4819_Filter_Bandwidth : uint8_t {
    BK4819_FILTER_BW_26k = 0,   ///< 26 kHz filter bandwidth
    BK4819_FILTER_BW_23k = 1,   ///< 23 kHz filter bandwidth
    BK4819_FILTER_BW_20k = 2,   ///< 20 kHz filter bandwidth
    BK4819_FILTER_BW_17k = 3,   ///< 17 kHz filter bandwidth
    BK4819_FILTER_BW_14k = 4,   ///< 14 kHz filter bandwidth
    BK4819_FILTER_BW_12k = 5,   ///< 12 kHz filter bandwidth
    BK4819_FILTER_BW_10k = 6,   ///< 10 kHz filter bandwidth (Narrow)
    BK4819_FILTER_BW_9k  = 7,   ///< 9 kHz filter bandwidth (Narrow)
    BK4819_FILTER_BW_7k  = 8,   ///< 7 kHz filter bandwidth (Ultra-narrow)
    BK4819_FILTER_BW_6k  = 9    ///< 6 kHz filter bandwidth (Ultra-narrow)
};

/**
 * @brief Enumerates audio filter (AF) modes for the BK4819 chip.
 * These modes determine how the audio path is configured (e.g., for FM, AM, tones).
 */
enum class BK4819_AF : uint16_t {
    MUTE   = 0x0000, ///< Mute audio output
    FM     = 0x0001, ///< Standard FM audio path
    ALAM   = 0x0002, ///< Alarm tone generation
    BEEP   = 0x0003, ///< Beep tone generation (often for TX)
    RAW    = 0x0004, ///< Raw audio path (e.g., SSB without IF filter)
    USB    = 0x0005, ///< USB/LSB audio path (simultaneous)
    CTCO   = 0x0006, ///< CTCSS/DCS tone filter path
    AM     = 0x0007, ///< AM audio path
    FSKO   = 0x0008, ///< FSK output test
    BYPASS = 0x0009  ///< FM audio path bypassing some filters
};

/**
 * @brief Enumerates different squelch detection types/logics.
 */
enum class SquelchType {
    SQUELCH_RSSI_NOISE_GLITCH, ///< Squelch based on RSSI, Noise, and Glitch detection
    SQUELCH_RSSI_GLITCH,       ///< Squelch based on RSSI and Glitch detection
    SQUELCH_RSSI_NOISE,        ///< Squelch based on RSSI and Noise detection
    SQUELCH_RSSI,              ///< Squelch based purely on RSSI
};

/**
 * @brief Container for pre-defined squelch parameter tables.
 * The `SQ` array holds threshold values for different squelch levels and bands.
 * Dimensions: [Band][Parameter_Type][Squelch_Level]
 */
class SQObject {
public:
    // SQ[band_index][parameter_type_index][squelch_level_index]
    // band_index: 0 or 1 (e.g., for different frequency bands like VHF/UHF)
    // parameter_type_index:
    //   0: RSSI Open Threshold
    //   1: RSSI Close Threshold
    //   2: Noise Open Threshold
    //   3: Noise Close Threshold
    //   4: Glitch Open Threshold
    //   5: Glitch Close Threshold
    // squelch_level_index: 0-10 corresponding to user-selectable squelch levels.
    static constexpr uint8_t SQ[2][6][11] = {
        // Band 0 Squelch Parameters
        {
            {0, 10, 62, 66, 74, 75, 92, 95, 98, 170, 252}, // RSSI Open
            {0, 5, 60, 64, 72, 70, 89, 92, 95, 166, 250},  // RSSI Close
            {255, 240, 56, 54, 48, 45, 32, 29, 20, 25, 20},// Noise Open
            {255, 250, 61, 58, 52, 48, 35, 32, 23, 30, 30},// Noise Close
            {255, 240, 135, 135, 116, 17, 3, 3, 2, 50, 50},// Glitch Open
            {255, 250, 150, 140, 120, 20, 5, 5, 4, 45, 45},// Glitch Close
        },
        // Band 1 Squelch Parameters
        {
            {0, 50, 78, 88, 94, 110, 114, 117, 119, 200, 252},// RSSI Open
            {0, 40, 76, 86, 92, 106, 110, 113, 115, 195, 250},// RSSI Close
            {255, 65, 49, 44, 42, 40, 33, 30, 22, 23, 22},   // Noise Open
            {255, 70, 59, 54, 46, 45, 37, 34, 25, 27, 25},   // Noise Close
            {255, 90, 135, 135, 116, 10, 8, 7, 6, 32, 32},   // Glitch Open
            {255, 100, 150, 140, 120, 15, 12, 11, 10, 30, 30},// Glitch Close
        },
    };
};

/**
 * @brief Driver class for the BK4819 radio transceiver chip.
 * Provides methods to interact with the chip via SPI for configuration and control.
 */
class BK4819 {
public:
    /**
     * @brief Constructor for the BK4819 class.
     * Calls `initializeChip()` to perform initial setup of the radio chip.
     */
    BK4819() {
        initializeChip();
    }

    /**
     * @brief Performs the initial hardware setup for the BK4819 chip.
     * This includes soft reset, clearing interrupts, disabling unused components,
     * setting LDO voltages, configuring GPIOs, AGC, MIC sensitivity, and RF parameters.
     */
    void initializeChip() {
        // Perform a soft reset of the chip.
        softReset();

        // Clear any pending interrupts in REG_02 and REG_3F.
        spi.writeRegister(BK4819_REG_02, 0x0000);
        spi.writeRegister(BK4819_REG_3F, 0x0000);

        // Disable various internal blocks of the chip to save power or set a known state.
        // (e.g., VCO calibration, RX link, AF DAC, PLL, PA, MIC ADC, DSPs)
        spi.writeRegister(BK4819_REG_30,
            BK4819_REG_30_DISABLE_VCO_CALIB |
            BK4819_REG_30_DISABLE_RX_LINK |
            BK4819_REG_30_DISABLE_AF_DAC |
            BK4819_REG_30_DISABLE_DISC_MODE |
            BK4819_REG_30_DISABLE_PLL_VCO |
            BK4819_REG_30_DISABLE_PA_GAIN |
            BK4819_REG_30_DISABLE_MIC_ADC |
            BK4819_REG_30_DISABLE_TX_DSP |
            BK4819_REG_30_DISABLE_RX_DSP
        );

        // Configure LDO (Low Drop-Out regulator) settings and enable core blocks.
        // Refer to the BK4819 datasheet for details on REG_37 bits.
        /*  REG_37<14:12>   001 DSP voltage setting
            REG_37<11>      1   ANA LDO selection: 1: 2.7 V / 0: 2.4 V
            ... (other bit descriptions) ...
        */
        spi.writeRegister(BK4819_REG_37, 0x1D0F); // Example: 0001110100001111

        // Configure Power Amplifier (PA) related settings.
        spi.writeRegister(BK4819_REG_36, 0x0022);

        // Initialize GPIO output state register (REG_33).
        gpioOutState = 0x9000; // Default GPIO output values.
        spi.writeRegister(BK4819_REG_33, gpioOutState);

        // Configure Automatic Gain Control (AGC).
        setAGC(true, 18); // Use default AGC settings with gain index 18.

        // Configure Automatic MIC PGA (Programmable Gain Amplifier) Gain Controller.
        spi.writeRegister(BK4819_REG_19, 0x1041);
        // Set MIC sensitivity.
        spi.writeRegister(BK4819_REG_7D, 0xE94F);
        
        // Configure RF parameters.
        spi.writeRegister(BK4819_REG_1F, 0x5454); // RF related settings.
        // Configure band selection threshold for RF frontend.
        spi.writeRegister(BK4819_REG_3E, 0xA037);
    }

    // ------------------------------------------------------------------------

    /**
     * @brief Sets up various registers for general radio operation.
     * This includes GPIOs for LNA/PA control, interrupt masks, MIC gain,
     * AF gain, and disabling features like DTMF.
     * Should be called after `initializeChip` and before specific RX/TX setup.
     */
    void setupRegisters(void) {
        // Initialize GPIOs for LEDs and PA enable.
        toggleGreen(false);
        toggleRed(false);
        toggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false); // PA disabled initially.

        // Wait for any pending operations to complete (checks REG_0C).
        while (spi.readRegister(BK4819_REG_0C) & 1U) {
            spi.writeRegister(BK4819_REG_02, 0); // Clear interrupts.
            delayMs(1); // Short delay.
        }
        spi.writeRegister(BK4819_REG_3F, 0); // Clear interrupt mask.
        spi.writeRegister(BK4819_REG_7D, 0xE94F | 10); // Set MIC sensitivity.
        
        // Configure TX audio path (e.g., response/emphasis).
        spi.writeRegister(BK4819_REG_74, 0xAF1F); // 3kHz response for TX.

        // Enable RX path via GPIO.
        toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true);

        // Configure RX AF (Audio Frequency) levels (REG_48).
        // See datasheet for bitfield details: AF Rx Gain-1, Gain-2, DAC Gain.
        spi.writeRegister(BK4819_REG_48, 
            (11u << 12) | // Unknown bits
            (0u << 10)  | // AF Rx Gain-1 (0dB)
            (50u << 4)  | // AF Rx Gain-2 (example value)
            (0u << 0));   // AF DAC Gain (min)

        disableDTMF(); // Ensure DTMF decoder is off by default.

        // Configure squelch related interrupt (REG_40), possibly for RSSI/noise detection.
        spi.writeRegister(BK4819_REG_40, (spi.readRegister(BK4819_REG_40) & ~(0b11111111111)) |
            1000 | (1 << 12)); // Example value, specific meaning depends on datasheet.
    }

    /**
     * @brief Configures the Automatic Gain Control (AGC) settings.
     * @param useDefault If true, uses predefined default AGC settings. If false, allows custom low/high thresholds.
     * @param gainIndex Index into the `gainTable` for fixed gain mode, or 18 for auto AGC.
     */
    void setAGC(bool useDefault, uint8_t gainIndex) {
        const uint8_t GAIN_AUTO = 18; // Special index for automatic AGC.
        const bool enableAgc = gainIndex == GAIN_AUTO; // True if auto AGC is selected.
        uint16_t regVal = spi.readRegister(BK4819_REG_7E); // Read current AGC config.

        // Set AGC mode (fixed or auto) and fix index.
        spi.writeRegister(BK4819_REG_7E, static_cast<uint16_t>((regVal & ~(1 << 15) & ~(0b111 << 12)) |
            (!enableAgc << 15) | // Bit 15: 0 for AGC fix mode, 1 for auto.
            (3u << 12))          // Bits 14-12: AGC fix index (example value).
        );

        // Set gain values based on whether AGC is auto or fixed.
        if (enableAgc) { // Auto AGC
            spi.writeRegister(BK4819_REG_13, 0x03BE); // Default auto AGC gain settings.
        }
        else { // Fixed gain
            spi.writeRegister(BK4819_REG_13,
                gainTable[gainIndex] | 6 | (3 << 3)); // Use gain from `gainTable`.
        }
        // Other related AGC registers.
        spi.writeRegister(BK4819_REG_12, 0x037B);
        spi.writeRegister(BK4819_REG_11, 0x027B);
        spi.writeRegister(BK4819_REG_10, 0x007A);

        // AGC attack/decay thresholds.
        uint8_t Lo = 0;    // AGC speed: 0-1 auto, 2 low, 3 high.
        uint8_t low = 56;  // Low signal threshold.
        uint8_t high = 84; // High signal threshold.

        if (useDefault) {
            spi.writeRegister(BK4819_REG_14, 0x0019); // Default thresholds.
        }
        else { // Custom thresholds for potentially faster/slower AGC.
            spi.writeRegister(BK4819_REG_14, 0x0000);
            low = 20;
            high = 50;
        }
        spi.writeRegister(BK4819_REG_49, (Lo << 14) | (high << 7) | (low << 0)); // Write thresholds to register.
        spi.writeRegister(BK4819_REG_7B, 0x8420); // Additional AGC settings.
    }

    /**
     * @brief Sets the IF (Intermediate Frequency) filter bandwidth.
     * @param bw The desired filter bandwidth from the BK4819_Filter_Bandwidth enum.
     * This function configures REG_43 based on lookup tables for RF, WB, AF, and BS parameters.
     * TODO: The "fix" comment suggests this implementation might need review or completion.
     */
    void setFilterBandwidth(BK4819_Filter_Bandwidth bw) {
        // TODO: fix - This comment indicates the implementation might be incomplete or require verification.
        uint8_t bandwidth_index = (uint8_t)bw;
        if (bandwidth_index > 9) return; // Validate input.

        // Lookup tables for different filter parameters based on the selected bandwidth index.
        // These values are specific to the BK4819 and its register settings.
        static const uint8_t rf_filter_bw_vals[] = { 7, 5, 4, 3, 2, 1, 3, 1, 1, 0 }; // For REG_43<14:12>
        static const uint8_t weak_sig_rf_bw_vals[] = { 6, 4, 3, 2, 2, 1, 2, 1, 0, 0 }; // For REG_43<11:9>
        static const uint8_t af_tx_lpf2_bw_vals[] = { 4, 5, 6, 7, 0, 0, 3, 0, 2, 1 }; // For REG_43<8:6>
        static const uint8_t bw_mode_sel_vals[] = { 2, 2, 2, 2, 2, 2, 0, 0, 1, 1 }; // For REG_43<5:4>

        // Construct the value for REG_43.
        const uint16_t reg_43_value =
            (0u << 15) |     // Bit 15: Reserved or specific function.
            (rf_filter_bw_vals[bandwidth_index] << 12) |    // RF filter bandwidth.
            (weak_sig_rf_bw_vals[bandwidth_index] << 9) |   // RF filter bandwidth for weak signals.
            (af_tx_lpf2_bw_vals[bandwidth_index] << 6) |   // AF TX LPF2 filter bandwidth.
            (bw_mode_sel_vals[bandwidth_index] << 4) |     // BW Mode Selection (12.5k, 6.25k, 25k/20k).
            (1u << 3) |      // Bit 3: Specific function (e.g., enable).
            (0u << 2) |      // Bit 2: Gain after FM Demodulation (0=0dB, 1=6dB).
            (0u << 0);       // Bits 1:0: Reserved or specific function.
        
        spi.writeRegister(BK4819_REG_43, reg_43_value);
    }

    /**
     * @brief Sets the squelch type/logic.
     * @param t The desired squelch type from the SquelchType enum.
     */
    void squelchType(SquelchType t) {
        // Uses a helper `setRegValue` (likely defined in bk4819-regs.h or similar)
        // to write to a specific bitfield (RS_SQ_TYPE) in a register.
        setRegValue(RS_SQ_TYPE, squelchTypeValues[(uint8_t)t]);
    }

    /**
     * @brief Tunes the radio to a specific frequency.
     * @param frequency The desired frequency in Hz.
     * @param precise If true, enables VCO calibration for precise tuning.
     */
    void tuneTo(uint32_t frequency, bool precise) {
        selectFilter(frequency); // Select appropriate RF frontend filter based on frequency.
        setFrequency(frequency); // Program the PLL with the target frequency.
        
        uint16_t reg_30_val = spi.readRegister(BK4819_REG_30); // Read current REG_30.
        if (precise) {
            // Enable VCO calibration for precise tuning.
            // The sequence 0x0200 then original value might be a specific calibration trigger.
            spi.writeRegister(BK4819_REG_30, 0x0200); // Specific value, possibly related to VCO cal.
        }
        else {
            // Disable VCO calibration if not precise tuning.
            spi.writeRegister(BK4819_REG_30, reg_30_val & ~BK4819_REG_30_ENABLE_VCO_CALIB);
        }
        spi.writeRegister(BK4819_REG_30, reg_30_val); // Restore/apply REG_30 settings.
    }

    /**
     * @brief Turns on the RX path and enables necessary blocks for reception.
     */
    void rxTurnOn(void) {
        spi.writeRegister(BK4819_REG_37, 0x1F0F); // Enable LDOs and core blocks for RX.
        spi.writeRegister(BK4819_REG_30, 0x0000); // Initial state for REG_30 before enabling specific blocks.
        delayMs(10); // Short delay for stabilization.
        // Enable RX specific blocks in REG_30.
        spi.writeRegister(
            BK4819_REG_30,
            BK4819_REG_30_ENABLE_VCO_CALIB | BK4819_REG_30_DISABLE_UNKNOWN | // Unknown bit, kept disabled.
            BK4819_REG_30_ENABLE_RX_LINK | BK4819_REG_30_ENABLE_AF_DAC |
            BK4819_REG_30_ENABLE_DISC_MODE | BK4819_REG_30_ENABLE_PLL_VCO |
            BK4819_REG_30_DISABLE_PA_GAIN | BK4819_REG_30_DISABLE_MIC_ADC | // Disable TX blocks.
            BK4819_REG_30_DISABLE_TX_DSP | BK4819_REG_30_ENABLE_RX_DSP);   // Enable RX DSP.
    }

    /**
     * @brief Sets the Audio Filter (AF) mode.
     * @param af The desired AF mode from the BK4819_AF enum.
     */
    void setAF(BK4819_AF af) {
        // Configures REG_47 for the specified AF mode.
        // 0x6040 seems to be a base value for this register.
        spi.writeRegister(BK4819_REG_47, 0x6040 | ((uint16_t)af << 8));
    }

    /**
     * @brief Toggles a specific bit (bit 8) in the AF control register (REG_47).
     * The purpose of this bit is not specified here but is likely an AF feature enable/disable.
     * @param on True to set the bit, false to clear it.
     */
    void toggleAFBit(bool on) {
        uint16_t reg = spi.readRegister(BK4819_REG_47);
        reg &= ~(1 << 8); // Clear bit 8.
        if (on)
            reg |= (1 << 8); // Set bit 8 if `on` is true.
        spi.writeRegister(BK4819_REG_47, reg);
    }

    /**
     * @brief Toggles the AF DAC (Digital-to-Analog Converter) enable bit.
     * @param on True to enable the AF DAC, false to disable it.
     */
    void toggleAFDAC(bool on) {
        uint16_t reg = spi.readRegister(BK4819_REG_30);
        reg &= ~BK4819_REG_30_ENABLE_AF_DAC; // Clear the AF DAC enable bit.
        if (on)
            reg |= BK4819_REG_30_ENABLE_AF_DAC; // Set the bit if `on` is true.
        spi.writeRegister(BK4819_REG_30, reg);
    }

    /**
     * @brief Checks if the squelch is currently open.
     * @return True if squelch is open (signal detected), false otherwise.
     */
    bool isSquelchOpen(void) {
        // Reads REG_0C, bit 1 indicates squelch status.
        return (spi.readRegister(BK4819_REG_0C) >> 1) & 1;
    }

    /**
     * @brief Configures the squelch parameters using values from SQObject.
     * @param sql The user-selected squelch level (0-10).
     * @param f The current frequency (unused in this implementation, marked with attribute).
     * @param OpenDelay Squelch open delay setting.
     * @param CloseDelay Squelch close delay setting.
     * TODO: The "fix" comment indicates this function needs review, especially band selection.
     */
    void squelch(uint8_t sql, __attribute__((unused))uint32_t f, uint8_t OpenDelay,
        uint8_t CloseDelay) {
        // TODO: fix - Band selection based on frequency `f` is commented out.
        const uint8_t band_index = 0; // Default to band 0.
        // Setup squelch registers using values from the SQObject table.
        setupSquelch(SQObject::SQ[band_index][0][sql], SQObject::SQ[band_index][1][sql], 
                     SQObject::SQ[band_index][2][sql], SQObject::SQ[band_index][3][sql], 
                     SQObject::SQ[band_index][4][sql], SQObject::SQ[band_index][5][sql],
                     OpenDelay, CloseDelay);
    }

    /**
     * @brief Sets the BK4819 chip to an idle state by clearing REG_30.
     * This typically disables most active blocks like RX/TX DSP, DACs, etc.
     */
    void setIdle(void) {
        spi.writeRegister(BK4819_REG_30, 0x0000); // Disable all blocks in REG_30.
    }

    /**
     * @brief Writes a configuration value to the tone generation register (REG_71).
     * @param toneConfig The configuration value for the tone generator.
     */
    void setToneRegister(uint16_t toneConfig) {
        spi.writeRegister(BK4819_REG_71, toneConfig);
    }

    /**
     * @brief Sets the frequency for Tone 1.
     * @param f The desired frequency in Hz. The value is scaled before writing to the register.
     */
    void setToneFrequency(uint16_t f) {
        setToneRegister(scaleFreq(f)); // Scale frequency and write to REG_71.
    }

    /**
     * @brief Sets the frequency for Tone 2.
     * @param f The desired frequency in Hz. The value is scaled before writing to the register.
     */
    void setTone2Frequency(uint16_t f) {
        spi.writeRegister(BK4819_REG_72, scaleFreq(f)); // Scale frequency and write to REG_72.
    }

    /**
     * @brief Puts the transmitter in a muted state (e.g., during tone generation).
     * Writes a specific value to REG_50.
     */
    void enterTxMute(void) {
        spi.writeRegister(BK4819_REG_50, 0xBB20); // Value to mute TX.
    }

    /**
     * @brief Takes the transmitter out of the muted state.
     * Writes a specific value to REG_50.
     */
    void exitTxMute(void) {
        spi.writeRegister(BK4819_REG_50, 0x3B20); // Value to unmute TX.
    }

    /**
     * @brief Reads the current configuration of the tone generation register (REG_71).
     * @return The 16-bit value of REG_71.
     */
    uint16_t getToneRegister(void) {
        return spi.readRegister(BK4819_REG_71);
    }

    /**
     * @brief Plays a single tone at the specified frequency.
     * @param frequency The frequency of the tone to play in Hz.
     * @param bTuningGainSwitch Boolean to select between two gain settings for the tone.
     */
    void playTone(uint16_t frequency, bool bTuningGainSwitch) {
        enterTxMute(); // Mute main TX audio.
        setAF(BK4819_AF::BEEP); // Set AF mode to BEEP for tone generation.

        // Select tone gain.
        uint8_t gain = bTuningGainSwitch ? 28 : 96; 

        // Configure REG_70 to enable Tone 1 and set its gain.
        uint16_t toneCfg = BK4819_REG_70_ENABLE_TONE1 |
            (gain << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN);
        spi.writeRegister(BK4819_REG_70, toneCfg);

        setIdle(); // Go to idle state.
        // Enable AF DAC, DISC_MODE (discriminator output?), and TX_DSP for tone output.
        spi.writeRegister(BK4819_REG_30, 0 | BK4819_REG_30_ENABLE_AF_DAC |
            BK4819_REG_30_ENABLE_DISC_MODE |
            BK4819_REG_30_ENABLE_TX_DSP);

        setToneFrequency(frequency); // Set the frequency of Tone 1.
    }

    /**
     * @brief Turns off any active tones and re-enables the RX path.
     */
    void turnsOffTonesTurnsOnRX(void) {
        spi.writeRegister(BK4819_REG_70, 0); // Disable tone generator.
        setAF(BK4819_AF::MUTE); // Mute AF path initially.
        exitTxMute(); // Unmute main TX audio (though going to RX).
        setIdle();    // Go to idle.
        // Reconfigure REG_30 for normal RX operation.
        spi.writeRegister(
            BK4819_REG_30, 0 | 
            BK4819_REG_30_ENABLE_VCO_CALIB | 
            BK4819_REG_30_ENABLE_RX_LINK |
            BK4819_REG_30_ENABLE_AF_DAC | 
            BK4819_REG_30_ENABLE_DISC_MODE |
            BK4819_REG_30_ENABLE_PLL_VCO | 
            BK4819_REG_30_ENABLE_RX_DSP);
    }

    /**
     * @brief Sets the modulation type (FM, AM, SSB, WFM).
     * @param type The desired modulation type from the ModType enum.
     */
    void setModulation(ModType type) {
        bool isSsb = type == ModType::MOD_LSB || type == ModType::MOD_USB;
        bool isFm = type == ModType::MOD_FM || type == ModType::MOD_WFM;

        // Set AF path based on modulation type using a lookup table.
        setAF((BK4819_AF)modTypeRegValues[(uint8_t)type]);
        // Set AF DAC gain (specific value 0x8).
        setRegValue(afDacGainRegSpec, 0x8); // afDacGainRegSpec is likely a RegisterSpec struct.
        // Configure REG_3D based on whether it's SSB or not.
        spi.writeRegister(0x3D, isSsb ? 0 : 0x2AAB); 
        // Disable AFC (Automatic Frequency Control) if not FM.
        setRegValue(afcDisableRegSpec, !isFm); // afcDisableRegSpec is likely a RegisterSpec.

        // Specific settings for Wideband FM (WFM).
        if (type == ModType::MOD_WFM) {
            setRegValue(RS_XTAL_MODE, 0);     // Crystal mode.
            setRegValue(RS_IF_F, 14223);      // IF frequency.
            setRegValue(RS_RF_FILT_BW, 7);    // RF filter bandwidth.
            setRegValue(RS_RF_FILT_BW_WEAK, 7); // RF filter bandwidth for weak signals.
            setRegValue(RS_BW_MODE, 3);       // Bandwidth mode.
        }
        else { // Settings for other modulation types (FM, AM, SSB).
            setRegValue(RS_XTAL_MODE, 2);
            setRegValue(RS_IF_F, 10923);
        }
    }

    /**
     * @brief Resets the RSSI circuitry.
     * This is done by toggling a bit in REG_30.
     */
    void resetRSSI(void) {
        uint16_t reg = spi.readRegister(BK4819_REG_30);
        reg &= ~1; // Clear bit 0.
        spi.writeRegister(BK4819_REG_30, reg);
        reg |= 1;  // Set bit 0.
        spi.writeRegister(BK4819_REG_30, reg);
    }

    /**
     * @brief Reads the current RSSI (Received Signal Strength Indicator) value.
     * @return The raw RSSI value from REG_67 (lower 9 bits).
     */
    uint16_t getRSSI(void) {
        return spi.readRegister(BK4819_REG_67) & 0x1FF; // Mask to get 9-bit RSSI.
    }

    /**
     * @brief Reads the current noise level.
     * @return The noise level from REG_65 (lower 8 bits).
     */
    uint8_t getNoise(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_65) & 0xFF;
    }

    /**
     * @brief Reads a relative RSSI value (potentially an offset or different measurement).
     * @return The relative RSSI value from REG_65 (upper 8 bits).
     */
    uint8_t getRSSIRelative(void) {
        return static_cast<uint8_t>(spi.readRegister(BK4819_REG_65) >> 8) & 0xFF;
    }

    /**
     * @brief Reads the current glitch count/level.
     * @return The glitch value from REG_63 (lower 8 bits).
     */
    uint8_t getGlitch(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_63) & 0xFF;
    }

    /**
     * @brief Reads the current SNR (Signal-to-Noise Ratio) value.
     * @return The SNR value from REG_61 (lower 8 bits).
     */
    uint8_t getSNR(void) {
        return (uint8_t)spi.readRegister(BK4819_REG_61) & 0xFF;
    }

    /**
     * @brief Reads the current voice amplitude (demodulated audio level).
     * @return The voice amplitude value from REG_64.
     */
    uint16_t getVoiceAmplitude(void) {
        return spi.readRegister(BK4819_REG_64);
    }

    /**
     * @brief Disables VOX (Voice Operated Transmission).
     */
    void disableVox(void) {
        uint16_t v = spi.readRegister(BK4819_REG_31);
        spi.writeRegister(BK4819_REG_31, v & 0xFFFB); // Clear VOX enable bit (bit 2).
    }

    /**
     * @brief Disables DTMF (Dual-Tone Multi-Frequency) decoding.
     */
    void disableDTMF(void) {
        spi.writeRegister(BK4819_REG_24, 0); // Clear DTMF control register.
    }

    /**
     * @brief Reads the interrupt request status register (REG_0C).
     * @return The 16-bit value of REG_0C, indicating active interrupt sources.
     */
    uint16_t getInterruptRequest(void) {
        return spi.readRegister(BK4819_REG_0C);
    }

    /**
     * @brief Clears pending interrupts by writing to REG_02.
     * Typically, writing the interrupt bits back to this register clears them.
     */
    void clearInterrupt(void) {
        spi.writeRegister(BK4819_REG_02, 0); // Writing 0 might clear all, or specific bits need to be written.
                                            // Datasheet would clarify if specific bits need to be 1 to clear.
    }

    /**
     * @brief Reads the interrupt status register (REG_02).
     * This register usually shows which interrupts have occurred.
     * @return The 16-bit value of REG_02.
     */
    uint16_t readInterrupt(void) {
        return spi.readRegister(BK4819_REG_02);
    }

    /**
     * @brief Sets the interrupt enable mask (REG_3F).
     * @param mask A bitmask where each bit corresponds to an interrupt source. Setting a bit enables that interrupt.
     */
    void setInterrupt(uint16_t mask) {
        spi.writeRegister(BK4819_REG_3F, mask);
    }

    /**
     * @brief Toggles the green LED connected to a BK4819 GPIO pin.
     * @param on True to turn the green LED on, false to turn it off.
     */
    void toggleGreen(bool on) {
        toggleGpioOut(BK4819_GPIO6_PIN2_GREEN, on);
    }

    /**
     * @brief Toggles the red LED connected to a BK4819 GPIO pin.
     * @param on True to turn the red LED on, false to turn it off.
     */
    void toggleRed(bool on) {
        toggleGpioOut(BK4819_GPIO5_PIN1_RED, on);
    }

    /**
     * @brief Sets the CDCSS (Continuous Digital-Coded Squelch System) code word and related parameters.
     * @param CodeWord The 23-bit CDCSS code word.
     */
    void setCDCSSCodeWord(uint32_t CodeWord) {
        // Configure REG_51 for CDCSS operation (23-bit, positive polarity, auto BW, specific gain).
        spi.writeRegister(
            BK4819_REG_51,
            0 | BK4819_REG_51_ENABLE_CxCSS | BK4819_REG_51_GPIO6_PIN2_NORMAL |
            BK4819_REG_51_TX_CDCSS_POSITIVE | BK4819_REG_51_MODE_CDCSS |
            BK4819_REG_51_CDCSS_23_BIT | BK4819_REG_51_1050HZ_NO_DETECTION |
            BK4819_REG_51_AUTO_CDCSS_BW_ENABLE |
            BK4819_REG_51_AUTO_CTCSS_BW_ENABLE |
            (51U << BK4819_REG_51_SHIFT_CxCSS_TX_GAIN1));

        // Set a fixed frequency control word for CTCSS1 (REG_07), value 2775.
        // This might be a prerequisite or related setting for CDCSS.
        spi.writeRegister(BK4819_REG_07,
            0 | BK4819_REG_07_MODE_CTC1 |
            (2775U << BK4819_REG_07_SHIFT_FREQUENCY));

        // Set the CDCSS code word in REG_08. It's a 23-bit code, written in two parts.
        spi.writeRegister(BK4819_REG_08, (CodeWord >> 0) & 0xFFF);       // Lower 12 bits.
        spi.writeRegister(BK4819_REG_08, 0x8000 | ((CodeWord >> 12) & 0xFFF)); // Upper 11 bits, with bit 15 set to indicate write to upper part.
    }

    /**
     * @brief Sets the CTCSS (Continuous Tone-Coded Squelch System) frequency.
     * @param FreqControlWord A pre-calculated control word representing the CTCSS frequency.
     *                        A special value (2625) enables 1050Hz detection mode.
     */
    void setCTCSSFrequency(uint32_t FreqControlWord) {
        uint16_t Config;

        if (FreqControlWord == 2625) { // Special case for 1050Hz detection.
            Config = 0x944A; // Enable TxCTCSS, CTCSS Mode, 1050/4 Detect Enable, Auto BW, Gain.
        }
        else { // Standard CTCSS frequency.
            Config = 0x904A; // Enable TxCTCSS, CTCSS Mode, Auto BW, Gain.
        }
        spi.writeRegister(BK4819_REG_51, Config); // Write configuration to REG_51.

        // Write the scaled frequency control word to REG_07 for CTC1.
        // The scaling factor (2065/1000) converts the input FreqControlWord to the chip's internal format.
        spi.writeRegister(BK4819_REG_07, uint16_t(0 | BK4819_REG_07_MODE_CTC1 |
            ((FreqControlWord * 2065) / 1000) // Scale factor
            << BK4819_REG_07_SHIFT_FREQUENCY));
    }

    /**
     * @brief Configures the CTCSS tail detection frequency (typically for CTC2).
     * @param freq_10Hz The tail frequency in units of 0.1 Hz (e.g., 550 for 55.0 Hz).
     */
    void setTailDetection(const uint32_t freq_10Hz) {
        // Configures REG_07 for CTC2 mode and calculates the frequency control word.
        // The formula (253910 + (freq_10Hz / 2)) / freq_10Hz is specific to the BK4819.
        spi.writeRegister(BK4819_REG_07,
            uint16_t(BK4819_REG_07_MODE_CTC2 | ((253910 + (freq_10Hz / 2)) / // Calculation with rounding
                freq_10Hz))); 
    }

    /**
     * @brief Checks if the compander (compressor/expander) is enabled.
     * @return True if compander is enabled (bit 3 of REG_31 is set), false otherwise.
     */
    bool companderEnabled(void) {
        return (spi.readRegister(BK4819_REG_31) & (1u << 3)) ? true : false;
    }

    /**
     * @brief Configures the audio compander (compressor for TX, expander for RX).
     * @param mode 0: OFF, 1: TX only, 2: RX only, 3: TX and RX.
     */
    void setCompander(const uint8_t mode) {
        const uint16_t r31 = spi.readRegister(BK4819_REG_31); // Read current REG_31.

        if (mode == 0) {    // Disable compander.
            spi.writeRegister(BK4819_REG_31, static_cast<uint16_t>(r31 & ~(1u << 3))); // Clear enable bit.
            return;
        }

        // Configure TX compressor settings (REG_29).
        // Ratio, 0dB point, noise point.
        const uint16_t compress_ratio = (mode == 1 || mode >= 3) ? 2 : 0;  // 2 = 2:1 ratio, 0 = Disable TX compress.
        const uint16_t compress_0dB = 86;
        const uint16_t compress_noise_dB = 64;
        spi.writeRegister(BK4819_REG_29,
            (compress_ratio << 14) |
            (compress_0dB << 7) |
            (compress_noise_dB << 0));

        // Configure RX expander settings (REG_28).
        // Ratio, 0dB point, noise point.
        const uint16_t expand_ratio = (mode >= 2) ? 1 : 0;   // 1 = 1:2 ratio, 0 = Disable RX expand.
        const uint16_t expand_0dB = 86;
        const uint16_t expand_noise_dB = 56;
        spi.writeRegister(BK4819_REG_28,
            (expand_ratio << 14) |
            (expand_0dB << 7) |
            (expand_noise_dB << 0));

        // Enable compander functionality in REG_31.
        spi.writeRegister(BK4819_REG_31, r31 | (1u << 3)); // Set enable bit.
    }

    /**
     * @brief Puts the BK4819 chip into a low-power sleep mode.
     * Disables most blocks via REG_30, adjusts LDOs/core blocks via REG_37,
     * and disables RX enable GPIO.
     */
    void setSleepMode(void) {
        spi.writeRegister(BK4819_REG_30, 0x0000); // Disable active blocks.
        spi.writeRegister(BK4819_REG_37, 0x1D00); // Configure LDOs/core for low power.
        toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, false); // Disable RX LNA enable.
    }

    /**
     * @brief Sets the BK4819 to normal operational mode (typically for RX).
     * Re-enables the RX path via GPIO. Further RX setup might be needed via `rxTurnOn`.
     */
    void setNormalMode(void) {        
        toggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, true); // Enable RX LNA.
        // Further RX setup (like calling rxTurnOn()) might be needed depending on previous state.
    }

private:
    // --- Private Constants ---
    static constexpr uint32_t frequencyMIN = 1600000;   ///< Minimum supported frequency (example).
    static constexpr uint32_t frequencyMAX = 134000000; ///< Maximum supported frequency (example).

    // Frequency boundaries for VHF/UHF LNA selection.
    static constexpr uint32_t VHF_UHF_BOUND1 = 24000000; 
    static constexpr uint32_t VHF_UHF_BOUND2 = 28000000;

    SPISoftwareInterface spi; ///< Instance of the software SPI interface for chip communication.
    uint16_t gpioOutState;    ///< Cached state of the GPIO output register (REG_33).

    // Lookup table for AGC fixed gain settings. Index corresponds to abstract gain level.
    static constexpr uint16_t gainTable[19] = {
        0x000, 0x100, 0x020, 0x200, 0x040, 0x220, 0x060, 0x240, 0x0A0, 0x260,
        0x1C0, 0x2A0, 0x2C0, 0x2E0, 0x360, 0x380, 0x3A0, 0x3C0, 0x3E0,
    };

    // Register values for different squelch types (written to RS_SQ_TYPE bitfield).
    static constexpr uint8_t squelchTypeValues[4] = { 0x88, 0xAA, 0xCC, 0xFF };

    // Lookup table to map ModType enum to BK4819_AF enum values for AF path configuration.
    static constexpr uint16_t modTypeRegValues[8] = {
        static_cast<uint16_t>(BK4819_AF::FM),      // MOD_FM
        static_cast<uint16_t>(BK4819_AF::AM),      // MOD_AM
        static_cast<uint16_t>(BK4819_AF::USB),     // MOD_LSB (uses USB AF path)
        static_cast<uint16_t>(BK4819_AF::USB),     // MOD_USB
        static_cast<uint16_t>(BK4819_AF::BYPASS),  // MOD_BYP
        static_cast<uint16_t>(BK4819_AF::RAW),     // MOD_RAW
        static_cast<uint16_t>(BK4819_AF::FM),      // MOD_WFM (uses FM AF path)
        static_cast<uint16_t>(BK4819_AF::RAW)      // MOD_PRST (uses RAW AF path)
    };

    // ------------------------------------------------------------------------
    // --- Internal Helper Methods ---
    // ------------------------------------------------------------------------

    /**
     * @brief Performs a software reset of the BK4819 chip.
     * Toggles the soft reset bit in REG_00.
     */
    void softReset() {
        spi.writeRegister(BK4819_REG_00, 0x8000); // Assert soft reset.
        spi.writeRegister(BK4819_REG_00, 0x0000); // De-assert soft reset.
    }

    /**
     * @brief Toggles a specific BK4819 GPIO output pin state.
     * Updates the cached `gpioOutState` and writes it to REG_33.
     * @param pin The GPIO pin to toggle (from BK4819_GPIO_PIN_t enum in bk4819-regs.h).
     * @param bSet True to set the pin high, false to set it low.
     */
    void toggleGpioOut(BK4819_GPIO_PIN_t pin, bool bSet) {
        // The GPIO pins in REG_33 are controlled by bits that are shifted.
        // (0x40u >> pin) calculates the bitmask for the given pin.
        if (bSet)
            gpioOutState |= static_cast<uint16_t>(0x40u >> pin); // Set the bit.
        else
            gpioOutState &= (uint16_t)~(0x40u >> pin); // Clear the bit.
        spi.writeRegister(BK4819_REG_33, gpioOutState); // Write updated state to register.
    }

    /**
     * @brief Sets the radio frequency by writing to PLL frequency registers.
     * @param frequency The desired frequency in Hz, split into two 16-bit words.
     */
    void setFrequency(uint32_t frequency) {
        spi.writeRegister(BK4819_REG_38, frequency & 0xFFFF); // Lower 16 bits of frequency.
        spi.writeRegister(BK4819_REG_39, static_cast<uint16_t>((frequency >> 16) & 0xFFFF)); // Upper 16 bits.
    }

    /**
     * @brief Selects the appropriate RF frontend filter (VHF or UHF LNA) based on frequency.
     * @param frequency The target frequency in Hz. If 0xFFFFFFFF, LNAs are turned off.
     */
    void selectFilter(uint32_t frequency) {
        if (frequency < VHF_UHF_BOUND2) { // VHF range
            toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, true);  // Enable VHF LNA.
            toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, false); // Disable UHF LNA.
        }
        else if (frequency == 0xFFFFFFFF) { // Turn off both LNAs.
            toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, false);
            toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, false);
        }
        else { // UHF range
            toggleGpioOut(BK4819_GPIO4_PIN32_VHF_LNA, false); // Disable VHF LNA.
            toggleGpioOut(BK4819_GPIO3_PIN31_UHF_LNA, true);  // Enable UHF LNA.
        }
    }

    /**
     * @brief Configures various squelch threshold registers.
     * @param ro RSSI open threshold.
     * @param rc RSSI close threshold.
     * @param no Noise open threshold.
     * @param nc Noise close threshold.
     * @param gc Glitch close threshold.
     * @param go Glitch open threshold.
     * @param delayO Squelch open delay.
     * @param delayC Squelch close delay.
     */
    void setupSquelch(uint8_t ro, uint8_t rc, uint8_t no, uint8_t nc,
        uint8_t gc, uint8_t go, uint8_t delayO,
        uint8_t delayC) {
        spi.writeRegister(BK4819_REG_4D, 0xA000 | gc); // Glitch close threshold and other settings.
        spi.writeRegister(
            BK4819_REG_4E, // Squelch delays and glitch open threshold.
            (1u << 14) |  // Unknown bit.
            static_cast<uint16_t>(delayO << 11) | // Squelch open delay.
            static_cast<uint16_t>(delayC << 9) |  // Squelch close delay.
            go);                                  // Glitch open threshold.
        spi.writeRegister(BK4819_REG_4F, (nc << 8) | no); // Noise close and open thresholds.
        spi.writeRegister(BK4819_REG_78, (ro << 8) | rc); // RSSI open and close thresholds.
    }

    /**
     * @brief Reads a specific bitfield from a BK4819 register.
     * Uses a RegisterSpec structure (defined in bk4819-regs.h) that specifies
     * the register number, bit offset, and mask for the field.
     * @param s The RegisterSpec structure for the field to read.
     * @return The value of the specified bitfield.
     */
    uint16_t getRegValue(RegisterSpec s) {
        return (spi.readRegister(s.num) >> s.offset) & s.mask;
    }

    /**
     * @brief Writes a value to a specific bitfield in a BK4819 register.
     * Uses a RegisterSpec structure. Performs a read-modify-write operation.
     * @param s The RegisterSpec structure for the field to write.
     * @param v The value to write to the bitfield.
     */
    void setRegValue(RegisterSpec s, uint16_t v) {
        uint16_t reg = spi.readRegister(s.num); // Read current register value.
        reg &= (uint16_t)~(s.mask << s.offset); // Clear the bits of the target field.
        spi.writeRegister(s.num, reg | (v << s.offset)); // Set the new value in the field and write back.
    }

    /**
     * @brief Scales a frequency value to the format required by BK4819 tone generator registers.
     * @param freq The frequency in Hz.
     * @return The scaled frequency value.
     */
    uint16_t scaleFreq(const uint16_t freq) {
        // Formula specific to BK4819 for tone frequency scaling.
        // ((freq * 1353245) + (1^16)) >> 17
        return static_cast<uint16_t>((((uint32_t)freq * 1353245u) + (1u << 16)) >> 17); // Includes rounding.
    }
};
