#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

const char* SetVFO::codeValue(Settings::CodeType type, uint8_t code, SelectionList& list)
{
    switch (type) {
    case Settings::CodeType::CT:
        list.setSuffix(ui.HZStr);
        return ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), code);
    case Settings::CodeType::DCS:
        list.setSuffix("I");
        return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), code);
    case Settings::CodeType::NDCS:
        list.setSuffix("N");
        return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), code);
    default:
        return nullptr;
    }
}

bool SetVFO::configureCodeList(Settings::CodeType type, uint8_t code)
{
    switch (type) {
    case Settings::CodeType::CT:
        optionlist.set(code, 5, 0, ui.generateCTDCList(Settings::CTCSSOptions, 50), ui.HZStr);
        return true;
    case Settings::CodeType::DCS:
        optionlist.set(code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "I");
        return true;
    case Settings::CodeType::NDCS:
        optionlist.set(code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "N");
        return true;
    default:
        return false;
    }
}

void SetVFO::drawScreen(void) {

    ui.clearDisplay();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 6, false, false, false, "%s %s", ui.VFOStr, vfoab == Settings::VFOAB::VFOA ? "A" : "B");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    menulist.draw(15, getCurrentOption());

    if (optionSelected != 0) {
        optionlist.drawPopup(ui, true);
    }
    if (userOptionSelected != 0) {
        // display user input
        ui.drawPopupWindow(36, 15, 90, 34, menulist.getStringLine());
    }

    ui.updateDisplay();
}

void SetVFO::init(void) {
    menulist.set(0, 6, 127, "SQUELCH\nSTEP\nMODE\nBANDWIDTH\nTX POWER\nSHIFT\nOFFSET\nRX CODE TYPE\nRX CODE\nTX CODE TYPE\nTX CODE\nTX STE\nRX STE\nCOMPANDER\nRX ACG\nPTT ID\nROGER");
    vfo = radio.getVFO(vfoab);
}

void SetVFO::update(void) {
    drawScreen();
}

void SetVFO::timeout(void) {
    if (optionSelected == 0 && userOptionSelected == 0) {
        settings.radioSettings.vfo[(uint8_t)vfoab] = vfo;
        settings.scheduleSaveIfNeeded();
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
    }
    else {
        //optionSelected = 0;
        //userOptionSelected = 0;
        inputSelect = 0;
    }
};

const char* SetVFO::getCurrentOption() {
    menulist.setSuffix(NULL);
    switch (menulist.getListPos() + 1) {
    case 1: // SQUELCH
        return ui.getStrValue(Settings::squelchStr, (uint8_t)vfo.squelch);
    case 2: // STEP
        menulist.setSuffix(ui.KHZStr);
        return ui.getStrValue(Settings::stepStr, (uint8_t)vfo.step);
    case 3: // MODE
        return ui.getStrValue(Settings::modulationStr, (uint8_t)vfo.modulation);
    case 4: // BANDWIDTH
        menulist.setSuffix(ui.KHZStr);
        return ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo.bw);
    case 5: // TX POWER
        return ui.getStrValue(Settings::powerStr, (uint8_t)vfo.power);
    case 6: // SHIFT
        return ui.getStrValue(Settings::offsetStr, (uint8_t)vfo.shift);
    case 7: // OFFSET
        menulist.setSuffix(ui.KHZStr);
        return "0.00";//ui.getStrValue(Settings::offsetStr, vfo.rx.frequency - vfo.tx.frequency);
    case 8: // RX CODE TYPE
        return ui.getStrValue(Settings::codetypeStr, (uint8_t)vfo.rx.codeType);
    case 10: // TX CODE TYPE
        return ui.getStrValue(Settings::codetypeStr, (uint8_t)vfo.tx.codeType);
    case 9: // RX CODE
        return codeValue(vfo.rx.codeType, vfo.rx.code, menulist);
    case 11: // TX CODE
        return codeValue(vfo.tx.codeType, vfo.tx.code, menulist);
    case 12: // TX STE
        return ui.getStrValue(Settings::onoffStr, (uint8_t)vfo.repeaterSte);
    case 13: // RX STE
        return ui.getStrValue(Settings::onoffStr, (uint8_t)vfo.ste);
    case 14: // COMPANDER
        return ui.getStrValue(Settings::txrxStr, (uint8_t)vfo.compander);    
    case 15: // RX ACG
        if (vfo.rxagc < ui.stringLengthNL(Settings::AGCStr) - 1) {
            menulist.setSuffix(ui.DBStr);
        }
        return ui.getStrValue(Settings::AGCStr, (uint8_t)vfo.rxagc);
    case 16: // PTT ID
        return ui.getStrValue(Settings::pttIDStr, (uint8_t)vfo.pttid);
    case 17: // ROGER
        return ui.getStrValue(Settings::rogerStr, (uint8_t)vfo.roger);
    default:
        return NULL;
    }
}

