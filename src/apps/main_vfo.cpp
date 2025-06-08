#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"
#include <cstring>

using namespace Applications;

void MainVFO::drawScreen(void) {

    Settings::VFOAB activeVFO1 = radio.getCurrentVFO();
    Settings::VFOAB activeVFO2 = activeVFO1 == Settings::VFOAB::VFOA ? Settings::VFOAB::VFOB : Settings::VFOAB::VFOA;

    Settings::VFO vfo1 = radio.getVFO(activeVFO1);
    Settings::VFO vfo2 = radio.getVFO(activeVFO2);
    bool rxVFO1 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO1 == radio.getRXVFO());
    bool rxVFO2 = (radio.getState() == Settings::RadioState::RX_ON && activeVFO2 == radio.getRXVFO());

    ui.clearDisplay();

    ui.lcd()->setColorIndex(BLACK);

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 1, 0, 6, false, false, false, vfo1.name);

    ui.setFont(Font::FONT_5_TR);
    const char* powerA = ui.getStrValue(Settings::powerStr, (uint8_t)vfo1.power);
    const char* bandwidthA = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo1.bw);
    const char* modulationA = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo1.modulation);
    const char* rxCode;
    const char* txCode;
    uint8_t codeXend = 127;

    if (vfo1.rx.codeType == Settings::CodeType::CT) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, ui.HZStr);
        codeXend -= 48;
    }
    else if (vfo1.rx.codeType == Settings::CodeType::DCS || vfo1.rx.codeType == Settings::CodeType::NDCS) {
        rxCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.rx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, radio.isRXToneDetected(), false, "%s %.*s%s", ui.RXStr, ui.stringLengthNL(rxCode), rxCode, vfo1.rx.codeType == Settings::CodeType::NDCS ? "N" : "I");
        codeXend -= 48;
    }

    if (vfo1.tx.codeType == Settings::CodeType::CT) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::CTCSSOptions, 50), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, ui.HZStr);
    }
    else if (vfo1.tx.codeType == Settings::CodeType::DCS || vfo1.tx.codeType == Settings::CodeType::NDCS) {
        txCode = ui.getStrValue(ui.generateCTDCList(Settings::DCSOptions, 104, false), (uint8_t)vfo1.tx.code);
        ui.drawStringf(TextAlign::RIGHT, 0, codeXend, 26, true, false, false, "%s %.*s%s", ui.TXStr, ui.stringLengthNL(txCode), txCode, vfo1.tx.codeType == Settings::CodeType::NDCS ? "N" : "I");
    }

    ui.drawStringf(TextAlign::RIGHT, 0, 127, 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationA), modulationA, ui.stringLengthNL(bandwidthA), bandwidthA, ui.stringLengthNL(powerA), powerA);

    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    ui.drawString(TextAlign::LEFT, 0, 0, 22, true, false, false, ui.VFOStr);

    //ui.lcd()->setColorIndex(BLACK);
    //ui.lcd()->drawLine(5, 9, 5, 25);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 14, true, true, false, "%s", activeVFO1 == Settings::VFOAB::VFOA ? "A" : "B");

    if (rxVFO1) {
        ui.drawString(TextAlign::LEFT, 12, 0, 14, true, true, false, ui.RXStr);
    }

    if (showFreqInput) {
        ui.drawFrequencyBig(true, freqInput, 111, 19);
    }
    else {
        ui.drawFrequencyBig(rxVFO1, vfo1.rx.frequency, 111, 19);
    }

    uint8_t vfoBY = 28;

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawBox(0, vfoBY, 128, 7);

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::LEFT, 1, 0, vfoBY + 6, false, false, false, "%S", vfo2.name);

    const char* powerB = ui.getStrValue(Settings::powerStr, (uint8_t)vfo2.power);
    const char* bandwidthB = ui.getStrValue(Settings::bandwidthStr, (uint8_t)vfo2.bw);
    const char* modulationB = ui.getStrValue(Settings::modulationStr, (uint8_t)vfo2.modulation);

    ui.drawStringf(TextAlign::RIGHT, 0, 127, vfoBY + 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationB), modulationB, ui.stringLengthNL(bandwidthB), bandwidthB, ui.stringLengthNL(powerB), powerB);

    ui.drawFrequencySmall(rxVFO2, vfo2.rx.frequency, 126, vfoBY + 17);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, vfoBY + 15, true, false, true, "%s", activeVFO2 == Settings::VFOAB::VFOB ? "B" : "A");

    if ((rxVFO2 && vfo2.rx.codeType != Settings::CodeType::NONE && radio.isRXToneDetected()) 
        || (rxVFO2 && vfo2.rx.codeType == Settings::CodeType::NONE)) {
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, true, false, ui.RXStr);
    }
    else {
        ui.setFont(Font::FONT_8_TR);
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, ui.VFOStr);
    }

    ui.lcd()->setColorIndex(BLACK);

    showRSSI(1, 52);

    //ui.draw_dotline(0, 47, BLACK);

    // Status information

    if (systask.getBattery().isCharging()) {
        ui.draw_ic8_charging(118, 52, BLACK);
    }
    else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 114, 52);
    }

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage());

    if (systask.wasFKeyPressed()) {
        ui.drawString(TextAlign::RIGHT, 0, 97, 56, true, true, false, "F");
    }

    if (radio.getState() == Settings::RadioState::RX_ON) {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, radio.getRXVFO() == Settings::VFOAB::VFOA ? "A" : "B");
    }
    else {
        ui.drawString(TextAlign::RIGHT, 0, 108, 64, true, false, false, "A/B");
    }

    if (radio.isPowerSaveMode()) {
        //ui.drawString(TextAlign::RIGHT, 0, 80, 64, true, false, false, "PS");
        ui.draw_ps(78, 59, BLACK);
    }

    if (systask.getSettings().isRadioSavePending()) {
        ui.draw_save(68, 59, BLACK);
    }

    if (popupSelected != NONE) {
        popupList.drawPopup(ui);
    }

    ui.updateDisplay();
}

