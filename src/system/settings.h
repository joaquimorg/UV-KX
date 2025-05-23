// -----------------------------------------------------------------------------------------
// This file defines the structure and management of persistent radio settings.
// It includes definitions for VFO parameters, general radio options, memory channels,
// and calibration data, along with the EEPROM layout for storing these settings.
// The `Settings` class provides methods to load, save, and manage these configurations.
// -----------------------------------------------------------------------------------------
#pragma once

#include <cstdint>  // For standard integer types like uint16_t, uint8_t
#include "bk4819.h" // For BK4819 specific types like BK4819_Filter_Bandwidth and ModType
#include "sys.h"    // For system-level definitions or utilities
#include "eeprom.h" // For EEPROM read/write operations

/*
    EEPROM Layout Overview:
    This comment block describes how data is organized within the EEPROM.

    [0x0000 - 0x004F] : Global Radio Settings (defined by SETTINGS struct, approx 80 bytes)
    [0x0050 - 0x1D0F] : Memory Channels (e.g., 230 channels * 32 bytes/channel = 7360 bytes = 0x1CC0 bytes)
                       (End address would be 0x0050 + 0x1CC0 - 1 = 0x1D0F)
    [0x1D10 - 0x1DFF] : Empty / Unused space
    [0x1E00 - ... ]   : Calibration Data
    ...
    [0x1FFF]          : End of a typical 8KB EEPROM (like 24C64)
*/

// Forward declaration for SystemTask to avoid circular dependency.
namespace System {
    class SystemTask;
}

/**
 * @brief Manages all persistent radio settings, including VFO configurations,
 * general radio options, and interaction with the EEPROM for storage.
 */
class Settings {
public:
    // --- String constants for UI display of setting options ---
    // These are newline-separated strings used by UI components (like SelectionList)
    // to display choices for various settings.

    static constexpr const char* squelchStr = "OFF\n1\n2\n3\n4\n5\n6\n7\n8\n9"; ///< Squelch level options.
    static constexpr const char* codetypeStr = "NONE\nCT\nDCS\n-DCS"; ///< CTCSS/DCS code type options (-DCS for inverted DCS).
    static constexpr const char* txrxStr = "OFF\nTX\nRX\nRX/TX"; ///< Options for features applicable to TX, RX, or both (e.g., compander).
    static constexpr const char* onoffStr = "OFF\nON"; ///< Simple ON/OFF options.
    static constexpr const char* powerStr = "LOW\nMID\nHIGH"; ///< Transmit power level options.
    static constexpr const char* offsetStr = "OFF\n+\n-"; ///< Repeater offset direction options.
    static constexpr const char* modulationStr = "FM\nAM\nLSB"; ///< Modulation type options (subset shown).
    // static constexpr const char* modulationStr = "FM\nAM\nLSB\nUSB\nBYP\nRAW\nWFM\nPRST"; // Full list
    static constexpr const char* bandwidthStr = "26\n23\n20\n17\n14\n12\n10\n9\n7\n6"; ///< Filter bandwidth options (in kHz).
    static constexpr const char* stepStr = "0.5\n1.0\n2.5\n5.0\n6.25\n10.0\n12.5\n15.0\n20.0\n25.0\n30.0\n50.0\n100.0\n500.0"; ///< Frequency step options (in kHz).
    static constexpr const char* AGCStr = "-43\n-40\n-38\n-35\n-33\n-30\n-28\n-25\n-23\n-20\n-18\n-15\n-13\n-11\n-9\n-6\n-4\n-2\nAUTO"; ///< AGC gain options (in dB, or AUTO).
    static constexpr const char* rogerStr = "OFF\nDEFAULT\nMOTO TPT"; ///< Roger beep type options.
    static constexpr const char* pttIDStr = "OFF\nQUINDAR\nUP CODE\nDOWN CODE\nUP & DOWN"; ///< PTT ID options.
    static constexpr const char* TXTimeoutStr = "30s\n1m\n2m\n4m\n6m\n8m"; ///< Transmit Time-Out Timer options.
    static constexpr const char* BacklightTimeStr = "OFF\nON\n5s\n10s\n15s\n20s\n30s\n1m\n2m\n4m"; ///< Backlight auto-off timer options.
    static constexpr const char* MicDBStr = "+1.1dB\n+4.0dB\n+8.0dB\n+12.0dB\n+15.1dB"; ///< Microphone gain options.
    static constexpr const char* BacklightModeStr = "OFF\nTX\nRX\nTX/RX"; ///< Backlight activation mode options.

