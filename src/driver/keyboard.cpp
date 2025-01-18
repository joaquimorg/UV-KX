#include "keyboard.h"
#include "gpio.h"
#include "gpio_hal.h"
//#include "i2c.h"
#include "sys.h"
#include "system.h"

const Keyboard::KeyboardRow Keyboard::KEYBOARD_LAYOUT[ROWS] = {
    // FN Row
    {
        .setToZeroMask = 0xffff,
        .pins = {
            { KeyCode::KEY_SIDE1,   GPIOA_PIN_KEYBOARD_0 },
            { KeyCode::KEY_SIDE2,   GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_INVALID, GPIOA_PIN_KEYBOARD_1 },
            { KeyCode::KEY_INVALID, GPIOA_PIN_KEYBOARD_1 }
        }
    },
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
    }
};

Keyboard::Keyboard(System::SystemTask& systask)
    : systask{ systask },
    mKeyPtt(false),
    mPrevStatePtt(KeyState::KEY_RELEASED),
    mKeyPressed(KeyCode::KEY_INVALID),
    mPrevKeyPressed(KeyCode::KEY_INVALID),
    mWasFKeyPressed(false)
{

    mPrevKeyState = KeyState::KEY_RELEASED;
    mLongPressTimer = 0;

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
    uint16_t reg, reg2;
    uint8_t ii;
    uint8_t k;

    // Handle PTT key
    mKeyPtt = !GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT);
    if (mPrevStatePtt == KeyState::KEY_PRESSED && !mKeyPtt) {
        pushKeyMessage(KeyCode::KEY_PTT, KeyState::KEY_RELEASED);
        mPrevStatePtt = KeyState::KEY_RELEASED;
        return;
    }
    else if (mPrevStatePtt == KeyState::KEY_RELEASED && mKeyPtt) {
        pushKeyMessage(KeyCode::KEY_PTT, KeyState::KEY_PRESSED);
        mPrevStatePtt = KeyState::KEY_PRESSED;
        return;
    }
    else if (mPrevStatePtt == KeyState::KEY_PRESSED && mKeyPtt) {
        return;
    }

    // Read matrix keyboard

    mKeyPressed = KeyCode::KEY_INVALID;
    // Scan main matrix
    for (uint8_t i = 0; i < ROWS; i++) {
        // Reset rows
        GPIOA->DATA |= (1u << GPIOA_PIN_KEYBOARD_4) |
            (1u << GPIOA_PIN_KEYBOARD_5) |
            (1u << GPIOA_PIN_KEYBOARD_6) |
            (1u << GPIOA_PIN_KEYBOARD_7);

        GPIOA->DATA &= KEYBOARD_LAYOUT[i].setToZeroMask;

        for (ii = 0, k = 0, reg = 0; ii < 3 && k < 8; ii++, k++) {
            delayUs(1);
            reg2 = (uint16_t)GPIOA->DATA;
            if (reg != reg2) { // noise
                reg = reg2;
                ii = 0;
            }
        }
        if (ii < 3)
            break; // noise is too bad

        for (uint8_t j = 0; j < COLS; j++) {
            const uint16_t mask = 1u << KEYBOARD_LAYOUT[i].pins[j].pin; 
            if (!(reg & mask)) {
                mKeyPressed = KEYBOARD_LAYOUT[i].pins[j].key;
                break;
            }
        }
        if (mKeyPressed != KeyCode::KEY_INVALID) {
            break;
        }
    }

    resetGPIO();
}

void Keyboard::processKeys() {
    TickType_t currentTick = xTaskGetTickCount();

    if (mKeyPressed != KeyCode::KEY_INVALID) {
        if (mPrevKeyState == KeyState::KEY_RELEASED) {
            if (mWasFKeyPressed) {
                pushKeyMessage(mKeyPressed, KeyState::KEY_PRESSED_WITH_F);
                mPrevKeyState = KeyState::KEY_PRESSED_WITH_F;
                mWasFKeyPressed = false;
            }
            else {
                pushKeyMessage(mKeyPressed, KeyState::KEY_PRESSED);
                mPrevKeyState = KeyState::KEY_PRESSED;
            }

            if (mKeyPressed == KeyCode::KEY_F) {
                mWasFKeyPressed = true;
            }

            mLongPressTimer = currentTick;
            mPrevKeyPressed = mKeyPressed;
        }
        else if (mPrevKeyState == KeyState::KEY_PRESSED) {
            TickType_t elapsedTime = currentTick - mLongPressTimer;
            if (elapsedTime >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
                mLongPressTimer = 0;
                mPrevKeyState = KeyState::KEY_LONG_PRESSED;
            }
        }
        else if (mPrevKeyState == KeyState::KEY_LONG_PRESSED ||
            mPrevKeyState == KeyState::KEY_LONG_PRESSED_CONT) {
            pushKeyMessage(mKeyPressed, mPrevKeyState);
            mPrevKeyState = KeyState::KEY_LONG_PRESSED_CONT;
        }
    }
    else {
        if (mPrevKeyState != KeyState::KEY_RELEASED) {
            if (mPrevKeyState != KeyState::KEY_PRESSED_WITH_F &&
                mPrevKeyState != KeyState::KEY_LONG_PRESSED &&
                mPrevKeyState != KeyState::KEY_LONG_PRESSED_CONT) {
                pushKeyMessage(mPrevKeyPressed, KeyState::KEY_RELEASED);
            }

            if (mPrevKeyState == KeyState::KEY_LONG_PRESSED_CONT &&
                (mPrevKeyPressed == KeyCode::KEY_UP ||
                    mPrevKeyPressed == KeyCode::KEY_DOWN)) {
                pushKeyMessage(mPrevKeyPressed, KeyState::KEY_RELEASED);
            }

            mLongPressTimer = 0;
            mPrevKeyState = KeyState::KEY_RELEASED;
            mPrevKeyPressed = KeyCode::KEY_INVALID;
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