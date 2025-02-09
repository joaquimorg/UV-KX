#pragma once

#include "apps.h"
#include "settings.h"
#include "ui.h"

namespace Applications
{

    class Info : public Application {
    public:
        Info(System::SystemTask& systask, UI& ui, Settings& settings)
            : Application(systask, ui), settings{ settings } {};

        void drawScreen(void);
        void init(void);
        void update(void);
        void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState);
        void timeout(void);

    private:
        Settings& settings;
        bool showQuestion = false;
        bool isToInitialize = false;
        bool isReady = false;
        uint8_t initProgress = 0;
    };

} // namespace Applications

