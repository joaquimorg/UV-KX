#pragma once

#include <array>
#include <cstdint>
#include "apps.h"
#include "radio.h"

namespace Applications {

    class Messenger : public Application {
    public:
        Messenger(System::SystemTask& systask, UI& ui, RadioNS::Radio& radio)
            : Application(systask, ui), radio{ radio } {}

        void init(void) override;
        void update(void) override;
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) override;
        void timeout(void) override;

    private:
        enum class InputMode : uint8_t { Upper = 0, Numeric };

        static constexpr size_t MAX_LOG_LINES = 6;
        static constexpr size_t MAX_MSG_LEN = 64;
        static constexpr uint32_t MULTITAP_TIMEOUT_MS = 600;

        RadioNS::Radio& radio;

        std::array<std::array<char, MAX_MSG_LEN>, MAX_LOG_LINES> logLines{};
        uint8_t logCount = 0;

        std::array<char, MAX_MSG_LEN> inputBuffer{};
        uint8_t inputLen = 0;
        int8_t recallIndex = -1;
        InputMode inputMode = InputMode::Upper;
        Keyboard::KeyCode lastKey = Keyboard::KeyCode::KEY_INVALID;
        uint32_t lastKeyTime = 0;
        uint8_t lastKeyCycle = 0;
        std::array<char, 8> popupChars{};
        uint8_t popupLen = 0;
        bool popupVisible = false;

        void drawScreen();
        void addLog(const char* prefix, const char* text);
        void handleInputKey(Keyboard::KeyCode key);
        const char* keyChars(Keyboard::KeyCode key) const;
        void cycleInputMode();
        void sendMessage();
        void backspace();
        void recallMessage(int8_t delta);
    };
} // namespace Applications
