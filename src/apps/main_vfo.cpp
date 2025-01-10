#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void MainVFO::drawScreen(void) {

    RadioNS::Radio::VFOAB activeVFO1 = radio.getCurrentVFO();
    RadioNS::Radio::VFOAB activeVFO2 = activeVFO1 == RadioNS::Radio::VFOAB::VFOA ? RadioNS::Radio::VFOAB::VFOB : RadioNS::Radio::VFOAB::VFOA;

    RadioNS::Radio::VFO vfo1 = radio.getVFO(activeVFO1);
    RadioNS::Radio::VFO vfo2 = radio.getVFO(activeVFO2);
    bool rxVFO1 = (radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFO1 == radio.getRXVFO());
    bool rxVFO2 = (radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFO2 == radio.getRXVFO());

    ui.clearDisplay();

    ui.lcd()->setColorIndex(BLACK);

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 1, 0, 6, false, false, false, vfo1.name);

    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    ui.drawString(TextAlign::LEFT, 12, 0, 14, true, false, false, "VFO");

    ui.setFont(Font::FONT_5_TR);
    const char* powerA = ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfo1.power);
    const char* bandwidthA = ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfo1.bw);
    const char* modulationA = ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfo1.modulation);
    
    ui.drawStringf(TextAlign::RIGHT, 0, 127, 6, false, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationA), modulationA, ui.stringLengthNL(bandwidthA), bandwidthA, ui.stringLengthNL(powerA), powerA);
    
    //ui.drawStringf(TextAlign::RIGHT, 0, 126, 26, true, false, false, "%s %s %s", "12.5K", "TX 131.8", "RX D023N");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 26, true, false, false, "%s %s %s", "", "", "");

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, 9, 5, 25);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 20, true, true, false, "%s", activeVFO1 == RadioNS::Radio::VFOAB::VFOA ? "A" : "B");

    if (rxVFO1) {
        ui.drawString(TextAlign::LEFT, 12, 0, 20, true, true, false, "RX");
    }
                                     
    ui.drawFrequencyBig(rxVFO1, vfo1.rx.frequency, 115, 19);

    uint8_t vfoBY = 29;
    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, vfoBY, 5, vfoBY + 3);
    ui.lcd()->drawLine(5, vfoBY + 12, 5, vfoBY + 14);
    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, vfoBY + 10, true, false, true, "%s", activeVFO2 == RadioNS::Radio::VFOAB::VFOB ? "B" : "A");
    ui.drawFrequencySmall(rxVFO2, vfo2.rx.frequency, 128, vfoBY + 8);

    ui.lcd()->setColorIndex(BLACK);
    ui.setFont(Font::FONT_8_TR);    
    ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 5, true, rxVFO2, false, vfo2.name);

    if (rxVFO2) {
        ui.setFont(Font::FONT_8B_TR);
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, true, false, "RX");
    } else {    
        ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, "VFO");   
    }

    ui.setFont(Font::FONT_5_TR);
    
    const char* powerB = ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfo2.power);
    const char* bandwidthB = ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfo2.bw);
    const char* modulationB = ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfo2.modulation);
    
    ui.drawStringf(TextAlign::RIGHT, 0, 128, vfoBY + 15, true, false, false, "%.*s %.*sK %.*s", ui.stringLengthNL(modulationB), modulationB, ui.stringLengthNL(bandwidthB), bandwidthB, ui.stringLengthNL(powerB), powerB);

    showRSSI();

    if (systask.getBattery().isCharging()) {
        ui.draw_ic8_charging(100, 59, BLACK);
    }
    else {
        ui.drawBattery(systask.getBattery().getBatteryPercentage(), 96, 59);
    }

    ui.setFont(Font::FONT_5_TR);
    ui.drawStringf(TextAlign::RIGHT, 0, 128, 64, true, false, false, "%i%%", systask.getBattery().getBatteryPercentage());

    if (systask.wasFKeyPressed()) {
        ui.drawStringf(TextAlign::RIGHT, 0, 120, 57, true, true, false, "F");
    }
    else {
        if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
            ui.drawString(TextAlign::RIGHT, 0, 128, 58, true, false, false, activeVFO1 == radio.getRXVFO() ? "A" : "B");
        } else {
            ui.drawStringf(TextAlign::RIGHT, 0, 128, 58, true, false, false, "A/B");
        }
    }

    if (showPopup) {
        popupList.drawPopup(ui);
    }

    ui.updateDisplay();
}

