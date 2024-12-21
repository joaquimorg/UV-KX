#pragma once

#include "ui.h"
#include "keyboard.h"

namespace System {
    class SystemTask;
}

namespace Applications
{

    enum class Applications
    {
        None = 0,
        Welcome,
        MainVFO,
        Menu,
    };

    class Application {
    public:
        explicit Application(System::SystemTask& systask, UI &ui) : systask{ systask }, ui{ui} {};

        virtual void init(void) = 0;
        virtual void update(void) {};
        virtual void action(Keyboard::KeyCode keyCode, Keyboard::KeyState keyState) = 0;
        virtual void timeout(void) {};

    protected:
        System::SystemTask& systask;
        UI& ui;
    };

} // namespace Applications
