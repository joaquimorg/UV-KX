#pragma once

#include "apps.h"
#include "ui.h"

namespace Applications
{

    class Welcome : public Application {
    public:
        Welcome(System::SystemTask& systask, UI& ui)
            : Application(systask, ui) {
        }

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);
        void timeout(void);

    private:
        
    };

} // namespace Applications