void MainVFO::showRSSI(void) {

    uint8_t sValue = 0;
    int16_t plusDB = 0;
    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON) {
        int16_t rssi_dBm = radio.getRSSIdBm(); // Get the RSSI value in dBm
        sValue = radio.convertRSSIToSLevel(rssi_dBm); // Convert RSSI to S-level
        if (sValue == 10) {
            plusDB = radio.convertRSSIToPlusDB(rssi_dBm); // Convert to +dB value if greater than S9
        }

        ui.drawRSSI(sValue, /*plusDB, */12, 52);

        ui.setFont(Font::FONT_8_TR);
        if (sValue > 0) {
            if (sValue == 10) {
                ui.drawString(TextAlign::LEFT, 0, 0, 59, true, false, false, "S9");
                ui.drawStringf(TextAlign::CENTER, 48, 70, 59, true, false, false, "+%idB", plusDB);
            }
            else {
                ui.drawStringf(TextAlign::LEFT, 0, 0, 59, true, false, false, "S%i", sValue);
            }
        }

    }
}

void MainVFO::init(void) {
}

void MainVFO::update(void) {
    drawScreen();
}

void MainVFO::timeout(void) {
    if (showPopup) {
        showPopup = false;
    }
}

void MainVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    RadioNS::Radio::VFO vfo = radio.getActiveVFO();

    if (keyState == Keyboard::KeyState::KEY_RELEASED) {

        if (showPopup) {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                popupList.prev();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                popupList.next();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                showPopup = false;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                showPopup = false;
            }
            else if (keyCode == Keyboard::KeyCode::KEY_4 || keyCode == Keyboard::KeyCode::KEY_5 || keyCode == Keyboard::KeyCode::KEY_6) {
                popupList.next();
            }
        }
        else {

            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                uint32_t newFrequency = vfo.rx.frequency + 1250;
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x07FFFFFF);

                radio.setVFO(radio.getCurrentVFO(), vfo.rx.frequency, vfo.rx.frequency, vfo.channel, vfo.modulation);
                radio.setupToVFO(radio.getCurrentVFO());
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                uint32_t newFrequency = vfo.rx.frequency - 1250;
                vfo.rx.frequency = (uint32_t)(newFrequency & 0x7FFFFFF);

                radio.setVFO(radio.getCurrentVFO(), vfo.rx.frequency, vfo.rx.frequency, vfo.channel, vfo.modulation);
                radio.setupToVFO(radio.getCurrentVFO());
            }
            else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Menu);
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {

            }
        }
    }

    if (keyState == Keyboard::KeyState::KEY_LONG_PRESSED || keyState == Keyboard::KeyState::KEY_PRESSED_WITH_F) {

        if (keyCode == Keyboard::KeyCode::KEY_2) {            
            radio.changeActiveVFO();
        }

        if (keyCode == Keyboard::KeyCode::KEY_4) {
            popupList.set((uint8_t)vfo.bw, 3, 0, RadioNS::Radio::bandwidthStr, "K");
            popupList.setPopupTitle("BANDWIDTH");
            showPopup = true;
        }

        if (keyCode == Keyboard::KeyCode::KEY_5) {
            popupList.set((uint8_t)vfo.modulation, 3, 0, RadioNS::Radio::modulationStr);
            popupList.setPopupTitle("MODULATION");
            showPopup = true;
        }

        if (keyCode == Keyboard::KeyCode::KEY_6) {
            popupList.set((uint8_t)vfo.power, 3, 0, RadioNS::Radio::powerStr);
            popupList.setPopupTitle("TX POWER");
            showPopup = true;
        }
    }
}
