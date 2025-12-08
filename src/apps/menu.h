#pragma once

#include "apps.h"
#include "ui.h"
#include "radio.h"

namespace Applications
{

    class Menu : public Application {
    public:
        Menu(System::SystemTask& systask, UI& ui, RadioNS::Radio& radio)
            : Application(systask, ui), radio{ radio }, menulist(ui) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        RadioNS::Radio& radio;
        SelectionList menulist;
        bool firstVFOIsA = true;
        char menuText[96]{};

    };

} // namespace Applications
