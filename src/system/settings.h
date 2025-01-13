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
