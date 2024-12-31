#pragma once

#include "apps.h"
#include "radio.h"
#include "ui.h"

namespace Applications
{

    class SetRadio : public Application {
    public:
        SetRadio(System::SystemTask& systask, UI& ui)
            : Application(systask, ui), menulist(ui) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void timeout(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        SelectionList menulist;

    };

} // namespace Applications
