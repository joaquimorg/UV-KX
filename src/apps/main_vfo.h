#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"

namespace Applications
{

    class MainVFO : public Application {
    public:
        MainVFO(System::SystemTask& systask, UI& ui, Keypad& keypad, RadioNS::Radio& radio)
            : Application(systask, ui, keypad), radio{ radio } {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(void);

    private:
        RadioNS::Radio& radio;        
        uint8_t activeVFO = 0;
        char key = '-';

        void showRSSI(void);

    };

} // namespace Applications
