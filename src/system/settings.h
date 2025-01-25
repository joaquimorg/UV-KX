#pragma once

#include <cstdint>
#include "sys.h"
#include "eeprom.h"
#include "radio.h"

/*

    SQUELCH
    STEP
    MODE
    BANDWIDTH
    TX POWER
    SHIFT
    OFFSET
    RX CTCS
    TX CTCS
    RX DTCS
    TX DTCS
    TX STE
    RX STE
    COMPANDER
    PTT ID
    AFC
    RX ACG

    MIC DB
    BATTERY SAVE
    BUSY LOCKOUT
    BACKLIGHT LEVEL
    BACKLIGHT TIME
    BACKLIGHT MODE
    LCD CONTRAST
    TX TOT
    BEEP

*/

/*

    Memory name encoding:
    " " -> 0x0
    "A" -> 0x1
    "B" -> 0x2
    "C" -> 0x3
    ...
    "Z" -> 0x1A
    "0" -> 0x1B
    "1" -> 0x1C
    "2" -> 0x1D
    "3" -> 0x1E
    "4" -> 0x1F
    "5" -> 0x20
    "6" -> 0x21
    "7" -> 0x22
    "8" -> 0x23
    "9" -> 0x24
    "-" -> 0x25
    "." -> 0x26
    "#" -> 0x27
    "/" -> 0x28
    ":" -> 0x29


*/

/*

    EEPROM Layout:

    [0x0000] - Settings Version
    [0x0002] - Settings VFO A
    [0x0004] - Settings VFO B


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
        RX = 1,
        TX = 2,
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
        uint8_t code;
    } __attribute__((packed)); // 5 Bytes

    struct VFO {
        FREQ rx;
        FREQ tx;
        char name[11];
        int16_t channel;
        uint8_t squelch : 4;
        Step step : 4;
        ModType modulation : 4;
        BK4819_Filter_Bandwidth bw : 4;
        TXOutputPower power : 2;
        OffsetDirection shift : 2;
        ONOFF repeaterSte : 1;
        ONOFF ste : 1;
        TXRX compander : 2;
        uint8_t pttid : 4;
        uint8_t afc : 4;
        uint8_t rxagc : 4;
    } __attribute__((packed));

    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}
    void factoryReset() {};

    uint16_t getSettingsVersion() {
        uint16_t version = 0;
        eeprom.readBuffer(0x0000, &version, sizeof(version));
        return version;
    }

    void setSettingsVersion(uint16_t version) {
        eeprom.writeBuffer(0x0000, &version, sizeof(version));
    }

    bool validateSettingsVersion() {
        uint16_t version = getSettingsVersion();
        if (version == settingsVersion) {
            return true;
        }
        return false;
    }

    // Memory Name compression
    //

    // Custom function to calculate string length
    int stringLength(const char* str) {
        int length = 0;
        while (str[length] != '\0') {
            ++length;
        }
        return length;
    }

    // Conversion function to encode a character
    uint8_t encodeChar(char c) {
        if (c == ' ') return 0x0;
        if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
        if (c >= '0' && c <= '9') return c - '0' + 0x1B;
        if (c == '-') return 0x25;
        if (c == '.') return 0x26;
        if (c == '#') return 0x27;
        if (c == '/') return 0x28;
        if (c == ':') return 0x29;
        return 0x0; // Default to space for unsupported characters
    }

    // Custom function to convert a character to uppercase
    char toUpperCase(char c) {
        if (c >= 'a' && c <= 'z') {
            return c - ('a' - 'A');
        }
        return c;
    }

    // Compress a string into a 7-byte array
    void compress(const char* input, uint8_t* output) {
        for (int i = 0; i < 7; ++i) {
            output[i] = 0; // Initialize output to zero
        }
        uint64_t buffer = 0;

        // Convert input to uppercase and pad with spaces if needed
        char temp[10];
        int len = stringLength(input);
        for (int i = 0; i < 10; ++i) {
            if (i < len) {
                temp[i] = toUpperCase(input[i]); // Convert to uppercase
            }
            else {
                temp[i] = ' '; // Pad with spaces
            }
        }

        // Encode each character into 5 bits and store in a 64-bit buffer
        for (int i = 0; i < 10; ++i) {
            buffer <<= 5;
            buffer |= encodeChar(temp[i]);
        }

        // Copy the 50 bits into the output array (7 bytes)
        for (int i = 6; i >= 0; --i) {
            output[i] = buffer & 0xFF;
            buffer >>= 8;
        }
    }

    // Decompress a 7-byte array into a string
    void decompress(const uint8_t* input, char* output) {
        uint64_t buffer = 0;

        // Reconstruct the 50 bits from the input array
        for (int i = 0; i < 7; ++i) {
            buffer <<= 8;
            buffer |= input[i];
        }

        // Decode each 5-bit chunk back to a character
        for (int i = 9; i >= 0; --i) {
            output[i] = decodeTable[buffer & 0x1F];
            buffer >>= 5;
        }
        output[10] = '\0'; // Null-terminate the string
    }

private:

    static constexpr char decodeTable[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.#/:";
    static constexpr uint16_t settingsVersion = 0x015A;
    System::SystemTask& systask;

    EEPROM eeprom;

};
