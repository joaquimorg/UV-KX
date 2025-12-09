#include "printf.h"
#include "messenger.h"
#include "system.h"
#include "u8g2.h"
#include "sys.h"
#include <cstring>

using namespace Applications;

void Messenger::init(void) {
    logLines.fill({});
    inputBuffer.fill('\0');
    logCount = 0;
    inputLen = 0;
    recallIndex = -1;
    inputMode = InputMode::Upper;
    lastKey = Keyboard::KeyCode::KEY_INVALID;
    lastKeyCycle = 0;
    lastKeyTime = 0;
    radio.setFSKRxEnabled(true);
}

void Messenger::update(void) {
    char rxBuf[64];
    while (radio.popFSKMessage(rxBuf, sizeof(rxBuf))) {
        addLog("< ", rxBuf);
        recallIndex = -1;
    }
    drawScreen();
}

void Messenger::timeout(void) {
    // TODO : handle timeout events if needed
}

void Messenger::drawScreen() {
    ui.clearDisplay();
    ui.setBlackColor();
    ui.lcd()->drawBox(0, 0, 128, 7);
    //ui.lcd()->drawHLine(0, 55, 128);
    ui.lcd()->drawBox(0, 56, 128, 8);
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "MESSENGER");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "-%s-",
        (inputMode == InputMode::Upper ? "ABC" : "123"));

    ui.setFont(Font::FONT_5_TR);
    //ui.setBlackColor();

    uint8_t y = 14;
    for (int i = static_cast<int>(logCount) - 1, shown = 0; i >= 0 && shown < static_cast<int>(MAX_LOG_LINES); --i, ++shown) {
        const auto& line = logLines[static_cast<size_t>(i)];
        ui.drawString(TextAlign::LEFT, 2, 0, y, true, false, false, line.data());
        y += 7;
    }

    ui.setFont(Font::FONT_8_TR);
    // draw text with trailing cursor underscore
    char displayBuf[MAX_MSG_LEN + 2] = {0};
    snprintf(displayBuf, sizeof(displayBuf), "%s_", inputBuffer.data());
    ui.drawString(TextAlign::LEFT, 2, 0, 62, false, false, false, displayBuf);
    recallIndex = -1; // typing resets recall

    // Popup with available characters and current selection
    if (popupLen > 0 && (getElapsedMilliseconds() - lastKeyTime) < MULTITAP_TIMEOUT_MS) {
        ui.setFont(Font::FONT_8_TR);
        uint8_t x = 4;
        const uint8_t y = 53;
        // compute total width
        uint8_t totalW = 2;
        for (uint8_t i = 0; i < popupLen; ++i) {
            char c[2] = { popupChars[i], '\0' };
            totalW = static_cast<uint8_t>(totalW + ui.lcd()->getStrWidth(c) + 3);
        }
        // background
        ui.setWhiteColor();
        ui.lcd()->drawBox(2, y - 9, totalW, 12);        // white background (color index inverted)
        ui.setBlackColor();
        for (uint8_t i = 0; i < popupLen; ++i) {
            char c[2] = { popupChars[i], '\0' };
            uint8_t w = static_cast<uint8_t>(ui.lcd()->getStrWidth(c));
            if (i == lastKeyCycle) {
                ui.drawString(TextAlign::LEFT, x, x + w, y, true, true, false, c); // white text on black box
            } else {
                ui.drawString(TextAlign::LEFT, x, x + w, y, true, false, false, c);
            }
            x = static_cast<uint8_t>(x + w + 3);
        }
    }

    ui.updateDisplay();
}

void Messenger::addLog(const char* prefix, const char* text) {
    if (!text || !prefix) return;

    std::array<char, MAX_MSG_LEN> combined{};
    snprintf(combined.data(), combined.size(), "%s%s", prefix, text);

    if (logCount < MAX_LOG_LINES) {
        logLines[logCount++] = combined;
    }
    else {
        for (size_t i = 1; i < MAX_LOG_LINES; ++i) {
            logLines[i - 1] = logLines[i];
        }
        logLines[MAX_LOG_LINES - 1] = combined;
    }
}

const char* Messenger::keyChars(Keyboard::KeyCode key) const {
    switch (key) {
    case Keyboard::KeyCode::KEY_1: return ",.-+#!?1";
    case Keyboard::KeyCode::KEY_2: return "ABC2";
    case Keyboard::KeyCode::KEY_3: return "DEF3";
    case Keyboard::KeyCode::KEY_4: return "GHI4";
    case Keyboard::KeyCode::KEY_5: return "JKL5";
    case Keyboard::KeyCode::KEY_6: return "MNO6";
    case Keyboard::KeyCode::KEY_7: return "PQRS7";
    case Keyboard::KeyCode::KEY_8: return "TUV8";
    case Keyboard::KeyCode::KEY_9: return "WXYZ9";
    case Keyboard::KeyCode::KEY_0: return " 0";
    default:
        return "";
    }
}

