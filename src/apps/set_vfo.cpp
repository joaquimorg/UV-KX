#include "printf.h"
#include "apps.h"
#include "set_vfo.h"
#include "system.h"
#include "ui.h"
#include "u8g2.h"

using namespace Applications;

void SetVFO::drawScreen(void) {

    ui.lcd()->clearBuffer();

    ui.setBlackColor();

    ui.lcd()->drawBox(0, 0, 128, 7);

    ui.setFont(Font::FONT_8B_TR);
    ui.drawStringf(TextAlign::LEFT, 2, 0, 6, false, false, false, "VFO %s", vfoab == RadioNS::Radio::VFOAB::VFOA ? "A" : "B");
    ui.drawStringf(TextAlign::RIGHT, 0, 126, 6, false, false, false, "%02u / %02u", menulist.getListPos() + 1, menulist.getTotal());

    ui.setBlackColor();

    menulist.draw(15);

    if (optionSelected != 0) {
        optionlist.drawPopup(true);
    }

    ui.updateDisplay();
}

void SetVFO::init(void) {
    menulist.set(0, 6, 80, "SQUELCH\nSTEP\nMODE\nBANDWIDTH\nTX POWER\nSHIFT\nOFFSET\nRX CTCS\nTX CTCS\nRX DTCS\nTX DTCS\nTX STE\nRX STE\nCOMPANDER\nPTT ID\nAFC\nRX ACG");
}

void SetVFO::update(void) {
    drawScreen();
}

void SetVFO::timeout(void) {
    if (optionSelected == 0) {
        systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
    } else {
        optionSelected = 0;
    }
};

void SetVFO::action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) {

    if (keyState == Keyboard::KeyState::KEY_PRESSED || keyState == Keyboard::KeyState::KEY_LONG_PRESSED_CONT) {
        if (optionSelected == 0) {

            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                menulist.prev();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                menulist.next();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                systask.pushMessage(System::SystemTask::SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::MainVFO);
            }

            if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                optionSelected = menulist.getListPos() + 1;
                switch (optionSelected) {
                case 1: // SQUELCH
                    optionlist.set(0, 5, 0, "OFF\n1\n2\n3\n4\n5\n6\n7\n8\n9");                    
                    break;
                case 2: // STEP
                    optionlist.set(0, 5, 0, "0.5\n1.0\n2.5\n5.0\n6.25\n10.0\n12.5\n15.0\n20.0\n25.0\n30.0\n50.0\n100.0\n500.0", "KHz");                    
                    break;
                case 3: // MODE
                    optionlist.set(0, 5, 0, RadioNS::Radio::modulationStr);                    
                    break;
                case 4: // BANDWIDTH
                    optionlist.set(0, 5, 0, RadioNS::Radio::bandwidthStr);                    
                    break;
                case 5: // TX POWER
                    optionlist.set(0, 5, 0, RadioNS::Radio::powerStr);                    
                    break;
                case 6: // SHIFT
                    optionlist.set(0, 3, 0, "OFF\n+\n-");                    
                    break;
                case 7: // OFFSET
                    //optionlist.set(0, 5, 0, "0.0\n1.6\n2.5\n3.0\n5.0\n6.0\n7.6\n8.0\n9.0\n10.0\n12.5\n15.0\n20.0\n25.0\n30.0\n50.0\n100.0\n500.0", "KHz");                    
                    break;
                case 8: // RX CTCS
                    optionlist.set(0, 5, 0, "OFF\n67.0\n69.3\n71.9\n74.4\n77.0\n79.7\n82.5\n85.4\n88.5\n91.5\n94.8\n97.4\n100.0\n103.5\n107.2\n110.9\n114.8\n118.8\n123.0\n127.3\n131.8\n136.5\n141.3\n146.2\n151.4\n156.7\n162.2\n167.9\n173.8\n179.9\n186.2\n192.8\n203.5\n210.7\n218.1\n225.7\n233.6\n241.8\n250.3\n254.1", "Hz");
                    break;
                default:
                    break;
                }
                optionlist.setPopupTitle(menulist.getStringLine());
            }
        }
        else {
            if (keyCode == Keyboard::KeyCode::KEY_UP) {
                optionlist.prev();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_DOWN) {
                optionlist.next();
            }
            else if (keyCode == Keyboard::KeyCode::KEY_EXIT) {
                optionSelected = 0;
            } else if (keyCode == Keyboard::KeyCode::KEY_MENU) {
                optionSelected = 0;
            }
        }
    }
}
