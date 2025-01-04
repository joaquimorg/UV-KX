#include "printf.h"
#include "apps.h"
#include "main_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void MainVFO::drawScreen(void) {

    RadioNS::Radio::VFOAB activeVFOA = radio.getCurrentVFO();
    RadioNS::Radio::VFOAB activeVFOB = activeVFOA == RadioNS::Radio::VFOAB::VFOA ? RadioNS::Radio::VFOAB::VFOB : RadioNS::Radio::VFOAB::VFOA;

    RadioNS::Radio::VFO vfoA = radio.getVFO(activeVFOA);
    RadioNS::Radio::VFO vfoB = radio.getVFO(activeVFOB);

    //char key = '-';

    ui.lcd()->clearBuffer();

    ui.lcd()->setColorIndex(BLACK);

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawString(TextAlign::LEFT, 2, 0, 6, false, false, false, vfoA.name);
    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->setColorIndex(BLACK);
    //ui.lcd()->drawStr(12, 14, "VFO");

    ui.setFont(Font::FONT_5_TR);
    ui.drawString(TextAlign::RIGHT, 0, 126, 6, false, false, false, ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfoA.power));
    ui.drawString(TextAlign::RIGHT, 0, 106, 6, false, false, false, ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfoA.bw));
    ui.drawString(TextAlign::RIGHT, 0, 84, 6, false, false, false, ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfoA.modulation));

    //ui.drawStringf(TextAlign::RIGHT, 0, 126, 26, true, false, false, "%s %s %s", "12.5K", "TX 131.8", "RX D023N");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 26, true, false, false, "%s %s %s", "12.5K", "", "");

    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, 9, 5, 25);

    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 14, "A");
    ui.drawStringf(TextAlign::LEFT, 2, 0, 20, true, true, false, "%s", activeVFOA == RadioNS::Radio::VFOAB::VFOA ? "A" : "B");

    if (radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOA == radio.getRXVFO()) {
        //ui.lcd()->drawStr(2, 20, "RX");
        ui.drawString(TextAlign::LEFT, 12, 0, 20, true, true, false, "RX");
    }
    ui.drawFrequencyBig((radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOA == radio.getRXVFO()), vfoA.rx.frequency, 113, 19);

    uint8_t vfoBY = 29;
    ui.lcd()->setColorIndex(BLACK);
    ui.lcd()->drawLine(5, vfoBY, 5, vfoBY + 3);
    ui.lcd()->drawLine(5, vfoBY + 12, 5, vfoBY + 14);
    ui.setFont(Font::FONT_8B_TR);
    //ui.lcd()->drawStr(2, 38, "B");
    ui.drawStringf(TextAlign::LEFT, 2, 0, vfoBY + 10, true, false, true, "%s", activeVFOB == RadioNS::Radio::VFOAB::VFOB ? "B" : "A");
    ui.drawFrequencySmall((radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOB == radio.getRXVFO()), vfoB.rx.frequency, 126, vfoBY + 8);
    //ui.drawFrequencyBig(143932500, 110, 40);

    ui.setFont(Font::FONT_8_TR);
    
    //ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 5, true, (radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOB == radio.getRXVFO()), false, vfoB.name);
    //ui.drawString(TextAlign::LEFT, 12, 0, vfoBY + 15, true, false, false, "VFO");
    
    ui.drawString(TextAlign::LEFT, 14, 0, vfoBY + 10, true, (radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOB == radio.getRXVFO()), false, vfoB.name);        

    ui.setFont(Font::FONT_5_TR);
    //ui.drawStringf(TextAlign::RIGHT, 0, 126, vfoBY + 15, true, false, false, "%s %s %s", ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfoB.modulation), ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfoB.bw), ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfoB.power));

    ui.drawString(TextAlign::RIGHT, 0, 126, vfoBY + 15, true, false, false, ui.getStrValue(RadioNS::Radio::powerStr, (uint8_t)vfoB.power));
    ui.drawString(TextAlign::RIGHT, 0, 106, vfoBY + 15, true, false, false, ui.getStrValue(RadioNS::Radio::bandwidthStr, (uint8_t)vfoB.bw));
    ui.drawString(TextAlign::RIGHT, 0, 84, vfoBY + 15, true, false, false, ui.getStrValue(RadioNS::Radio::modulationStr, (uint8_t)vfoB.modulation));

    /*if ((radio.getState() == RadioNS::Radio::RadioState::RX_ON && activeVFOB == radio.getRXVFO())) {
        ui.setFont(Font::FONT_8B_TR);
        ui.drawString(TextAlign::LEFT, 2, 0, 48, true, true, false, "RX");
    } else {

    }*/



    /*ui.lcd()->setColorIndex(BLACK);
    ui.setFont(Font::FONT_8_TR);
    ui.lcd()->drawStr(2, 50, "C");
    ui.drawFrequencySmall(143865000, 126, 50);*/

    //ui.setFont(Font::FONT_8B_TR);
    //ui.drawString(TextAlign::LEFT, 10, 0, 62, true, false, false, "M049 LISBOA1234");

    //ui.lcd()->setColorIndex(BLACK);

    //ui.drawStrf(10, 30, "BATT : %u.%02uV - %3i%%", systask.getBattery().getBatteryVoltageAverage() / 100, systask.getBattery().getBatteryVoltageAverage() % 100, systask.getBattery().getBatteryPercentage());

    /*ui.setFont(Font::FONT_5_TR);
    ui.drawStrf(5, 63, "%i", getElapsedMilliseconds());*/

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
        ui.drawStringf(TextAlign::RIGHT, 0, 128, 58, true, false, false, "A/B");
        //ui.drawString(TextAlign::RIGHT, 0, 128, 58, true, false, false, activeVFOA == RadioNS::Radio::VFOAB::VFOA ? "A" : "B");
    }

    if (showPopup) {
        popupList.drawPopup();
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

        ui.drawRSSI(sValue, plusDB, 10, 52);

        ui.setFont(Font::FONT_5_TR);
        if (sValue > 0) {
            if (sValue == 10) {
                ui.drawString(TextAlign::LEFT, 0, 0, 59, true, false, false, "S9");
                ui.drawStringf(TextAlign::CENTER, 48, 75, 57, true, false, false, "+%idB", plusDB);
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
            popupList.set((uint8_t)vfo.bw, 3, 0, RadioNS::Radio::bandwidthStr);
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
