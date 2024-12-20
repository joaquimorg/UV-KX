#pragma once

#include "ui.h"
#include "keypad.h"

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
        explicit Application(System::SystemTask& systask, UI &ui, Keypad &keypad) : systask{ systask }, ui{ui}, keypad{keypad} {};

        virtual void init(void) = 0;
        virtual void update(void) {};
        virtual void action(void) = 0;
        virtual void timeout(void) {};

    protected:
        System::SystemTask& systask;
        UI& ui;
        Keypad& keypad;
    };

} // namespace Applications
