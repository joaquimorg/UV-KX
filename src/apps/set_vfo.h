#pragma once

#include "apps.h"
#include "radio.h"
#include "ui.h"

namespace Applications
{

    class SetVFO : public Application {
    public:
        SetVFO(System::SystemTask& systask, UI& ui, RadioNS::Radio::VFOAB vfoab)
            : Application(systask, ui), menulist(ui), vfoab{ vfoab } {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist;

        RadioNS::Radio::VFOAB vfoab;

    };

} // namespace Applications