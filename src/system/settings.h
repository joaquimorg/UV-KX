#pragma once

#include <cstdint>
#include "bk4819.h"
#include "sys.h"
#include "eeprom.h"


/*

    EEPROM Layout:

    [0x0000] - Settings
    [0x0050] - MEMORY - (230 memory's 32 bytes each)
    [0x1D10] - EMPTY
    [0x1E00] - CALIBRATION DATA

    [0x1FFF] - END OF EEPROM

*/

namespace System {
    class SystemTask;
}

class Settings {
public:

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

    struct FREQ {
        uint32_t frequency : 27;
        CodeType codeType : 4;
        uint8_t  code;
    } __attribute__((packed)); // 5 Bytes

    struct VFO {
        FREQ     rx;                       // RX Frequency
        FREQ     tx;                       // TX Frequency
        char     name[10];                 // Memory Name
        uint16_t channel;                  // Channel Number
        uint8_t  squelch : 4;       // Squelch Level
        Step     step : 4;       // Step Frequency
        ModType  modulation : 4;       // Modulation Type
        BK4819_Filter_Bandwidth bw : 4;       // Filter Bandwidth
        TXOutputPower           power : 2;       // TX Power Level
        OffsetDirection         shift : 2;       // Offset Direction
        ONOFF    repeaterSte : 1;       // Repeater STE
        ONOFF    ste : 1;       // STE
        TXRX     compander : 2;       // Compander
        uint8_t  roger : 4;       // Roger Beep
        uint8_t  pttid : 4;       // PTT ID
        uint8_t  rxagc : 6;       // RX AGC Level
        uint8_t  reserved1[5];             // Reserved
    } __attribute__((packed)); // 32 Bytes    

    static_assert(sizeof(VFO) == 32, "VFO struct size mismatch");


    // MIC DB\nBATT SAVE\nBUSY LOCKOUT\nBCKLIGHT LEVEL\nBCKLIGHT TIME\nBCKLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP
    struct SETTINGS {
        uint16_t version;                 // Settings Version
        uint8_t  batteryType : 2;         // Battery Type
        uint8_t  busyLockout : 1;         // Busy Lockout
        uint8_t  beep : 1;                // Beep
        uint8_t  backlightLevel : 4;      // Backlight Level
        uint8_t  backlightTime : 4;       // Backlight Time                
        uint8_t  micDB : 4;               // Mic DB
        uint8_t  lcdContrast : 4;         // LCD Contrast
        uint8_t  txTOT : 4;               // TX TOT        
        uint8_t  batterySave : 4;         // Battery Save
        uint8_t  backlightMode : 2;       // Backlight Mode
        uint8_t  vfoSelected : 2;         // VFO Selected - 0 = VFOA, 1 = VFOB, 2 = CH1, 3 = CH2        
        uint16_t channel[2];              // Channel Number        
        VFO      vfo[2];                  // VFO Settings        
        uint8_t  reserved1[6];            // Reserved
    } __attribute__((packed));

    static_assert(sizeof(SETTINGS) == 80, "SETTINGS struct size mismatch");

    SETTINGS radioSettings;

    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}
    void factoryReset() {};

    void getRadioSettings() {
        eeprom.readBuffer(0x0000, &radioSettings, sizeof(SETTINGS));
       
    }

    void setRadioSettings() {
        eeprom.writeBuffer(0x0000, &radioSettings, sizeof(SETTINGS));
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
        // TODO: Implement EEPROM initialization
        if (initBlock < maxBlock) {
            initBlock++;
        }
        return (uint8_t)((initBlock * 100) / maxBlock);
    }

    EEPROM& getEEPROM() {
        return eeprom;
    }

private:

    static constexpr uint16_t settingsVersion = 0x015A;
    System::SystemTask& systask;

    EEPROM eeprom;

    uint16_t initBlock = 0x0000;
    static constexpr uint16_t maxBlock = 0x000F;

};