void MainVFO::showRSSI(uint8_t posX, uint8_t posY) {

    uint8_t sValue = 0;
    int16_t plusDB = 0;

    if (radio.getState() == Settings::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        sValue = radio.convertRSSIToSLevel(rssi_dBm); // Convert RSSI to S-level
        if (sValue == 10) {
            plusDB = radio.convertRSSIToPlusDB(rssi_dBm); // Convert to +dB value if greater than S9
        }
    }

    ui.drawRSSI(sValue, posX, posY + 1);

    ui.setFont(Font::FONT_8_TR);
    if (sValue > 0) {
        if (sValue == 10) {
            ui.drawString(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S9");
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 12, true, false, false, "+%idB", plusDB);
        }
        else {
            ui.drawStringf(TextAlign::LEFT, posX + 38, 0, posY + 5, true, false, false, "S%i", sValue);
        }
    }


}

void MainVFO::init(void) {
}

void MainVFO::update(void) {
    drawScreen();
}

void MainVFO::timeout(void) {
    if (popupSelected != NONE) {
        popupSelected = NONE;
    }
    if (showFreqInput) {
        showFreqInput = false;
    }
}

void MainVFO::savePopupValue(void) {
    Settings::VFO vfo = radio.getActiveVFO();
    Settings::VFO before = vfo;

    if (popupSelected == BANDWIDTH) {
        vfo.bw = (BK4819_Filter_Bandwidth)popupList.getListPos();
    }
    else if (popupSelected == MODULATION) {
        vfo.modulation = (ModType)popupList.getListPos();
    }
    else if (popupSelected == POWER) {
        vfo.power = (Settings::TXOutputPower)popupList.getListPos();
    }

    bool changed = memcmp(&before, &vfo, sizeof(Settings::VFO)) != 0;

    radio.setVFO(radio.getCurrentVFO(), vfo);
    radio.setupToVFO(radio.getCurrentVFO());
    if (changed) {
        systask.getSettings().radioSettings.vfo[(uint8_t)radio.getCurrentVFO()] = vfo;
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
    }
}

