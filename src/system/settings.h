#pragma once

#include <cstdint>  // For standard integer types like uint16_t, uint8_t
#include <cstring>
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

namespace System {
    class SystemTask;
}

class Settings {
public:
    static constexpr uint16_t MAX_CHANNELS = 230;

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
    static constexpr const char* BacklightLevelStr = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10"; ///< Backlight brightness levels.
    static constexpr const char* LCDContrastStr = "100\n110\n120\n130\n140\n150\n160\n170\n180\n190\n200"; ///< LCD contrast options.

    static constexpr uint16_t CTCSSOptions[50] = {
        670,  693,  719,  744,  770,  797,  825,  854,  885,  915,
        948,  974,  1000, 1035, 1072, 1109, 1148, 1188, 1230, 1273,
        1318, 1365, 1413, 1462, 1514, 1567, 1598, 1622, 1655, 1679,
        1713, 1738, 1773, 1799, 1835, 1862, 1899, 1928, 1966, 1995,
        2035, 2065, 2107, 2181, 2257, 2291, 2336, 2418, 2503, 2541 };

    static constexpr uint16_t DCSOptions[104] = {
        0x0013, 0x0015, 0x0016, 0x0019, 0x001A, 0x001E, 0x0023, 0x0027, 0x0029,
        0x002B, 0x002C, 0x0035, 0x0039, 0x003A, 0x003B, 0x003C, 0x004C, 0x004D,
        0x004E, 0x0052, 0x0055, 0x0059, 0x005A, 0x005C, 0x0063, 0x0065, 0x006A,
        0x006D, 0x006E, 0x0072, 0x0075, 0x007A, 0x007C, 0x0085, 0x008A, 0x0093,
        0x0095, 0x0096, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A9, 0x00AA, 0x00AD,
        0x00B1, 0x00B3, 0x00B5, 0x00B6, 0x00B9, 0x00BC, 0x00C6, 0x00C9, 0x00CD,
        0x00D5, 0x00D9, 0x00DA, 0x00E3, 0x00E6, 0x00E9, 0x00EE, 0x00F4, 0x00F5,
        0x00F9, 0x0109, 0x010A, 0x010B, 0x0113, 0x0119, 0x011A, 0x0125, 0x0126,
        0x012A, 0x012C, 0x012D, 0x0132, 0x0134, 0x0135, 0x0136, 0x0143, 0x0146,
        0x014E, 0x0153, 0x0156, 0x015A, 0x0166, 0x0175, 0x0186, 0x018A, 0x0194,
        0x0197, 0x0199, 0x019A, 0x01AC, 0x01B2, 0x01B4, 0x01C3, 0x01CA, 0x01D3,
        0x01D9, 0x01DA, 0x01DC, 0x01E3, 0x01EC };

    enum class BatteryType : uint8_t {
        BAT_1600 = 0,
        BAT_2200 = 1,
        BAT_3500 = 2,
    };

    enum class TXOutputPower : uint8_t {
        TX_POWER_LOW = 0,
        TX_POWER_MID = 1,
        TX_POWER_HIGH = 2
    };

    enum class BEEPType : uint8_t {
        BEEP_NONE = 0,
        BEEP_1KHZ_60MS_OPTIONAL = 1,
        BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL = 2,
        BEEP_440HZ_500MS = 3,
        BEEP_880HZ_200MS = 4,
        BEEP_880HZ_500MS = 5,
        BEEP_500HZ_60MS_DOUBLE_BEEP = 6,
        BEEP_440HZ_40MS_OPTIONAL = 7,
        BEEP_880HZ_40MS_OPTIONAL = 8,
        BEEP_880HZ_60MS_TRIPLE_BEEP = 9
    };

    enum class RadioState : uint8_t {
        IDLE = 0,
        RX_ON = 1,
        TX_ON = 2,
    };

    enum class VFOAB : uint8_t {
        VFOA = 0,
        VFOB = 1,
        NONE = 2
    };

    static constexpr uint16_t  StepFrequencyTable[13] = {
        50,  100,
        250, 500, 625, 1000, 1250, 1500,
        2000, 2500, 5000, 10000, 50000
    };

