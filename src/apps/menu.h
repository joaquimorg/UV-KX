#pragma once

#include "apps.h"
#include "ui.h"

namespace Applications
{

    class Menu : public Application {
    public:
        Menu(System::SystemTask& systask, UI& ui)
            : Application(systask, ui) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);

    private:
        char key = '-';

    };

} // namespace Applications