void SetVFO::loadOptions() {
    switch (optionSelected) {
    case 1: // SQUELCH
        optionlist.set((uint8_t)vfo.squelch, 5, 0, Settings::squelchStr);
        break;
    case 2: // STEP
        optionlist.set((uint8_t)vfo.step, 5, 0, Settings::stepStr, ui.KHZStr);
        break;
    case 3: // MODE
        optionlist.set((uint8_t)vfo.modulation, 5, 0, Settings::modulationStr);
        break;
    case 4: // BANDWIDTH
        optionlist.set((uint8_t)vfo.bw, 5, 0, Settings::bandwidthStr, ui.KHZStr);
        break;
    case 5: // TX POWER
        optionlist.set((uint8_t)vfo.power, 5, 0, Settings::powerStr);
        break;
    case 6: // SHIFT
        optionlist.set((uint8_t)vfo.shift, 3, 0, Settings::offsetStr);
        break;
    case 7: // OFFSET
        userOptionInput = uint32_t(vfo.rx.frequency - vfo.tx.frequency);
        break;
    case 8: // RX CODE TYPE
        optionlist.set((uint8_t)vfo.rx.codeType, 5, 0, Settings::codetypeStr);
        break;
    case 10: // TX CODE TYPE
        optionlist.set((uint8_t)vfo.tx.codeType, 5, 0, Settings::codetypeStr);
        break;
    case 9: // RX CODE
        if (!configureCodeList(vfo.rx.codeType, vfo.rx.code)) {
            optionSelected = 0;
        }
        break;
    case 11: // TX CODE
        if (!configureCodeList(vfo.tx.codeType, vfo.tx.code)) {
            optionSelected = 0;
        }
        break;
    case 12: // TX STE
        optionlist.set((uint8_t)vfo.repeaterSte, 5, 0, Settings::onoffStr);
        break;
    case 13: // RX STE
        optionlist.set((uint8_t)vfo.ste, 5, 0, Settings::onoffStr);
        break;
    case 14: // COMPANDER
        optionlist.set((uint8_t)vfo.compander, 5, 0, Settings::txrxStr);
        break;    
    case 15: // RX ACG
        optionlist.set((uint8_t)vfo.rxagc, 5, 0, Settings::AGCStr, ui.DBStr);
        break;
    case 16: // PTT ID
        optionlist.set((uint8_t)vfo.pttid, 5, 0, Settings::pttIDStr);
        break;
    case 17: // ROGER
        optionlist.set((uint8_t)vfo.roger, 5, 0, Settings::rogerStr);
        break;
    default:
        break;
    }

}