void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    Settings::VFO vfo = radio.getActiveVFO();

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {

        if (popupSelected != NONE) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                popupList.prev();
                savePopupValue();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                popupList.next();
                savePopupValue();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                popupSelected = NONE;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                popupSelected = NONE;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4 || keyCode == Keyboard::KeyCode::KEY_5 || keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.next();
                savePopupValue();
            }
        }
        else {

            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                Settings::VFO before = vfo;
                uint32_t newFrequency = vfo.rx.frequency + Settings::StepFrequencyTable[(uint8_t)vfo.step];
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x07FFFFFF);

                bool changed = memcmp(&before, &vfo, sizeof(Settings::VFO)) != 0;

                radio.setVFO(radio.getCurrentVFO(), vfo);
                radio.setupToVFO(radio.getCurrentVFO());
                if (changed) {
                    systask.getSettings().radioSettings.vfo[(uint8_t)radio.getCurrentVFO()] = vfo;
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                Settings::VFO before = vfo;
                uint32_t newFrequency = vfo.rx.frequency - Settings::StepFrequencyTable[(uint8_t)vfo.step];
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x7FFFFFF);

                bool changed = memcmp(&before, &vfo, sizeof(Settings::VFO)) != 0;

                radio.setVFO(radio.getCurrentVFO(), vfo);
                radio.setupToVFO(radio.getCurrentVFO());
                if (changed) {
                    systask.getSettings().radioSettings.vfo[(uint8_t)radio.getCurrentVFO()] = vfo;
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                if (showFreqInput) {
                    showFreqInput = false;
                    Settings::VFO before = vfo;
                    vfo.rx.frequency = (uint32_t)(freqInput & 0x7FFFFFF);
                    vfo.tx.frequency = (uint32_t)(freqInput & 0x7FFFFFF);
                    bool changed = memcmp(&before, &vfo, sizeof(Settings::VFO)) != 0;
                    radio.setVFO(radio.getCurrentVFO(), vfo);
                    radio.setupToVFO(radio.getCurrentVFO());
                    if (changed) {
                        systask.getSettings().radioSettings.vfo[(uint8_t)radio.getCurrentVFO()] = vfo;
                        systask.pushMessage(System::SystemTask::SystemMSG::MSG_SAVESETTINGS, 0);
                    }
                }
                else {
                    systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                if (showFreqInput) {
                    showFreqInput = false;
                }
            }
            else if (keyCode == Keyboard::KeyCode::KEY_STAR) {
                if (showFreqInput && freqInput > 0) {
                    freqInput /= 10;
                }
            }
            else if (keyCode >= Keyboard::KeyCode::KEY_0 && keyCode <= Keyboard::KeyCode::KEY_9) {
                if (!showFreqInput) {
                    showFreqInput = true;
                    freqInput = 0;

                }
                uint8_t number = ui.keycodeToNumber(keyCode);

                freqInput *= (uint32_t)(10 & 0x7FFFFFF);
                freqInput += (uint32_t)(number & 0x7FFFFFF);
                if (freqInput >= 999999999) {
                    showFreqInput = false;
                }
            }
        }
    }
    else if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED || keyState == Keyboard::KeyState::KEY_PRESSED_WITH_F) {

        if (!showFreqInput && popupSelected == NONE) {
            if (keyCode == Keyboard::KeyCode::KEY_2) {
                radio.changeActiveVFO();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4) {
                popupList.set((uint8_t)vfo.bw, 3, 0, Settings::bandwidthStr, ui.KHZStr);
                popupList.setPopupTitle("BANDWIDTH");
                popupSelected = BANDWIDTH;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_5) {
                popupList.set((uint8_t)vfo.modulation, 3, 0, Settings::modulationStr);
                popupList.setPopupTitle("MODULATION");
                popupSelected = MODULATION;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.set((uint8_t)vfo.power, 3, 0, Settings::powerStr);
                popupList.setPopupTitle("TX POWER");
                popupSelected = POWER;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, radio.getCurrentVFO() == Settings::VFOAB::VFOA ? (uint32_t)Applications::SETVFOA : (uint32_t)Applications::SETVFOB);
            }
        }
    }
}
