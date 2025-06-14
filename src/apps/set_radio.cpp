#include "printf.h"
#include "apps.h"
#include "set_vfo.h" // Included for potential navigation
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void SetRadio::drawScreen(void) {

    ui.clearDisplay();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);
    
    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, "RADIO");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    // Draw the radio settings list items starting at Y position 15
    menulist.draw(15, getCurrentOption());

    if (optionSelected != 0) {
        optionlist.drawPopup(ui, true);
    }

    ui.updateDisplay();
}


void SetRadio::init(void) {
    menulist.set(0, 6, 127, "MIC DB\nBATT SAVE\nBUSY LOCKOUT\nBLIGHT LEVEL\nBLIGHT TIME\nBLIGHT MODE\nLCD CONTRAST\nTX TOT\nBEEP\nRESET");
}

void SetRadio::update(void) {
    drawScreen();
}

void SetRadio::timeout(void) {
    settings.scheduleSaveIfNeeded();
    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
};

const char* SetRadio::getCurrentOption() {
    switch (menulist.getListPos() + 1) {
    case 1:
        return ui.getStrValue(Settings::MicDBStr, (uint8_t)settings.radioSettings.micDB - 1);
    case 2:
        return ui.getStrValue(Settings::onoffStr, (uint8_t)settings.radioSettings.batterySave);
    case 3:
        return ui.getStrValue(Settings::onoffStr, (uint8_t)settings.radioSettings.busyLockout);
    case 4:
        return ui.getStrValue(Settings::BacklightLevelStr, settings.radioSettings.backlightLevel);
    case 5:
        return ui.getStrValue(Settings::BacklightTimeStr, (uint8_t)settings.radioSettings.backlightTime);
    case 6:
        return ui.getStrValue(Settings::BacklightModeStr, (uint8_t)settings.radioSettings.backlightMode);
    case 7:
        return ui.getStrValue(Settings::LCDContrastStr, settings.radioSettings.lcdContrast);
    case 8:
        return ui.getStrValue(Settings::TXTimeoutStr, (uint8_t)settings.radioSettings.txTOT);
    case 9:
        return ui.getStrValue(Settings::onoffStr, (uint8_t)settings.radioSettings.beep);
    default:
        return NULL;
    }
}

void SetRadio::loadOptions() {
    switch (optionSelected) {
    case 1:
        optionlist.set((uint8_t)settings.radioSettings.micDB - 1, 5, 0, Settings::MicDBStr);
        break;
    case 2:
        optionlist.set((uint8_t)settings.radioSettings.batterySave, 5, 0, Settings::onoffStr);
        break;
    case 3:
        optionlist.set((uint8_t)settings.radioSettings.busyLockout, 5, 0, Settings::onoffStr);
        break;
    case 4:
        optionlist.set(settings.radioSettings.backlightLevel, 5, 0, Settings::BacklightLevelStr);
        break;
    case 5:
        optionlist.set((uint8_t)settings.radioSettings.backlightTime, 5, 0, Settings::BacklightTimeStr);
        break;
    case 6:
        optionlist.set((uint8_t)settings.radioSettings.backlightMode, 5, 0, Settings::BacklightModeStr);
        break;
    case 7:
        optionlist.set(settings.radioSettings.lcdContrast, 5, 0, Settings::LCDContrastStr);
        break;
    case 8:
        optionlist.set((uint8_t)settings.radioSettings.txTOT, 5, 0, Settings::TXTimeoutStr);
        break;
    case 9:
        optionlist.set((uint8_t)settings.radioSettings.beep, 5, 0, Settings::onoffStr);
        break;
    default:
        optionSelected = 0;
        break;
    }
}

void SetRadio::setOptions() {
    uint8_t sel = optionlist.getListPos();
    switch (optionSelected) {
    case 1:
        settings.radioSettings.micDB = (Settings::MicDB)(sel + 1);
        break;
    case 2:
        settings.radioSettings.batterySave = (Settings::ONOFF)sel;
        break;
    case 3:
        settings.radioSettings.busyLockout = (Settings::ONOFF)sel;
        break;
    case 4:
        settings.radioSettings.backlightLevel = sel & 0x0F;        
        //systask.pushMessage(System::SystemTask::SystemMSG::MSG_BKCLIGHT_LEVEL, (uint32_t)settings.radioSettings.backlightLevel);
        break;
    case 5:
        settings.radioSettings.backlightTime = (Settings::BacklightTime)sel;
        break;
    case 6:
        settings.radioSettings.backlightMode = (Settings::BacklightMode)sel;
        break;
    case 7:
        settings.radioSettings.lcdContrast = sel & 0x0F;
        break;
    case 8:
        settings.radioSettings.txTOT = (Settings::TXTimeout)sel;
        break;
    case 9:
        settings.radioSettings.beep = (Settings::ONOFF)sel;
        break;
    default:
        break;
    }
}

/**
 * @brief Handles keyboard input for the SetRadio application.
 * This function processes key presses for navigating the settings list,
 * selecting a setting (MENU key), or exiting the menu (EXIT key).
 * @param keyCode The code of the key that was pressed or released.
 * @param keyState The state of the key (e.g., KEY_PRESSED, KEY_LONG_PRESSED_CONT).
 */
void SetRadio::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    if (optionSelected == 0) {
        if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                menulist.prev();
            } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                menulist.next();
            } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                settings.scheduleSaveIfNeeded();
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
            } else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                inputSelect = 0; // reset direct input
                uint8_t idx = menulist.getListPos() + 1;
                if (idx == 10) {
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::RESETEEPROM);
                } else {
                    optionSelected = idx;
                    optionlist.setPopupTitle(menulist.getStringLine());
                    loadOptions();
                }
             } else if (keyCode >= Keyboard::KeyCode::KEY_0 &&
                       keyCode <= Keyboard::KeyCode::KEY_9 &&
                       keyState == Keyboard::KeyState::KEY_PRESSED) {
                uint8_t num = ui.keycodeToNumber(keyCode);
                inputSelect = (inputSelect == 0) ? num
                                                 : static_cast<uint8_t>((inputSelect * 10) + num);

                if (inputSelect > menulist.getTotal() || inputSelect == 0) {
                    inputSelect = 0;
                } else {
                    menulist.setCurrentPos(inputSelect - 1);
                    if (inputSelect * 10 > menulist.getTotal() && inputSelect >= 10) {
                        inputSelect = 0;
                    }
                }
            }            
        }
    } else {
        if (keyState == Keyboard::KeyState::KEY_PRESSED) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                optionlist.prev();
            } else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                optionlist.next();
            } else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                optionSelected = 0;
            } else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                setOptions();
                settings.applyRadioSettings();                
                optionSelected = 0;
            }
        }
    }
}