void Messenger::cycleInputMode() {
    inputMode = (inputMode == InputMode::Upper) ? InputMode::Numeric : InputMode::Upper;
}

void Messenger::handleInputKey(Keyboard::KeyCode key) {
    const char* chars = keyChars(key);
    size_t charSetLen = std::strlen(chars);
    if (charSetLen == 0)
        return;

    uint32_t now = getElapsedMilliseconds();
    bool sameKey = (key == lastKey) && (now - lastKeyTime < MULTITAP_TIMEOUT_MS);
    uint8_t cycle = sameKey ? static_cast<uint8_t>((lastKeyCycle + 1) % charSetLen) : 0;

    char outChar = chars[cycle];
    if (inputMode == InputMode::Numeric) {
        outChar = chars[charSetLen - 1]; // force digit
    } else if (outChar >= 'a' && outChar <= 'z') {
        outChar = static_cast<char>(outChar - 'a' + 'A');
    }

    if (sameKey && inputLen > 0) {
        inputBuffer[inputLen - 1] = outChar;
    } else if (inputLen < MAX_MSG_LEN - 1) {
        inputBuffer[inputLen++] = outChar;
        inputBuffer[inputLen] = '\0';
    }

    // Update popup state only for alpha mode
    if (inputMode == InputMode::Upper) {
        popupLen = static_cast<uint8_t>(std::min(charSetLen, popupChars.size()));
        for (uint8_t i = 0; i < popupLen; ++i) {
            popupChars[i] = chars[i];
        }
    } else {
        popupLen = 0;
    }
    lastKeyCycle = cycle;
    lastKey = key;
    lastKeyTime = now;
    recallIndex = -1; // typing cancels recall
}

void Messenger::backspace() {
    if (inputLen > 0) {
        inputBuffer[--inputLen] = '\0';
    }
    // reset multi-tap state and popup so next key starts fresh immediately
    lastKey = Keyboard::KeyCode::KEY_INVALID;
    lastKeyCycle = 0;
    lastKeyTime = 0;
    popupLen = 0;
}

void Messenger::sendMessage() {
    if (inputLen == 0) return;
    // drop the cursor underscore before sending
    inputBuffer[inputLen] = '\0';
    recallIndex = -1;
    addLog("> ", inputBuffer.data());
    bool ok = radio.sendFSKMessage(inputBuffer.data());
    if (!ok) {
        addLog("ERR: ", "TX FAILED");
    }
    inputLen = 0;
    inputBuffer[0] = '\0';
}

void Messenger::recallMessage(int8_t delta) {
    if (logCount == 0) return;
    recallIndex = static_cast<int8_t>(std::max<int>(-1, std::min<int>(logCount - 1, recallIndex + delta)));
    if (recallIndex < 0) {
        inputLen = 0;
        inputBuffer[0] = '\0';
        return;
    }
    size_t idx = static_cast<size_t>(logCount - 1 - recallIndex); // newest first
    const auto& line = logLines[idx];
    const char* msg = line.data();
    if (msg[0] == '>' && msg[1] == ' ') {
        msg += 2;
    }
    strncpy(inputBuffer.data(), msg, MAX_MSG_LEN - 1);
    inputBuffer[MAX_MSG_LEN - 1] = '\0';
    inputLen = static_cast<uint8_t>(strlen(inputBuffer.data()));
}

void Messenger::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    if (keyState != Keyboard::KeyState::KEY_PRESSED) {
        return;
    }

    switch (keyCode) {
    case Keyboard::KeyCode::KEY_EXIT:
        radio.setFSKRxEnabled(false);
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
        return;
    case Keyboard::KeyCode::KEY_UP:
        recallMessage(-1);
        break;
    case Keyboard::KeyCode::KEY_DOWN:
        recallMessage(+1);
        break;
    case Keyboard::KeyCode::KEY_MENU: // send
        sendMessage();
        break;
    case Keyboard::KeyCode::KEY_STAR: // cycle keyboard
        cycleInputMode();
        break;
    case Keyboard::KeyCode::KEY_F: // backspace
        backspace();
        break;
    case Keyboard::KeyCode::KEY_0:
    case Keyboard::KeyCode::KEY_1:
    case Keyboard::KeyCode::KEY_2:
    case Keyboard::KeyCode::KEY_3:
    case Keyboard::KeyCode::KEY_4:
    case Keyboard::KeyCode::KEY_5:
    case Keyboard::KeyCode::KEY_6:
    case Keyboard::KeyCode::KEY_7:
    case Keyboard::KeyCode::KEY_8:
    case Keyboard::KeyCode::KEY_9:
        handleInputKey(keyCode);
        break;
    default:
        break;
    }

}
