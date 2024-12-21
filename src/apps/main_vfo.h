#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"

namespace Applications
{

    class MainVFO : public Application {
    public:
        MainVFO(System::SystemTask& systask, UI& ui, RadioNS::Radio& radio)
            : Application(systask, ui), radio{ radio } {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        RadioNS::Radio& radio;        
        uint8_t activeVFO = 0;

        void showRSSI(void);

    };

} // namespace Applications
