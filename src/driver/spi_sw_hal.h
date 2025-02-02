#pragma once

#include <cstdint>
#include "sys.h"
#include "gpio.h"
#include "portcon.h"
#include "gpio_hal.h"

class SPISoftwareInterface {
public:

    SPISoftwareInterface() {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
    }

    // Method to write to a register
    void writeRegister(uint8_t reg, uint16_t value) {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
        delay250ns(1);
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        spiWriteU8(reg);
        spiWriteU16(value);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        delay250ns(1);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
    }

    // Method to read from a register
    uint16_t readRegister(uint8_t reg) {
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
        delay250ns(1);
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        spiWriteU8(reg | 0x80);
        uint16_t value = spiReadU16();
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCN);
        delay250ns(1);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
        GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
        return value;
    }

private:
    uint16_t spiReadU16(void) {
        uint16_t value = 0;

        // Enable input on PORTC pin C2
        PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) | PORTCON_PORTC_IE_C2_BITS_ENABLE;
        GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_INPUT;

        delay250ns(1);

        // Read 16 bits from the SPI
        for (int i = 0; i < 16; i++) {
            value = (value << 1) | (GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA) ? 1 : 0);

            // Generate clock pulse
            GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);
            GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);
        }

        // Restore default configuration
        PORTCON_PORTC_IE = (PORTCON_PORTC_IE & ~PORTCON_PORTC_IE_C2_MASK) | PORTCON_PORTC_IE_C2_BITS_DISABLE;
        GPIOC->DIR = (GPIOC->DIR & ~GPIO_DIR_2_MASK) | GPIO_DIR_2_BITS_OUTPUT;

        return value;
    }

    void spiWriteU8(uint8_t data) {
        // Ensure clock is initially low
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);

        for (int i = 0; i < 8; i++) {
            // Set SDA based on the MSB of the data
            if (data & 0x80) {
                GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
            }
            else {
                GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
            }

            // Create the clock pulse
            delay250ns(1);
            GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);

            // Shift data to the left to prepare for the next bit
            data <<= 1;

            // Lower the clock signal
            GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);
        }
    }


    void spiWriteU16(uint16_t data) {
        // Ensure clock is initially low
        GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);

        for (int i = 0; i < 16; i++) {
            // Set SDA based on the MSB of the data
            if (data & 0x8000) {
                GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
            }
            else {
                GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SDA);
            }

            // Generate clock pulse
            delay250ns(1);
            GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);

            // Shift data to prepare for the next bit
            data <<= 1;

            // Lower the clock signal
            GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_BK4819_SCL);
            delay250ns(1);
        }
    }
};

