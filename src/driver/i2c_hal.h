#pragma once

#include <cstdint>

#include "sys.h"
#include "gpio.h"
#include "portcon.h"
#include "gpio_hal.h"

class I2C {
public:
    // Constants
    static constexpr uint8_t WRITE = 0U;
    static constexpr uint8_t READ = 1U;
    
    I2C() {};

    // Core I2C operations
    void start() {
        setSDA(true);
        delayMicroseconds(1);
        setSCL(true);
        delayMicroseconds(1);
        setSDA(false);
        delayMicroseconds(1);
        setSCL(false);
        delayMicroseconds(1);
    }
    void stop() {
        setSDA(false);
        delayMicroseconds(1);
        setSCL(false);
        delayMicroseconds(1);
        setSCL(true);
        delayMicroseconds(1);
        setSDA(true);
        delayMicroseconds(1);
    }

    // Reading operations
    uint8_t read(bool isFinal) {
        uint8_t data = 0;

        configureSDAPinInput();

        for (int i = 0; i < 8; i++) {
            setSCL(false);
            delayMicroseconds(1);
            setSCL(true);
            delayMicroseconds(1);
            data <<= 1;
            delayMicroseconds(1);
            if (readSDA()) {
                data |= 1U;
            }
            setSCL(false);
            delayMicroseconds(1);
        }

        configureSDAPinOutput();
        setSCL(false);
        delayMicroseconds(1);
        setSDA(isFinal);
        delayMicroseconds(1);
        setSCL(true);
        delayMicroseconds(1);
        setSCL(false);
        delayMicroseconds(1);

        return data;
    }

    uint16_t readBuffer(uint8_t* buffer, uint16_t size) {
        if (!buffer || size == 0) {
            return 0;
        }

        for (uint16_t i = 0; i < size - 1; i++) {
            delayMicroseconds(1);
            buffer[i] = read(false);
        }

        delayMicroseconds(1);
        buffer[size - 1] = read(true);

        return size;
    }

    // Writing operations
    int16_t write(uint8_t data) {
        int16_t ret = -1;

        setSCL(false);
        delayMicroseconds(1);

        for (int i = 0; i < 8; i++) {
            setSDA((data & 0x80) != 0);
            data <<= 1;
            delayMicroseconds(1);
            setSCL(true);
            delayMicroseconds(1);
            setSCL(false);
            delayMicroseconds(1);
        }

        configureSDAPinInput();
        setSDA(true);
        delayMicroseconds(1);
        setSCL(true);
        delayMicroseconds(1);

        for (int i = 0; i < 255; i++) {
            if (!readSDA()) {
                ret = 0;
                break;
            }
        }

        setSCL(false);
        delayMicroseconds(1);
        configureSDAPinOutput();
        setSDA(true);

        return ret;
    }

    int16_t writeBuffer(const uint8_t* buffer, uint16_t size) {
        if (!buffer || size == 0) {
            return -1;
        }

        for (uint16_t i = 0; i < size; i++) {
            if (write(buffer[i]) < 0) {
                return -1;
            }
        }
        return 0;
    }


private:    

    void configureSDAPinInput() {
        PORTCON_PORTA_IE |= PORTCON_PORTA_IE_A11_BITS_ENABLE;
        PORTCON_PORTA_OD &= ~PORTCON_PORTA_OD_A11_MASK;
        GPIOA->DIR &= ~GPIO_DIR_11_MASK;
    }

    void configureSDAPinOutput() {
        PORTCON_PORTA_IE &= ~PORTCON_PORTA_IE_A11_MASK;
        PORTCON_PORTA_OD |= PORTCON_PORTA_OD_A11_BITS_ENABLE;
        GPIOA->DIR |= GPIO_DIR_11_BITS_OUTPUT;
    }

    void setSCL(bool state) {
        if (state) {
            GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
        }
        else {
            GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
        }
    }

    void setSDA(bool state) {
        if (state) {
            GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
        }
        else {
            GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
        }
    }

    bool readSDA() {
        return GPIO_CheckBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
    }

    void delayMicroseconds(uint32_t us) {
        delayUs(us);
    }
};