    // --- CTCSS and DCS code tables ---
    /// Standard CTCSS tone frequencies in 0.1 Hz units (e.g., 670 for 67.0 Hz).
    static constexpr uint16_t CTCSSOptions[50] = {
        670,  693,  719,  744,  770,  797,  825,  854,  885,  915, /* ... rest of the values ... */ 2541 };
    /// Standard DCS codes (octal representation, e.g., 023 is 0x13).
    static constexpr uint16_t DCSOptions[104] = {
        0x0013, 0x0015, 0x0016, 0x0019, 0x001A, 0x001E, 0x0023, 0x0027, 0x0029, /* ... rest of the values ... */ 0x01EC };

    // --- Enumerations for various settings ---

    /** @brief Battery capacity types, affects voltage-to-percentage calculation. */
    enum class BatteryType : uint8_t {
        BAT_1600 = 0, ///< 1600 mAh battery.
        BAT_2200 = 1, ///< 2200 mAh battery.
        BAT_3500 = 2, ///< 3500 mAh battery (example, may not be standard).
    };

    /** @brief Transmitter output power levels. */
    enum class TXOutputPower : uint8_t {
        TX_POWER_LOW = 0,  ///< Low power.
        TX_POWER_MID = 1,  ///< Medium power.
        TX_POWER_HIGH = 2  ///< High power.
    };

    /** @brief Types of beep tones for feedback. */
    enum class BEEPType : uint8_t {
        BEEP_NONE = 0,
        BEEP_1KHZ_60MS_OPTIONAL = 1,
        // ... other beep types ...
        BEEP_880HZ_60MS_TRIPLE_BEEP = 9
    };

    /** @brief Operational states of the radio. */
    enum class RadioState : uint8_t {
        IDLE = 0,  ///< Radio is idle.
        RX_ON = 1, ///< Radio is actively receiving.
        TX_ON = 2, ///< Radio is transmitting.
    };

    /** @brief Identifiers for VFO A and VFO B, or none. */
    enum class VFOAB : uint8_t {
        VFOA = 0, ///< VFO A.
        VFOB = 1, ///< VFO B.
        NONE = 2  ///< No VFO selected or applicable.
    };

    /// Table of frequency step values in 0.1 kHz units (e.g., 50 for 5.0 kHz).
    static constexpr uint16_t  StepFrequencyTable[13] = {
        50,  100, 250, 500, 625, 1000, 1250, 1500, 2000, 2500, 5000, 10000, 50000 };

    /** @brief Frequency step sizes for tuning. */
    enum class Step : uint8_t {
        STEP_0_5kHz = 0, STEP_1_0kHz = 1, STEP_2_5kHz = 2, STEP_5_0kHz = 3,
        STEP_6_25kHz = 4, STEP_10_0kHz = 5, STEP_12_5kHz = 6, STEP_15_0kHz = 7,
        STEP_20_0kHz = 8, STEP_25_0kHz = 9, STEP_50_0kHz = 10, STEP_100_0kHz = 11,
        STEP_500_0kHz = 12,
    };

    /** @brief Repeater offset direction. */
    enum class OffsetDirection : uint8_t {
        OFFSET_NONE = 0,  ///< No offset.
        OFFSET_PLUS = 1,  ///< Positive offset.
        OFFSET_MINUS = 2, ///< Negative offset.
    };

    /** @brief Options for features applicable to TX, RX, or both (e.g., compander). */
    enum class TXRX : uint8_t {
        OFF = 0,   ///< Feature is off.
        TX = 1,    ///< Feature active on Transmit only.
        RX = 2,    ///< Feature active on Receive only.
        RX_TX = 3  ///< Feature active on both Transmit and Receive.
    };

    /** @brief Simple ON/OFF setting. */
    enum class ONOFF : uint8_t {
        OFF = 0,
        ON = 1
    };

    /** @brief CTCSS/DCS code types. */
    enum class CodeType : uint8_t {
        NONE = 0, ///< No tone/code.
        CT = 1,   ///< CTCSS (Continuous Tone-Coded Squelch System).
        DCS = 2,  ///< DCS (Digital-Coded Squelch), typically inverted.
        NDCS = 3  ///< DCS Normal (non-inverted).
    };

