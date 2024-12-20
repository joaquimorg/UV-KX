#pragma once

#include "apps.h"
#include "ui.h"

namespace Applications
{

    class Menu : public Application {
    public:
        Menu(System::SystemTask& systask, UI& ui, Keypad& keypad)
            : Application(systask, ui, keypad) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(void);

    private:
        char key = '-';

    };

} // namespace Applications