    enum class Step : uint8_t {
        STEP_0_5kHz = 0,
        STEP_1_0kHz = 1,
        STEP_2_5kHz = 2,
        STEP_5_0kHz = 3,
        STEP_6_25kHz = 4,
        STEP_10_0kHz = 5,
        STEP_12_5kHz = 6,
        STEP_15_0kHz = 7,
        STEP_20_0kHz = 8,
        STEP_25_0kHz = 9,
        STEP_50_0kHz = 10,
        STEP_100_0kHz = 11,
        STEP_500_0kHz = 12,
    };

    enum class OffsetDirection : uint8_t {
        OFFSET_NONE = 0,
        OFFSET_PLUS = 1,
        OFFSET_MINUS = 2,
    };

    enum class TXRX : uint8_t {
        OFF = 0,
        TX = 1,
        RX = 2,
        RX_TX = 3
    };

    enum class ONOFF : uint8_t {
        OFF = 0,
        ON = 1
    };

    enum class CodeType : uint8_t {
        NONE = 0,
        CT = 1,
        DCS = 2,
        NDCS = 3
    };

    enum class TXTimeout : uint8_t {
        TX_TIMEOUT_30S = 0,  // 30 seconds
        TX_TIMEOUT_60S = 1,  // 1 minute
        TX_TIMEOUT_120S = 2, // 2 minute
        TX_TIMEOUT_240S = 3, // 4 minute
        TX_TIMEOUT_360S = 4, // 6 minute
        TX_TIMEOUT_480S = 5  // 8 minutes        
    };

    enum class BacklightTime : uint8_t {
        BACKLIGHT_OFF = 0,  // OFF
        BACKLIGHT_ON = 1,   // ON
        BACKLIGHT_5S = 2,   // 5 seconds
        BACKLIGHT_10S = 3,  // 10 seconds
        BACKLIGHT_15S = 4,  // 15 seconds
        BACKLIGHT_20S = 5,  // 20 seconds
        BACKLIGHT_30S = 6,  // 30 seconds
        BACKLIGHT_60S = 7,  // 1 minute
        BACKLIGHT_120S = 8, // 2 minute
        BACKLIGHT_240S = 9  // 4 minutes
    };

    enum class MicDB : uint8_t {
        MIC_DB_1 = 1, // +1.1dB
        MIC_DB_2 = 2, // +4.0dB
        MIC_DB_3 = 3, // +8.0dB
        MIC_DB_4 = 4, // +12.0dB
        MIC_DB_5 = 5  // +15.1dB
    };

    enum class BacklightMode : uint8_t {
        BACKLIGHT_MODE_OFF = 0, // OFF
        BACKLIGHT_MODE_TX = 1,  // TX
        BACKLIGHT_MODE_RX = 2,  // RX
        BACKLIGHT_MODE_TX_RX = 3 // TX/RX
    };

    struct FREQ {
        uint32_t frequency;     // 32-bit frequency
        CodeType codeType;      // Modulation code type
        uint8_t  code;          // Tone/DCS code
    } __attribute__((packed)); // 6 Bytes

    static_assert(sizeof(FREQ) == 6, "FREQ struct size mismatch");
    
    struct VFO {
        FREQ                    rx;             // RX Frequency
        FREQ                    tx;             // TX Frequency
        char                    name[10];       // Memory Name
        uint16_t                channel;        // Channel Number ??
        uint8_t                 squelch : 4;    // Squelch Level
        Step                    step : 4;       // Step Frequency
        ModType                 modulation : 4; // Modulation Type
        BK4819_Filter_Bandwidth bw : 4;         // Filter Bandwidth
        TXOutputPower           power : 2;      // TX Power Level
        OffsetDirection         shift : 2;      // Offset Direction
        ONOFF                   repeaterSte : 1;// Repeater STE
        ONOFF                   ste : 1;        // STE
        TXRX                    compander : 2;  // Compander
        uint8_t                 roger : 4;      // Roger Beep
        uint8_t                 pttid : 4;      // PTT ID
        uint8_t                 rxagc : 6;      // RX AGC Level
        uint8_t                 reserved1[3] = {0xFF};   // Reserved
    } __attribute__((packed)); // 32 Bytes

    static_assert(sizeof(VFO) == 32, "VFO struct size mismatch");