    /** @brief Transmit Time-Out Timer durations. */
    enum class TXTimeout : uint8_t {
        TX_TIMEOUT_30S = 0,  TX_TIMEOUT_60S = 1, TX_TIMEOUT_120S = 2,
        TX_TIMEOUT_240S = 3, TX_TIMEOUT_360S = 4, TX_TIMEOUT_480S = 5        
    };

    /** @brief Backlight auto-off timer durations. */
    enum class BacklightTime : uint8_t {
        BACKLIGHT_OFF = 0, BACKLIGHT_ON = 1, BACKLIGHT_5S = 2, BACKLIGHT_10S = 3,
        BACKLIGHT_15S = 4, BACKLIGHT_20S = 5, BACKLIGHT_30S = 6, BACKLIGHT_60S = 7,
        BACKLIGHT_120S = 8, BACKLIGHT_240S = 9
    };

    /** @brief Microphone gain levels (in dB). */
    enum class MicDB : uint8_t {
        MIC_DB_1 = 1, MIC_DB_2 = 2, MIC_DB_3 = 3, MIC_DB_4 = 4, MIC_DB_5 = 5
    };

    /** @brief Backlight activation modes. */
    enum class BacklightMode : uint8_t {
        BACKLIGHT_MODE_OFF = 0,   ///< Backlight always off.
        BACKLIGHT_MODE_TX = 1,    ///< Backlight on during TX.
        BACKLIGHT_MODE_RX = 2,    ///< Backlight on during RX.
        BACKLIGHT_MODE_TX_RX = 3  ///< Backlight on during TX or RX.
    };

    // --- Structures for settings data ---

    /**
     * @brief Structure representing frequency and associated code (CTCSS/DCS) settings.
     * Uses bitfields for compact storage.
     */
    struct FREQ {
        uint32_t frequency : 27; ///< Frequency in 10Hz units (allows up to ~1.3 GHz).
        CodeType codeType  : 4;  ///< Type of CTCSS/DCS code.
        uint8_t  code;           ///< Index into CTCSSOptions or DCSOptions table.
    } __attribute__((packed));   ///< Ensures the compiler packs the structure tightly (5 bytes).

    /**
     * @brief Structure representing a VFO's complete settings.
     * Includes RX/TX frequencies, name, channel info, and various radio parameters.
     * Uses bitfields for compact storage.
     */
    struct VFO {
        FREQ                    rx;             ///< RX frequency and code settings.
        FREQ                    tx;             ///< TX frequency and code settings.
        char                    name[10];       ///< VFO/Memory Name (max 9 chars + null).
        uint16_t                channel;        ///< Channel number (if VFO is linked to a memory channel, 0 otherwise).
        uint8_t                 squelch     : 4;///< Squelch level (0-9).
        Step                    step        : 4;///< Frequency step size.
        ModType                 modulation  : 4;///< Modulation type (FM, AM, LSB).
        BK4819_Filter_Bandwidth bw          : 4;///< IF Filter bandwidth.
        TXOutputPower           power       : 2;///< Transmit power level.
        OffsetDirection         shift       : 2;///< Repeater offset direction.
        ONOFF                   repeaterSte : 1;///< Repeater Squelch Tail Elimination (STE).
        ONOFF                   ste         : 1;///< Squelch Tail Elimination (direct).
        TXRX                    compander   : 2;///< Compander mode (Off, TX, RX, TX/RX).
        uint8_t                 roger       : 4;///< Roger beep type.
        uint8_t                 pttid       : 4;///< PTT ID type.
        uint8_t                 rxagc       : 6;///< RX Automatic Gain Control setting.
        uint8_t                 reserved1[5];   ///< Reserved for future use, ensure struct size.
    } __attribute__((packed)); ///< Ensures the compiler packs the structure tightly (32 bytes).

    // Compile-time assertion to ensure VFO struct is the expected size for EEPROM layout.
    static_assert(sizeof(VFO) == 32, "VFO struct size mismatch");

