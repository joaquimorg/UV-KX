#pragma once

#include "sys.h"
#include "gpio.h"
#include "gpio_hal.h"

// Define the rows and columns based on the schematic
const uint8_t rows[] = { GPIOA_PIN_KEYBOARD_4, GPIOA_PIN_KEYBOARD_5, GPIOA_PIN_KEYBOARD_6, GPIOA_PIN_KEYBOARD_7 }; // Replace with GPIO pins for rows
const uint8_t cols[] = { GPIOA_PIN_KEYBOARD_0, GPIOA_PIN_KEYBOARD_1, GPIOA_PIN_KEYBOARD_2, GPIOA_PIN_KEYBOARD_3 }; // Replace with GPIO pins for columns

// Key mapping (row-major order)
const char keyMap[4][4] = {
    {'M', '1', '4', '7'},
    {'U', '2', '5', '8'},
    {'D', '3', '6', '9'},
    {'E', '*', '0', 'F'} };

// Timing thresholds
const uint32_t DEBOUNCE_DELAY = 50;    // 50ms debounce time
const uint32_t LONG_PRESS_TIME = 1000;  // 1s for long press
const uint8_t SAMPLES_FOR_VALID = 3;    // Number of consistent samples needed for valid key press

class Keypad {
private:
    enum KeyState {
        IDLE,
        DEBOUNCING_PRESS,
        PRESSED,
        LONG_PRESSED,
        DEBOUNCING_RELEASE,
        RELEASED
    };

    struct KeyStatus {
        int row = -1;
        int col = -1;
        uint8_t consecutiveSamples = 0;
        uint32_t lastChangeTime = 0;
        uint32_t pressStartTime = 0;
        uint32_t releaseTime = 0;
        KeyState state = IDLE;
    };

    KeyStatus currentKey;

    void setupPins() {
        for (uint8_t row : rows) {
            GPIO_SetBit(&GPIOA->DATA, row);
        }
    }

    int scanKeys() {
        for (uint8_t row = 0; row < 4; row++) {
            GPIO_ClearBit(&GPIOA->DATA, rows[row]); // Activate the row
            // Add small delay for electrical settling
            __asm__("nop");
            __asm__("nop");

            for (uint8_t col = 0; col < 4; col++) {
                // Read column multiple times to ensure stability
                bool keyPressed = true;
                for (int i = 0; i < 4; i++) {
                    if (GPIO_CheckBit(&GPIOA->DATA, cols[col])) {
                        keyPressed = false;
                        break;
                    }
                }

                if (keyPressed) {
                    GPIO_SetBit(&GPIOA->DATA, rows[row]); // Reset row
                    return row * 4 + col;
                }
            }
            GPIO_SetBit(&GPIOA->DATA, rows[row]); // Reset row
        }
        return -1; // No key pressed
    }

public:
    Keypad() {
        setupPins();
    }

    void update() {
        int keyIndex = scanKeys();
        uint32_t currentTime = getElapsedMilliseconds();

        // Handle key press detection with debouncing
        if (keyIndex != -1) {
            int newRow = keyIndex / 4;
            int newCol = keyIndex % 4;

            switch (currentKey.state) {
            case IDLE:
                currentKey.row = newRow;
                currentKey.col = newCol;
                currentKey.lastChangeTime = currentTime;
                currentKey.consecutiveSamples = 1;
                currentKey.state = DEBOUNCING_PRESS;
                break;

            case DEBOUNCING_PRESS:
                if (newRow == currentKey.row && newCol == currentKey.col) {
                    currentKey.consecutiveSamples++;
                    if (currentKey.consecutiveSamples >= SAMPLES_FOR_VALID &&
                        (currentTime - currentKey.lastChangeTime) >= DEBOUNCE_DELAY) {
                        currentKey.state = PRESSED;
                        currentKey.pressStartTime = currentTime;
                    }
                }
                else {
                    currentKey.consecutiveSamples = 1;
                    currentKey.lastChangeTime = currentTime;
                }
                break;

            case PRESSED:
                if (currentTime - currentKey.pressStartTime >= LONG_PRESS_TIME) {
                    currentKey.state = LONG_PRESSED;
                }
                break;

            case LONG_PRESSED:
                // Maintain state while key is held
                break;

            case DEBOUNCING_RELEASE:
                // Unexpected press during release debounce
                currentKey.state = PRESSED;
                break;

            case RELEASED:
                // New press after release
                currentKey.state = DEBOUNCING_PRESS;
                currentKey.row = newRow;
                currentKey.col = newCol;
                currentKey.consecutiveSamples = 1;
                currentKey.lastChangeTime = currentTime;
                break;
            }
        }
        else {
            // Handle key release detection
            switch (currentKey.state) {
            case IDLE:
                // No key pressed, maintain idle state
                break;

            case DEBOUNCING_PRESS:
                // Key released during press debounce, return to idle
                currentKey.state = IDLE;
                currentKey.consecutiveSamples = 0;
                break;
                
            case PRESSED:
            case LONG_PRESSED:
                currentKey.state = DEBOUNCING_RELEASE;
                currentKey.lastChangeTime = currentTime;
                currentKey.consecutiveSamples = 1;
                break;

            case DEBOUNCING_RELEASE:
                currentKey.consecutiveSamples++;
                if (currentKey.consecutiveSamples >= SAMPLES_FOR_VALID &&
                    (currentTime - currentKey.lastChangeTime) >= DEBOUNCE_DELAY) {
                    currentKey.state = RELEASED;
                    currentKey.releaseTime = currentTime;
                }
                break;

            case RELEASED:
                currentKey.state = IDLE;
                currentKey.row = -1;
                currentKey.col = -1;
                break;
            }
        }

        // Reset VOICE pins
        //GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
        //GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);
    }

    char getKey() {
        if (currentKey.row != -1 && currentKey.col != -1) {
            return keyMap[currentKey.row][currentKey.col];
        }
        return 0;
    }

    bool isPressed() {
        return currentKey.state == PRESSED || currentKey.state == LONG_PRESSED;
    }

    bool isReleased() {
        return currentKey.state == RELEASED;
    }

    bool isLongPressed() {
        return currentKey.state == LONG_PRESSED;
    }

    uint32_t getPressedDuration() {
        if (currentKey.state == PRESSED || currentKey.state == LONG_PRESSED) {
            return getElapsedMilliseconds() - currentKey.pressStartTime;
        }
        return 0;
    }

    uint32_t getReleasedDuration() {
        if (currentKey.state == RELEASED) {
            return getElapsedMilliseconds() - currentKey.releaseTime;
        }
        return 0;
    }
};
