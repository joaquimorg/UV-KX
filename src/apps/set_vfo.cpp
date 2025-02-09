#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

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
    menulist.set(0, 6, 127, "SQUELCH\nSTEP\nMODE\nBANDWIDTH\nTX POWER\nSHIFT\nOFFSET\nRX CODE TYPE\nRX CODE\nTX CODE TYPE\nTX CODE\nTX STE\nRX STE\nCOMPANDER\nPTT ID\nRX ACG");
    vfo = radio.getVFO(vfoab);
}

void SetVFO::update(void) {
    drawScreen();
}

void SetVFO::timeout(void) {
    if (optionSelected == 0 && userOptionSelected == 0) {
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
        return ui.getStrValue(RadioNS::Radio::squelchStr, (uint8_t)vfo.squelch);
    case 2: // STEP
        menulist.setSuffix(ui.KHZStr);
        return ui.getStrValue(RadioNS::Radio::stepStr, (uint8_t)vfo.step);
    case 3: // MODE
        return ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfo.modulation);
    case 4: // BANDWIDTH
        menulist.setSuffix(ui.KHZStr);
        return ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfo.bw);
    case 5: // TX POWER
        return ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfo.power);
    case 6: // SHIFT
        return ui.getStrValue(RadioNS::Radio::offsetStr, (uint8_t)vfo.shift);
    case 7: // OFFSET
        menulist.setSuffix(ui.KHZStr);
        return "0.00";//ui.getStrValue(RadioNS::Radio::offsetStr, vfo.rx.frequency - vfo.tx.frequency);
    case 8: // RX CODE TYPE
        return ui.getStrValue(RadioNS::Radio::codetypeStr, (uint8_t)vfo.rx.codeType);
    case 10: // TX CODE TYPE
        return ui.getStrValue(RadioNS::Radio::codetypeStr, (uint8_t)vfo.tx.codeType);
    case 9: // RX CODE
        if (vfo.rx.codeType == Settings::CodeType::CT) {
            menulist.setSuffix(ui.HZStr);
            return ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo.rx.code);
        }
        else if (vfo.rx.codeType == Settings::CodeType::DCS) {
            menulist.setSuffix("I");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.rx.code);
        }
        else if (vfo.rx.codeType == Settings::CodeType::NDCS) {
            menulist.setSuffix("N");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.rx.code);
        }
        else {
            return NULL;
        }
    case 11: // TX CODE
        if (vfo.tx.codeType == Settings::CodeType::CT) {
            menulist.setSuffix(ui.HZStr);
            return ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo.tx.code);
        }
        else if (vfo.tx.codeType == Settings::CodeType::DCS) {
            menulist.setSuffix("I");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.tx.code);
        }
        else if (vfo.tx.codeType == Settings::CodeType::NDCS) {
            menulist.setSuffix("N");
            return ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo.tx.code);
        }
        else {
            return NULL;
        }
    case 12: // TX STE
        return ui.getStrValue(RadioNS::Radio::onoffStr, (uint8_t)vfo.repeaterSte);
    case 13: // RX STE
        return ui.getStrValue(RadioNS::Radio::onoffStr, (uint8_t)vfo.ste);
    case 14: // COMPANDER
        return ui.getStrValue(RadioNS::Radio::txrxStr, (uint8_t)vfo.compander);
    case 15: // PTT ID
        return ui.getStrValue("0\n1", (uint8_t)vfo.pttid);
    case 16: // RX ACG
        if (vfo.rxagc < ui.stringLengthNL(RadioNS::Radio::AGCStr) - 1) {
            menulist.setSuffix(ui.DBStr);
        }
        return ui.getStrValue(RadioNS::Radio::AGCStr, (uint8_t)vfo.rxagc);
    default:
        return NULL;
    }
}

void SetVFO::loadOptions() {
    switch (optionSelected) {
    case 1: // SQUELCH
        optionlist.set((uint8_t)vfo.squelch, 5, 0, RadioNS::Radio::squelchStr);
        break;
    case 2: // STEP
        optionlist.set((uint8_t)vfo.step, 5, 0, RadioNS::Radio::stepStr, ui.KHZStr);
        break;
    case 3: // MODE
        optionlist.set((uint8_t)vfo.modulation, 5, 0, RadioNS::Radio::modulationStr);
        break;
    case 4: // BANDWIDTH
        optionlist.set((uint8_t)vfo.bw, 5, 0, RadioNS::Radio::bandwidthStr, ui.KHZStr);
        break;
    case 5: // TX POWER
        optionlist.set((uint8_t)vfo.power, 5, 0, RadioNS::Radio::powerStr);
        break;
    case 6: // SHIFT
        optionlist.set((uint8_t)vfo.shift, 3, 0, RadioNS::Radio::offsetStr);
        break;
    case 7: // OFFSET
        userOptionInput = uint32_t(vfo.rx.frequency - vfo.tx.frequency);
        break;
    case 8: // RX CODE TYPE
        optionlist.set((uint8_t)vfo.rx.codeType, 5, 0, RadioNS::Radio::codetypeStr);
        break;
    case 10: // TX CODE TYPE
        optionlist.set((uint8_t)vfo.tx.codeType, 5, 0, RadioNS::Radio::codetypeStr);
        break;
    case 9: // RX CODE
        if (vfo.rx.codeType == Settings::CodeType::CT) {
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::CTCSSOptions, 50), ui.HZStr);
        }
        else if (vfo.rx.codeType == Settings::CodeType::DCS) {
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "I");
        }
        else if (vfo.rx.codeType == Settings::CodeType::NDCS) {
            optionlist.set((uint8_t)vfo.rx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "N");
        }
        else {
            optionSelected = 0;
        }
        break;
    case 11: // TX CODE
        if (vfo.tx.codeType == Settings::CodeType::CT) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::CTCSSOptions, 50), ui.HZStr);
        }
        else if (vfo.tx.codeType == Settings::CodeType::DCS) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "I");
        }
        else if (vfo.tx.codeType == Settings::CodeType::NDCS) {
            optionlist.set((uint8_t)vfo.tx.code, 5, 0, ui.generateCTDCList(Settings::DCSOptions, 104, false), "N");
        }
        else {
            optionSelected = 0;
        }
        break;
    case 12: // TX STE
        optionlist.set((uint8_t)vfo.repeaterSte, 5, 0, RadioNS::Radio::onoffStr);
        break;
    case 13: // RX STE
        optionlist.set((uint8_t)vfo.ste, 5, 0, RadioNS::Radio::onoffStr);
        break;
    case 14: // COMPANDER
        optionlist.set((uint8_t)vfo.compander, 5, 0, RadioNS::Radio::txrxStr);
        break;
    case 15: // PTT ID
        optionlist.set((uint8_t)vfo.pttid, 5, 0, "0\n1");
        break;
    case 16: // RX ACG
        optionlist.set((uint8_t)vfo.rxagc, 5, 0, RadioNS::Radio::AGCStr, ui.DBStr);
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
    case 15: // PTT ID
        vfo.pttid = optionlistSelected & 0xF;
        break;
    case 16: // RX ACG
        vfo.rxagc = optionlistSelected & 0x1F;
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
                    // TODO : save ???
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
                }
                break;
            default:
               break;
        }
    }

}