    /**
     * @brief Structure representing the main radio settings stored in EEPROM.
     * Includes general radio options, two VFOs, and memory channel information.
     * Uses bitfields for compact storage.
     */
    struct SETTINGS {
        uint16_t        version;            ///< Version number of the settings structure, for compatibility.
        BatteryType     batteryType     : 2;///< Selected battery type.
        ONOFF           busyLockout     : 1;///< Busy Channel Lockout status.
        ONOFF           beep            : 1;///< Key beep status.
        uint8_t         backlightLevel  : 4;///< Backlight brightness level.
        BacklightTime   backlightTime   : 4;///< Backlight auto-off timer duration.
        MicDB           micDB           : 4;///< Microphone gain level.
        uint8_t         lcdContrast     : 4;///< LCD contrast level.
        TXTimeout       txTOT           : 4;///< Transmit Time-Out Timer duration.
        ONOFF           batterySave     : 4;///< Battery save mode status.
        BacklightMode   backlightMode   : 2;///< Backlight activation mode.
        VFOAB           vfoSelected     : 2;///< Currently active VFO for user interaction (A or B).
        uint16_t        memory[2];          ///< Currently selected memory channel number for VFO A and VFO B.
        VFO             vfo[2];             ///< Settings for VFO A and VFO B.
        ONOFF           showVFO[2];         ///< For VFO A/B, whether to display VFO frequency or memory channel info.
        uint8_t         reserved1[4];       ///< Reserved for future use.
    } __attribute__((packed)); ///< Ensures the compiler packs the structure tightly (80 bytes).

    // Compile-time assertion to ensure SETTINGS struct is the expected size for EEPROM layout.
    static_assert(sizeof(SETTINGS) == 80, "SETTINGS struct size mismatch");

    SETTINGS radioSettings; ///< Instance of the main radio settings structure.

    /**
     * @brief Constructor for the Settings class.
     * @param systask Reference to the SystemTask, used for potential system interactions.
     */
    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}

    /** @brief Resets all settings to their factory defaults (not fully implemented). */
    void factoryReset() {}; // TODO: Implement factory reset logic.

    /** @brief Reads all radio settings from EEPROM into the `radioSettings` member. */
    void getRadioSettings() {
        eeprom.readBuffer(0x0000, &radioSettings, sizeof(SETTINGS)); // Read from EEPROM address 0.
    }

    /** @brief Writes the current `radioSettings` member to EEPROM. */
    void setRadioSettings() {
        eeprom.writeBuffer(0x0000, &radioSettings, sizeof(SETTINGS)); // Write to EEPROM address 0.
    }

    /** @brief Sets the `radioSettings` member to default factory values. */
    void setRadioSettingsDefault() {
        radioSettings.version           = settingsVersion;
        radioSettings.batteryType       = BatteryType::BAT_1600;
        // ... (all other default settings) ...
        radioSettings.showVFO[0]        = ONOFF::ON; 
        radioSettings.showVFO[1]        = ONOFF::ON; 
        // Note: Default VFO struct contents (radioSettings.vfo[0] and radioSettings.vfo[1])
        // should also be initialized here, perhaps by calling a VFO-specific default function.
    }

    /** @brief Gets the current version of the settings structure stored in `radioSettings`. */
    uint16_t getSettingsVersion() {
        return radioSettings.version;
    }

    /**
     * @brief Validates if the loaded settings version matches the firmware's expected version.
     * @return True if versions match, false otherwise.
     */
    bool validateSettingsVersion() {
        uint16_t version = getSettingsVersion();
        return version == settingsVersion;
    }

    /**
     * @brief Performs a step of EEPROM initialization (e.g., formatting or writing defaults).
     * @return Current progress of initialization (0-100).
     * TODO: Implement the actual EEPROM initialization logic.
     */
    uint8_t initEEPROM(void) {
        // This is a placeholder for progressive EEPROM initialization.
        // Actual implementation would involve writing default data in chunks.
        if (initBlock < maxBlock) {
            initBlock++;
        }
        return (uint8_t)((initBlock * 100) / maxBlock);
    }

    /** @brief Provides access to the underlying EEPROM object. */
    EEPROM& getEEPROM() {
        return eeprom;
    }

private:
    static constexpr uint16_t settingsVersion = 0x015A; ///< Current firmware's settings structure version.
    System::SystemTask& systask; ///< Reference to SystemTask.

    EEPROM eeprom; ///< EEPROM driver instance.

    // Variables for progressive EEPROM initialization.
    uint16_t initBlock = 0x0000;      ///< Current block being initialized.
    static constexpr uint16_t maxBlock = 0x000F; ///< Total blocks for initialization progress calculation.

    bool hasDataToSave = false; ///< Flag to indicate if settings have changed and need saving (not used in provided code).
};