    // Explicit packed representation used in EEPROM to avoid bitfield endianness issues.
    struct PackedVFOData {
        uint32_t rx_frequency;
        uint8_t  rx_code_type;
        uint8_t  rx_code;
        uint32_t tx_frequency;
        uint8_t  tx_code_type;
        uint8_t  tx_code;
        char     name[10];
        uint16_t channel_id;
        uint8_t  squelch_step;
        uint8_t  modulation_bw;
        uint8_t  power_shift_misc;
        uint8_t  roger_pttid;
        uint8_t  rxagc_reserved;
        uint8_t  reserved_bytes[3];
    } __attribute__((packed));

    static_assert(sizeof(PackedVFOData) == 32, "PackedVFOData size mismatch");


    // MIC DB\nBATT SAVE\nBUSY LOCKOUT\nBCKLIGHT LEVEL\nBCKLIGHT TIME\nBCKLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP
    struct SETTINGS {
        uint16_t        version;                 // Settings Version
        BatteryType     batteryType : 2;         // Battery Type
        ONOFF           busyLockout : 1;         // Busy Lockout
        ONOFF           beep : 1;                // Beep
        uint8_t         backlightLevel : 4;      // Backlight Level
        BacklightTime   backlightTime : 4;       // Backlight Time
        MicDB           micDB : 4;               // Mic DB
        uint8_t         lcdContrast : 4;         // LCD Contrast
        TXTimeout       txTOT : 4;               // TX TOT
        ONOFF           batterySave : 4;         // Battery Save
        BacklightMode   backlightMode : 2;       // Backlight Mode
        VFOAB           vfoSelected : 2;         // VFO Selected - 0 = VFOA, 1 = VFOB
        uint16_t        memory[2];               // Memory Number
        VFO             vfo[2];                  // VFO Settings
        ONOFF           showVFO[2];              // Show VFO or Memory
        uint8_t         reserved1[4] = {0xFF};   // Reserved
    } __attribute__((packed));

    static_assert(sizeof(SETTINGS) == 80, "SETTINGS struct size mismatch");

