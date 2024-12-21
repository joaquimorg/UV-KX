#include "keyboard.h"
#include "gpio.h"
#include "gpio_hal.h"
//#include "i2c.h"
#include "sys.h"
#include "system.h"

const Keyboard::KeyboardRow Keyboard::KEYBOARD_LAYOUT[ROWS] = {
    // First row
    {
        .setToZeroMask = ~(1u << GPIOA_PIN_KEYBOARD_4) & 0xffff,
        .pins = {
            { KeyCode::KEY_MENU,  GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_1,     GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_4,     GPIOA_PIN_KEYBOARD_2 },
            { KeyCode::KEY_7,     GPIOA_PIN_KEYBOARD_3 }
        }
    },
    // Second row
    {
        .setToZeroMask = ~(1u << GPIOA_PIN_KEYBOARD_5) & 0xffff,
        .pins = {
            { KeyCode::KEY_UP,    GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_2,     GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_5,     GPIOA_PIN_KEYBOARD_2 },
            { KeyCode::KEY_8,     GPIOA_PIN_KEYBOARD_3 }
        }
    },
    // Third row
    {
        .setToZeroMask = ~(1u << GPIOA_PIN_KEYBOARD_6) & 0xffff,
        .pins = {
            { KeyCode::KEY_DOWN,  GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_3,     GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_6,     GPIOA_PIN_KEYBOARD_2 },
            { KeyCode::KEY_9,     GPIOA_PIN_KEYBOARD_3 }
        }
    },
    // Fourth row
    {
        .setToZeroMask = ~(1u << GPIOA_PIN_KEYBOARD_7) & 0xffff,
        .pins = {
            { KeyCode::KEY_EXIT,  GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_STAR,  GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_0,     GPIOA_PIN_KEYBOARD_2 },
            { KeyCode::KEY_F,     GPIOA_PIN_KEYBOARD_3 }
        }
    },
    // FN Row
    {
        .setToZeroMask = 0xffff,
        .pins = {
            { KeyCode::KEY_SIDE1,   GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_SIDE2,   GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_INVALID, GPIOA_PIN_KEYBOARD_2 },
            { KeyCode::KEY_PTT,     GPIOA_PIN_KEYBOARD_3 }
        }
    }
};

Keyboard::Keyboard(System::SystemTask& systask) 
    : systask{ systask }
    , mKeyPtt(false)
    , mPrevStatePtt(KeyState::KEY_RELEASED)
    , mWasFKeyPressed(false)
{
    // Initialize arrays
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            mKeyState[i][j] = false;
            mPrevKeyState[i][j] = KeyState::KEY_RELEASED;
            mLongPressTimer[i][j] = 0;
        }
    }
}

void Keyboard::init() {
    xTaskCreateStatic(
        keyTaskWrapper,
        "KEY",
        configMINIMAL_STACK_SIZE,
        this,
        1 + tskIDLE_PRIORITY,
        mKeyTaskStack,
        &mKeyTaskBuffer
    );
}

void Keyboard::keyTaskWrapper(void* parameter) {
    auto* keyboard = static_cast<Keyboard*>(parameter);
    keyboard->keyTask();
}

