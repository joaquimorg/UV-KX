#pragma once

#include "apps.h"
#include "ui.h"

namespace Applications
{

    class Welcome : public Application {
    public:
        Welcome(System::SystemTask& systask, UI& ui, Keypad& keypad)
            : Application(systask, ui, keypad) {
        }

        void drawScreen(void);
        void init(void);
        void action(void);
        void timeout(void);

    private:
        
    };

} // namespace Applications

