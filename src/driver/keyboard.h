#pragma once

#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

namespace System {
    class SystemTask;
}

class Keyboard {
public:
    // Key codes enum
    enum class KeyCode {
        KEY_0 = 0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_MENU,
        KEY_UP,
        KEY_DOWN,
        KEY_EXIT,
        KEY_STAR,
        KEY_F,
        KEY_PTT,
        KEY_SIDE2,
        KEY_SIDE1,
        KEY_INVALID
    };

    // Key states enum
    enum class KeyState {
        KEY_RELEASED = 0,
        KEY_PRESSED,
        KEY_PRESSED_WITH_F,
        KEY_LONG_PRESSED,
        KEY_LONG_PRESSED_CONT
    };

    // Constructor
    Keyboard(System::SystemTask& systask);
    
    // Initialize the keyboard
    void init();
    
    // Get current F-key state
    bool wasFKeyPressed() const { return mWasFKeyPressed; }


private:

    System::SystemTask& systask;

    static constexpr uint8_t ROWS = 5;
    static constexpr uint8_t COLS = 4;
    static constexpr uint32_t LONG_PRESS_TIME = 500;

    // Internal structures
    struct KeyPin {
        KeyCode key;
        uint8_t pin;
    };

    struct KeyboardRow {
        uint16_t setToZeroMask;
        KeyPin pins[4];
    };

    // Member variables
    bool mKeyState[ROWS][COLS];
    bool mKeyPtt;
    KeyState mPrevKeyState[ROWS][COLS];
    KeyState mPrevStatePtt;
    TickType_t mLongPressTimer[ROWS][COLS];
    bool mWasFKeyPressed;
    
    // Task related
    StaticTask_t mKeyTaskBuffer;
    StackType_t mKeyTaskStack[configMINIMAL_STACK_SIZE];
    
    // Keyboard layout configuration
    static const KeyboardRow KEYBOARD_LAYOUT[ROWS];

    // Private methods
    void readKeyboard();
    void processKeys();
    static void keyTaskWrapper(void* parameter);
    void keyTask();

    // Helper methods
    void pushKeyMessage(KeyCode key, KeyState state);
    void resetGPIO();
};