void Keyboard::keyTask() {
    for (;;) {
        readKeyboard();
        processKeys();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void Keyboard::readKeyboard() {
    uint32_t regH, regL;

    // Handle PTT key
    mKeyPtt = !GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT);
    if (mPrevStatePtt == KeyState::KEY_PRESSED && !mKeyPtt) {
        pushKeyMessage(KeyCode::KEY_PTT, KeyState::KEY_RELEASED);
        mPrevStatePtt = KeyState::KEY_RELEASED;
    } else if (mPrevStatePtt == KeyState::KEY_RELEASED && mKeyPtt) {
        pushKeyMessage(KeyCode::KEY_PTT, KeyState::KEY_PRESSED);
        mPrevStatePtt = KeyState::KEY_PRESSED;
    } else if (mPrevStatePtt == KeyState::KEY_PRESSED && mKeyPtt) {
        return;
    }

    // Read matrix keyboard
    // Set all rows high
    GPIOA->DATA |= (1u << GPIOA_PIN_KEYBOARD_4) |
                   (1u << GPIOA_PIN_KEYBOARD_5) |
                   (1u << GPIOA_PIN_KEYBOARD_6) |
                   (1u << GPIOA_PIN_KEYBOARD_7);

    // Read FN row
    GPIOA->DATA &= KEYBOARD_LAYOUT[ROWS - 1].setToZeroMask;
    
    // Debounce delay
    for (uint8_t i = 0; i < 8; i++) {
        regL = GPIOA->DATA;
        delayMs(1);
    }
    regL = GPIOA->DATA;

    // Read FN keys
    for (uint8_t j = 0; j < 2; j++) {
        const uint16_t mask = 1u << KEYBOARD_LAYOUT[ROWS - 1].pins[j].pin;
        mKeyState[ROWS - 1][j] = !(regL & mask);
    }

    // Scan main matrix
    for (uint8_t i = 0; i < (ROWS - 1); i++) {
        // Reset rows
        GPIOA->DATA |= (1u << GPIOA_PIN_KEYBOARD_4) |
                       (1u << GPIOA_PIN_KEYBOARD_5) |
                       (1u << GPIOA_PIN_KEYBOARD_6) |
                       (1u << GPIOA_PIN_KEYBOARD_7);

        regH = GPIOA->DATA;
        GPIOA->DATA &= KEYBOARD_LAYOUT[i].setToZeroMask;

        // Debounce delay
        for (uint8_t k = 0; k < 8; k++) {
            regL = GPIOA->DATA;
            delayMs(1);
        }
        regL = GPIOA->DATA;

        for (uint8_t j = 0; j < COLS; j++) {
            const uint16_t mask = 1u << KEYBOARD_LAYOUT[i].pins[j].pin;
            if (regH & mask) {
                mKeyState[i][j] = !(regL & mask);
            }
        }
    }

    resetGPIO();
}

void Keyboard::processKeys() {
    TickType_t currentTick = xTaskGetTickCount();
    
    for (uint8_t i = 0; i < ROWS; i++) {
        for (uint8_t j = 0; j < COLS; j++) {
            if (mKeyState[i][j]) {
                if (mPrevKeyState[i][j] == KeyState::KEY_RELEASED) {
                    if (mWasFKeyPressed) {
                        pushKeyMessage(KEYBOARD_LAYOUT[i].pins[j].key, KeyState::KEY_PRESSED_WITH_F);
                        mPrevKeyState[i][j] = KeyState::KEY_PRESSED_WITH_F;
                        mWasFKeyPressed = false;
                    } else {
                        pushKeyMessage(KEYBOARD_LAYOUT[i].pins[j].key, KeyState::KEY_PRESSED);
                        mPrevKeyState[i][j] = KeyState::KEY_PRESSED;
                    }

                    if (KEYBOARD_LAYOUT[i].pins[j].key == KeyCode::KEY_F) {
                        mWasFKeyPressed = true;
                    }

                    mLongPressTimer[i][j] = currentTick;
                } else if (mPrevKeyState[i][j] == KeyState::KEY_PRESSED) {
                    TickType_t elapsedTime = currentTick - mLongPressTimer[i][j];
                    if (elapsedTime >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
                        mLongPressTimer[i][j] = 0;
                        mPrevKeyState[i][j] = KeyState::KEY_LONG_PRESSED;
                    }
                } else if (mPrevKeyState[i][j] == KeyState::KEY_LONG_PRESSED || 
                         mPrevKeyState[i][j] == KeyState::KEY_LONG_PRESSED_CONT) {
                    pushKeyMessage(KEYBOARD_LAYOUT[i].pins[j].key, mPrevKeyState[i][j]);
                    mPrevKeyState[i][j] = KeyState::KEY_LONG_PRESSED_CONT;
                }
            } else {
                if (mPrevKeyState[i][j] != KeyState::KEY_RELEASED) {
                    if (mPrevKeyState[i][j] != KeyState::KEY_PRESSED_WITH_F && 
                        mPrevKeyState[i][j] != KeyState::KEY_LONG_PRESSED && 
                        mPrevKeyState[i][j] != KeyState::KEY_LONG_PRESSED_CONT) {
                        pushKeyMessage(KEYBOARD_LAYOUT[i].pins[j].key, KeyState::KEY_RELEASED);
                    }

                    if (mPrevKeyState[i][j] == KeyState::KEY_LONG_PRESSED_CONT &&
                        (KEYBOARD_LAYOUT[i].pins[j].key == KeyCode::KEY_UP || 
                         KEYBOARD_LAYOUT[i].pins[j].key == KeyCode::KEY_DOWN)) {
                        pushKeyMessage(KEYBOARD_LAYOUT[i].pins[j].key, KeyState::KEY_RELEASED);
                    }

                    mLongPressTimer[i][j] = 0;
                    mPrevKeyState[i][j] = KeyState::KEY_RELEASED;
                }
            }
        }
    }
}

void Keyboard::pushKeyMessage(KeyCode key, KeyState state) {
    systask.pushMessageKey(key, state);
}

void Keyboard::resetGPIO() {
    // Create I2C stop condition
    //I2C_Stop();

    // Reset VOICE pins
    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_6);
    GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_KEYBOARD_7);
}