void SetVFO::setOptions() {
    uint8_t optionlistSelected = optionlist.getListPos();
    switch (optionSelected) {
    case 1: // SQUELCH
        vfo.squelch = optionlistSelected & 0xF;
        break;
    case 2: // STEP
        vfo.step = (Settings::Step)optionlistSelected;
        break;
    case 3: // MODE
        vfo.modulation = (ModType)optionlistSelected;
        break;
    case 4: // BANDWIDTH
        vfo.bw = (BK4819_Filter_Bandwidth)optionlistSelected;
        break;
    case 5: // TX POWER
        vfo.power = (Settings::TXOutputPower)optionlistSelected;
        break;
    case 6: // SHIFT
        vfo.shift = (Settings::OffsetDirection)optionlistSelected;
        break;
    case 7: // OFFSET
        if (vfo.shift == Settings::OffsetDirection::OFFSET_PLUS) {
            vfo.tx.frequency = static_cast<unsigned int>(vfo.rx.frequency + userOptionInput) & 0x7FFFFFF;
        }
        else if (vfo.shift == Settings::OffsetDirection::OFFSET_MINUS) {
            vfo.tx.frequency = static_cast<unsigned int>(vfo.rx.frequency - userOptionInput) & 0x7FFFFFF;
        }
        break;
    case 8: // RX CODE TYPE
        vfo.rx.codeType = (Settings::CodeType)optionlistSelected;
        break;
    case 10: // TX CODE TYPE
        vfo.tx.codeType = (Settings::CodeType)optionlistSelected;
        break;
    case 9: // RX CODE
        vfo.rx.code = optionlistSelected;
        break;
    case 11: // TX CODE
        vfo.tx.code = optionlistSelected;
        break;
    case 12: // TX STE
        vfo.repeaterSte = (Settings::ONOFF)optionlistSelected;
        break;
    case 13: // RX STE
        vfo.ste = (Settings::ONOFF)optionlistSelected;
        break;
    case 14: // COMPANDER
        vfo.compander = (Settings::TXRX)optionlistSelected;
        break;    
    case 15: // RX ACG
        vfo.rxagc = optionlistSelected & 0x1F;
        break;
    case 16: // PTT ID
        vfo.pttid = optionlistSelected & 0xF;
        break;
    case 17: // ROGER
        vfo.roger = optionlistSelected & 0xF;
        break;
    default:
        break;
    }
}

void SetVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {
    uint8_t optionListSelected = 0;

    if (optionSelected == 0 && userOptionSelected == 0) {
        switch (keyCode) {
            case Keyboard::KeyCode::KEY_UP:
                if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
                    menulist.prev();
                }
                break;
            case Keyboard::KeyCode::KEY_DOWN:
                if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
                    menulist.next();
                }
                break;
            case Keyboard::KeyCode::KEY_EXIT:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    settings.radioSettings.vfo[(uint8_t)vfoab] = vfo;
                    settings.scheduleSaveIfNeeded();
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
                }
                break;
            case Keyboard::KeyCode::KEY_MENU:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    inputSelect = 0;
                    optionListSelected = menulist.getListPos() + 1;
                    if (optionListSelected == 7) {
                        // not a list, user input
                        if (vfo.shift == Settings::OffsetDirection::OFFSET_NONE) {
                            systask.pushMessage(System::SystemTask::SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
                        } else {
                            userOptionSelected = optionListSelected;
                        }
                    } else {
                        optionSelected = optionListSelected;
                        optionlist.setPopupTitle(menulist.getStringLine());
                    }
                    loadOptions();
                }
                break;
            default:
                if ((keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) && keyState == Keyboard::KeyState::KEY_PRESSED) {
                    inputSelect = (inputSelect == 0) ? ui.keycodeToNumber(keyCode) : static_cast<uint8_t>((inputSelect * 10) + ui.keycodeToNumber(keyCode));
                    if (inputSelect > menulist.getTotal()) {
                        inputSelect = 0;
                    } else {
                        menulist.setCurrentPos(inputSelect - 1);
                        if (inputSelect >= 10) {
                            inputSelect = 0;
                        }
                    }
                }
                break;
        }
    } else {
        switch (keyCode) {
            case Keyboard::KeyCode::KEY_UP:
                if (keyState == Keyboard::KeyState::KEY_PRESSED && optionSelected != 0) {
                    optionlist.prev();
                }
                break;
            case Keyboard::KeyCode::KEY_DOWN:
                if (keyState == Keyboard::KeyState::KEY_PRESSED && optionSelected != 0) {
                    optionlist.next();
                }
                break;
            case Keyboard::KeyCode::KEY_EXIT:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    optionSelected = 0;
                    userOptionSelected = 0;
                }
                break;
            case Keyboard::KeyCode::KEY_MENU:
                if (keyState == Keyboard::KeyState::KEY_PRESSED) {
                    setOptions();
                    radio.setVFO(vfoab, vfo);
                    optionSelected = 0;
                    userOptionSelected = 0;
                    settings.radioSettings.vfo[(uint8_t)vfoab] = vfo;
                    settings.scheduleSaveIfNeeded();
                }
                break;
            default:
               break;
        }
    }

}
