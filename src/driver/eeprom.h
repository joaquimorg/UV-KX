#pragma once

#include <cstdint>
#include "i2c_hal.h"
#include "FreeRTOS.h"
#include "task.h"

class EEPROM {
public:
    EEPROM() {};

    // Constants
    static constexpr uint8_t PAGE_SIZE = 32;
    static constexpr uint8_t BASE_ADDRESS = 0xA0;
    static constexpr uint32_t PROTECTED_ADDR = 0x1E00;
    static constexpr uint32_t PROTECTED_SIZE = 0x200;

    // Core EEPROM operations
    void readBuffer(uint32_t address, void* buffer, uint16_t size) {
        if (!buffer || size == 0) {
            return;
        }

        taskENTER_CRITICAL();

        uint8_t deviceAddr = getDeviceAddress(address);

        i2c.start();
        i2c.write(deviceAddr);
        i2c.write(static_cast<uint8_t>((address >> 8) & 0xFF));
        i2c.write(static_cast<uint8_t>(address & 0xFF));

        i2c.start();
        i2c.write(deviceAddr | 0x01);  // Set read bit
        i2c.readBuffer(static_cast<uint8_t*>(buffer), size);
        i2c.stop();

        taskEXIT_CRITICAL();
    }

    void writeBuffer(uint32_t address, const void* buffer, uint16_t size) {
        if (!buffer || size == 0) {
            return;
        }

        // Calculate write end address
        uint32_t endAddr = address + size - 1;

        if ((address >= PROTECTED_ADDR && address < (PROTECTED_ADDR + PROTECTED_SIZE)) ||
            (endAddr >= PROTECTED_ADDR && endAddr < (PROTECTED_ADDR + PROTECTED_SIZE)) ||
            (address < PROTECTED_ADDR && endAddr >= (PROTECTED_ADDR + PROTECTED_SIZE))) {
            return;  // Protected area
        }

        const uint8_t* data = static_cast<const uint8_t*>(buffer);

        taskENTER_CRITICAL();

        while (size > 0) {
            // Calculate page boundaries
            uint16_t offset = address % PAGE_SIZE;
            uint16_t remainingInPage = PAGE_SIZE - offset;
            uint16_t writeSize = (size < remainingInPage) ? size : remainingInPage;

            // Read current content
            readBuffer(address, tmpBuffer, writeSize);

            // Only write if data is different
            if (memcmp(data, tmpBuffer, writeSize) != 0) {
                uint8_t deviceAddr = getDeviceAddress(address);

                i2c.start();
                i2c.write(deviceAddr);
                i2c.write(static_cast<uint8_t>((address >> 8) & 0xFF));
                i2c.write(static_cast<uint8_t>(address & 0xFF));

                i2c.writeBuffer(data, writeSize);
                i2c.stop();

                // Wait for write to complete
                waitForWrite();
            }

            // Update pointers and remaining size
            data += writeSize;
            address += writeSize;
            size -= writeSize;
        }

        taskEXIT_CRITICAL();
    }

private:

    // Internal helper methods
    uint8_t getDeviceAddress(uint32_t address) const {
        //return static_cast<uint8_t>(BASE_ADDRESS | ((address / 0x10000) << 1));
        return static_cast<uint8_t>(BASE_ADDRESS);
    }

    void waitForWrite() const {
        delayMs(2);  // Wait for EEPROM write to complete
    }

    // Reference to I2C instance
    I2C i2c;

    // Temporary buffer for write operations
    static constexpr size_t TMP_BUFFER_SIZE = 128;
    uint8_t tmpBuffer[TMP_BUFFER_SIZE];
};