    SETTINGS radioSettings;

    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}
    void factoryReset() {};

    void getRadioSettings() {
        eeprom.readBuffer(0x0000, &radioSettings, sizeof(SETTINGS));
        lastSavedRadioSettings = radioSettings;
    }

    void setRadioSettings() {
        eeprom.writeBuffer(0x0000, &radioSettings, sizeof(SETTINGS));
        lastSavedRadioSettings = radioSettings;
    }

    void setRadioSettingsDefault() {
        radioSettings.version           = settingsVersion;
        radioSettings.batteryType       = BatteryType::BAT_1600;
        radioSettings.busyLockout       = ONOFF::ON;
        radioSettings.beep              = ONOFF::ON;
        radioSettings.backlightLevel    = 0x0A; // 10
        radioSettings.backlightTime     = BacklightTime::BACKLIGHT_15S;
        radioSettings.micDB             = MicDB::MIC_DB_5; // +15.1dB
        radioSettings.lcdContrast       = 0x04; // 4
        radioSettings.txTOT             = TXTimeout::TX_TIMEOUT_120S;
        radioSettings.batterySave       = ONOFF::ON;
        radioSettings.backlightMode     = BacklightMode::BACKLIGHT_MODE_TX_RX;
        radioSettings.vfoSelected       = VFOAB::VFOA; // VFOA

        radioSettings.memory[0] = 0x0000; // VFOA Memory Number
        radioSettings.memory[1] = 0x0000; // VFOB Memory Number

        radioSettings.showVFO[0] = ONOFF::ON; // VFOA Show VFO
        radioSettings.showVFO[1] = ONOFF::ON; // VFOB Show VFO

        radioSettings.vfo[0].rx.frequency = 14500000; // VFOA RX Frequency
        radioSettings.vfo[0].rx.codeType = CodeType::NONE; // VFOA RX Code Type
        radioSettings.vfo[0].rx.code = 0; // VFOA RX Code
        radioSettings.vfo[0].tx.frequency = 14500000; // VFOA TX Frequency
        radioSettings.vfo[0].tx.codeType = CodeType::NONE; // VFOA TX Code Type
        radioSettings.vfo[0].tx.code = 0; // VFOA TX
        radioSettings.vfo[0].name[0] = '\0'; // VFOA Name
        radioSettings.vfo[0].channel = 0; // VFOA Channel Number
        radioSettings.vfo[0].squelch = 0x1; // VFOA Squelch OFF
        radioSettings.vfo[0].step = Step::STEP_5_0kHz; // VFOA Step 5.0kHz
        radioSettings.vfo[0].modulation = ModType::MOD_FM; // VFOA Modulation FM
        radioSettings.vfo[0].bw = BK4819_Filter_Bandwidth::BK4819_FILTER_BW_20k; // VFOA Bandwidth 20kHz
        radioSettings.vfo[0].power = TXOutputPower::TX_POWER_LOW; // VFOA Power Low
        radioSettings.vfo[0].shift = OffsetDirection::OFFSET_NONE; // VFOA Offset None

        radioSettings.vfo[1].rx.frequency = 44000000; // VFOB RX Frequency
        radioSettings.vfo[1].rx.codeType = CodeType::NONE; // VFOB RX Code Type
        radioSettings.vfo[1].rx.code = 0; // VFOB RX Code
        radioSettings.vfo[1].tx.frequency = 44000000; // VFOB TX Frequency
        radioSettings.vfo[1].tx.codeType = CodeType::NONE; // VFOB TX Code Type
        radioSettings.vfo[1].tx.code = 0; // VFOB TX
        radioSettings.vfo[1].name[0] = '\0'; // VFOB Name
        radioSettings.vfo[1].channel = 0; // VFOB Channel Number
        radioSettings.vfo[1].squelch = 0x1; // VFOB Squelch OFF
        radioSettings.vfo[1].step = Step::STEP_5_0kHz; // VFOB Step 5.0kHz
        radioSettings.vfo[1].modulation = ModType::MOD_FM; // VFOB Modulation FM
        radioSettings.vfo[1].bw = BK4819_Filter_Bandwidth::BK4819_FILTER_BW_20k; // VFOB Bandwidth 20kHz
        radioSettings.vfo[1].power = TXOutputPower::TX_POWER_LOW; // VFOB Power Low
        radioSettings.vfo[1].shift = OffsetDirection::OFFSET_NONE; // VFOB Offset None

        lastSavedRadioSettings = radioSettings;
    }

    uint16_t getSettingsVersion() {
        return radioSettings.version;
    }

    bool validateSettingsVersion() {
        uint16_t version = getSettingsVersion();
        if (version == settingsVersion) {
            return true;
        }
        return false;
    }

    uint8_t initEEPROM(void) {
        static constexpr uint16_t blockSize = 0x0200; // 512 bytes per block
        uint8_t buffer[blockSize];

        if (initBlock < maxBlock) {
            memset(buffer, 0xFF, sizeof(buffer));

            if (initBlock == 0) {
                setRadioSettingsDefault();
                memcpy(buffer, &radioSettings, sizeof(SETTINGS));
            }

            eeprom.writeBuffer(initBlock * blockSize, buffer, sizeof(buffer));
            initBlock++;
        }

        return static_cast<uint8_t>((initBlock * 100) / maxBlock);
    }

    void saveRadioSettings() {
        eeprom.writeBuffer(0x0000, &radioSettings, sizeof(SETTINGS));
        lastSavedRadioSettings = radioSettings;
    }

    void requestSaveRadioSettings() {
        if (memcmp(&radioSettings, &lastSavedRadioSettings, sizeof(SETTINGS)) != 0) {
            radioSavePending = true;
            radioSaveDelay = saveDelayTicks;
        }
    }

    /**
     * Request to save a memory channel
     * @param channelNumber Channel number (1-230)
     * @param vfoIndex VFO index (0 for VFOA, 1 for VFOB)
     */
    void requestSaveMemory(uint16_t channelNumber, uint8_t vfoIndex) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS || vfoIndex > 1) {
            return;
        }
        
        // Check if the VFO has actually changed compared to what's stored in EEPROM
        VFO currentChannelData;
        bool channelExists = readChannel(channelNumber, currentChannelData);
        
        // If channel doesn't exist or data has changed, schedule save
        if (!channelExists || memcmp(&radioSettings.vfo[vfoIndex], &currentChannelData, sizeof(VFO)) != 0) {
            pendingMemoryChannel = channelNumber;
            pendingMemoryVFO = vfoIndex;
            memorySavePending = true;
            memorySaveDelay = saveDelayTicks;
        }
    }

    void scheduleSaveIfNeeded();
    void scheduleMemorySaveIfNeeded(uint16_t channelNumber, uint8_t vfoIndex);

    /**
     * Check if a save is pending and handle it
     * This should be called periodically (e.g., in a loop or timer)
     */
    void handleSaveTimers() {
        // Handle radio settings save
        if (radioSavePending) {
            if (radioSaveDelay > 0) {
                --radioSaveDelay;
            } else {
                saveRadioSettings();
                radioSavePending = false;
            }
        }

        // Handle memory save
        if (memorySavePending) {
            if (memorySaveDelay > 0) {
                --memorySaveDelay;
            } else {
                // Save the pending memory channel
                if (pendingMemoryChannel >= 1 && pendingMemoryChannel <= MAX_CHANNELS && 
                    pendingMemoryVFO <= 1) {
                    saveVFOToChannel(pendingMemoryChannel, pendingMemoryVFO);
                }
                memorySavePending = false;
            }
        }
    }

    void applyRadioSettings();

    EEPROM& getEEPROM() {
        return eeprom;
    }


    bool isRadioSavePending() const {
        return radioSavePending;
    }

    bool isMemorySavePending() const {
        return memorySavePending;
    }

    /**
     * Read a channel from EEPROM
     * @param channelNumber Channel number (1-230)
     * @param channel Reference to VFO struct to store the data
     * @return true if successful, false if channel number is invalid
     */
    bool readChannel(uint16_t channelNumber, VFO& channel) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS) {
            return false;
        }

        uint16_t address = static_cast<uint16_t>(CHANNEL_START_ADDRESS + ((channelNumber - 1) * CHANNEL_SIZE));
        PackedVFOData packed{};
        eeprom.readBuffer(address, &packed, sizeof(PackedVFOData));

        channel.rx.frequency = packed.rx_frequency;
        channel.rx.codeType = static_cast<CodeType>(packed.rx_code_type);
        channel.rx.code = packed.rx_code;
        channel.tx.frequency = packed.tx_frequency;
        channel.tx.codeType = static_cast<CodeType>(packed.tx_code_type);
        channel.tx.code = packed.tx_code;

        memcpy(channel.name, packed.name, sizeof(channel.name));
        channel.name[sizeof(channel.name) - 1] = '\0';
        if (channel.name[0] == static_cast<char>(0xFF)) {
            channel.name[0] = '\0';
        }

        channel.channel = packed.channel_id;
        channel.squelch = (packed.squelch_step & 0x0F);
        channel.step = static_cast<Step>((packed.squelch_step >> 4) & 0x0F);
        channel.modulation = static_cast<ModType>(packed.modulation_bw & 0x0F);
        channel.bw = static_cast<BK4819_Filter_Bandwidth>((packed.modulation_bw >> 4) & 0x0F);

        uint8_t powerShift = packed.power_shift_misc;
        channel.power = static_cast<TXOutputPower>(powerShift & 0x03);
        channel.shift = static_cast<OffsetDirection>((powerShift >> 2) & 0x03);
        channel.repeaterSte = (powerShift & 0x10) ? ONOFF::ON : ONOFF::OFF;
        channel.ste = (powerShift & 0x20) ? ONOFF::ON : ONOFF::OFF;
        channel.compander = static_cast<TXRX>((powerShift >> 6) & 0x03);

        uint8_t rp = packed.roger_pttid;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wconversion"
        channel.roger = static_cast<decltype(channel.roger)>(rp & 0x0F);
        channel.pttid = static_cast<decltype(channel.pttid)>((rp >> 4) & 0x0F);
        #pragma GCC diagnostic pop
        channel.rxagc = packed.rxagc_reserved & 0x3F;
        memset(channel.reserved1, 0xFF, sizeof(channel.reserved1));
        return true;
    }

    /**
     * Write a channel to EEPROM
     * @param channelNumber Channel number (1-230)
     * @param channel Reference to VFO struct containing the data
     * @return true if successful, false if channel number is invalid
     */
    bool writeChannel(uint16_t channelNumber, const VFO& channel) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS) {
            return false;
        }

        uint16_t address = static_cast<uint16_t>(CHANNEL_START_ADDRESS + ((channelNumber - 1) * CHANNEL_SIZE));
        PackedVFOData packed{};

        packed.rx_frequency = channel.rx.frequency;
        packed.rx_code_type = static_cast<uint8_t>(channel.rx.codeType);
        packed.rx_code = channel.rx.code;
        packed.tx_frequency = channel.tx.frequency;
        packed.tx_code_type = static_cast<uint8_t>(channel.tx.codeType);
        packed.tx_code = channel.tx.code;
        memcpy(packed.name, channel.name, sizeof(packed.name));
        packed.channel_id = channel.channel;
        packed.squelch_step = static_cast<uint8_t>((channel.squelch & 0x0F) | (static_cast<uint8_t>(channel.step) << 4));
        packed.modulation_bw = static_cast<uint8_t>((static_cast<uint8_t>(channel.modulation) & 0x0F) |
                                                    ((static_cast<uint8_t>(channel.bw) & 0x0F) << 4));

        packed.power_shift_misc = static_cast<uint8_t>(
            (static_cast<uint8_t>(channel.power) & 0x03) |
            ((static_cast<uint8_t>(channel.shift) & 0x03) << 2) |
            ((channel.repeaterSte == ONOFF::ON ? 1 : 0) << 4) |
            ((channel.ste == ONOFF::ON ? 1 : 0) << 5) |
            ((static_cast<uint8_t>(channel.compander) & 0x03) << 6));

        packed.roger_pttid = static_cast<uint8_t>((channel.roger & 0x0F) | ((channel.pttid & 0x0F) << 4));
        packed.rxagc_reserved = static_cast<uint8_t>((channel.rxagc & 0x3F) | 0xC0); // keep reserved bits high
        packed.reserved_bytes[0] = 0xFF;
        packed.reserved_bytes[1] = 0xFF;
        packed.reserved_bytes[2] = 0xFF;

        eeprom.writeBuffer(address, &packed, sizeof(PackedVFOData));
        return true;
    }

    /**
     * Check if a channel is in use (has a non-empty name)
     * @param channelNumber Channel number (1-230)
     * @return true if channel is in use, false otherwise
     */
    bool isChannelInUse(uint16_t channelNumber) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS) {
            return false;
        }
        
        VFO channel;
        if (!readChannel(channelNumber, channel)) {
            return false;
        }
        
        // Check if name is not empty (at least first character is not null or space)
        return (channel.name[0] != '\0' && channel.name[0] != ' ');
    }

    /**
     * Get the next channel in use
     * @param currentChannel Current channel number
     * @return Next channel number in use, or first channel in use if at end
     */
    uint16_t getNextChannel(uint16_t currentChannel) {
        if (currentChannel < 1 || currentChannel > MAX_CHANNELS) {
            currentChannel = 1;
        }

        // Start searching from the next channel
        uint16_t searchChannel = static_cast<uint16_t>(currentChannel + 1);
        
        // Search forward from current position
        for (uint16_t i = 0; i < MAX_CHANNELS; i++) {
            if (searchChannel > MAX_CHANNELS) {
                searchChannel = 1; // Wrap around to beginning
            }
            
            if (isChannelInUse(searchChannel)) {
                return searchChannel;
            }
            
            searchChannel = static_cast<uint16_t>(searchChannel + 1);
        }
        
        // If no channels are in use, return channel 1
        return 1;
    }

    /**
     * Get the previous channel in use
     * @param currentChannel Current channel number
     * @return Previous channel number in use, or last channel in use if at beginning
     */
    uint16_t getPreviousChannel(uint16_t currentChannel) {
        if (currentChannel < 1 || currentChannel > MAX_CHANNELS) {
            currentChannel = MAX_CHANNELS;
        }

        // Start searching from the previous channel
        uint16_t searchChannel = static_cast<uint16_t>(currentChannel - 1);
        
        // Search backward from current position
        for (uint16_t i = 0; i < MAX_CHANNELS; i++) {
            if (searchChannel < 1) {
                searchChannel = MAX_CHANNELS; // Wrap around to end
            }
            
            if (isChannelInUse(searchChannel)) {
                return searchChannel;
            }
            
            searchChannel = static_cast<uint16_t>(searchChannel - 1);
        }
        
        // If no channels are in use, return channel 1
        return 1;
    }

    /**
     * Get the first channel in use
     * @return First channel number in use, or 1 if no channels are in use
     */
    uint16_t getFirstChannel() {
        for (uint16_t i = 1; i <= MAX_CHANNELS; i++) {
            if (isChannelInUse(i)) {
                return i;
            }
        }
        return 1; // Default to channel 1 if none are in use
    }

    /**
     * Get the last channel in use
     * @return Last channel number in use, or MAX_CHANNELS if no channels are in use
     */
    uint16_t getLastChannel() {
        for (uint16_t i = MAX_CHANNELS; i >= 1; i--) {
            if (isChannelInUse(i)) {
                return i;
            }
        }
        return MAX_CHANNELS; // Default to last channel if none are in use
    }

    /**
     * Clear/erase a channel (set name to empty)
     * @param channelNumber Channel number to clear
     * @return true if successful, false if channel number is invalid
     */
    bool clearChannel(uint16_t channelNumber) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS) {
            return false;
        }
        
        VFO emptyChannel = {};
        memset(&emptyChannel, 0, sizeof(VFO));
        return writeChannel(channelNumber, emptyChannel);
    }

    /**
     * Get total number of channels in use
     * @return Number of channels that have non-empty names
     */
    uint16_t getChannelsInUseCount() {
        uint16_t count = 0;
        for (uint16_t i = 1; i <= MAX_CHANNELS; i++) {
            if (isChannelInUse(i)) {
                count++;
            }
        }
        return count;
    }

    /**
     * Copy current VFO settings to a channel
     * @param channelNumber Channel number to save to
     * @param vfoIndex VFO index (0 for VFOA, 1 for VFOB)
     * @return true if successful, false if invalid parameters
     */
    bool saveVFOToChannel(uint16_t channelNumber, uint8_t vfoIndex) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS || vfoIndex > 1) {
            return false;
        }
        
        VFO channelData = radioSettings.vfo[vfoIndex];
        channelData.channel = channelNumber;
        
        return writeChannel(channelNumber, channelData);
    }

    /**
     * Load channel settings to current VFO
     * @param channelNumber Channel number to load from
     * @param vfoIndex VFO index (0 for VFOA, 1 for VFOB)
     * @return true if successful, false if invalid parameters or channel not in use
     */
    bool loadChannelToVFO(uint16_t channelNumber, uint8_t vfoIndex) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS || vfoIndex > 1) {
            return false;
        }
        
        if (!isChannelInUse(channelNumber)) {
            return false;
        }
        
        VFO channelData;
        if (readChannel(channelNumber, channelData)) {
            radioSettings.vfo[vfoIndex] = channelData;
            radioSettings.memory[vfoIndex] = channelNumber;
            radioSettings.showVFO[vfoIndex] = ONOFF::OFF; // Show memory, not VFO
            return true;
        }

        return false;
    }

    /**
     * Retrieve channel information without modifying the active VFO
     * @param channelNumber Channel number (1-230)
     * @param channelData Destination VFO struct
     * @return true if the channel exists and data was copied
     */
    bool getChannelData(uint16_t channelNumber, VFO& channelData) {
        if (channelNumber < 1 || channelNumber > MAX_CHANNELS) {
            return false;
        }

        if (!isChannelInUse(channelNumber)) {
            return false;
        }

        if (!readChannel(channelNumber, channelData)) {
            return false;
        }

        channelData.channel = channelNumber;
        return true;
    }

private:

    static constexpr uint16_t settingsVersion = 0x015A;
    System::SystemTask& systask;

    EEPROM eeprom;

    uint16_t initBlock = 0x0000;
    static constexpr uint16_t maxBlock = 0x000F;

    static constexpr uint8_t saveDelaySeconds = 5;
    static constexpr uint8_t saveDelayTicks = saveDelaySeconds * 2; // half-second ticks

    bool radioSavePending = false;
    uint8_t radioSaveDelay = 0;

    SETTINGS lastSavedRadioSettings{};

    bool memorySavePending = false;        // Placeholder for channel memory save
    uint8_t memorySaveDelay = 0;           // Placeholder counter
    uint16_t pendingMemoryChannel = 0;     // Channel number to save
    uint8_t pendingMemoryVFO = 0;          // VFO index to save (0 or 1)

    static constexpr uint16_t CHANNEL_START_ADDRESS = 0x0050;
    static constexpr uint16_t CHANNEL_SIZE = sizeof(PackedVFOData); // 32 bytes

};
