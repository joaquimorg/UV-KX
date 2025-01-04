#pragma once

#include <cstdint>
#include "sys.h"
#include "eeprom.h"


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

namespace System {
    class SystemTask;
}
class Settings {
public:
    Settings(System::SystemTask& systask) : systask{ systask }, eeprom() {}
    void factoryReset() {};


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

    System::SystemTask& systask;
    EEPROM eeprom;

};
