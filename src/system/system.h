#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "sys.h"
#include "spi_hal.h"
#include "uart_hal.h"
#include "i2c_hal.h"
#include "keyboard.h"
#include "backlight.h"
#include "battery.h"
#include "bk4819.h"
#include "radio.h"
#include "ui.h"

#include "settings.h"
#include "apps.h"

#include "welcome.h"
#include "main_vfo.h"
#include "menu.h"
#include "set_vfo.h"
#include "set_radio.h"

// ------------------------------------------------------------------------------------------------------------
namespace System
{

    class SystemTask {
    public:
        enum class SystemMSG {
            MSG_TIMEOUT,
            MSG_BKCLIGHT,
            MSG_KEYPRESSED,
            MSG_PLAY_BEEP,
            MSG_RADIO_RX,
            MSG_RADIO_TX,
            MSG_APP_LOAD,
        };

        SystemTask() :
            spi0(SPI0),
            settings(*this),
            st7565(),
            uart(),
            ui(st7565, uart),
            backlight(),            
            keyboard(*this),
            battery(),
            bk4819(),
            radio(*this, uart, bk4819, settings),
            welcomeApp(*this, ui),
            mainVFOApp(*this, ui, radio),
            menuApp(*this, ui),
            setVFOAApp(*this, ui, Settings::VFOAB::VFOA, settings, radio),
            setVFOBApp(*this, ui, Settings::VFOAB::VFOB, settings, radio),
            setRadioApp(*this, ui)
        {
            initSystem(); // Initialize system
        }

        void loadApplication(Applications::Applications app);
        void pushMessage(SystemMSG msg, uint32_t value);
        void pushMessageKey(Keyboard::KeyCode key, Keyboard::KeyState state);
        bool wasFKeyPressed() const { return keyboard.wasFKeyPressed(); };
        void setActionTimeout( uint16_t timeout ) {
            actionTimeout = timeout;
        }

        Battery getBattery() { return battery; }

        void playBeep(Settings::BEEPType beep) { radio.playBeep(beep); }

        // Static methods (required by FreeRTOS)
        static void runStatusTask(void* pvParameters);
        static void appTimerCallback(TimerHandle_t xTimer);
        static void runTimerCallback(TimerHandle_t xTimer);

        void debug(const char* msg) { uart.sendLog(msg); };

    private:

        SPI spi0;
        Settings settings;
        ST7565 st7565;
        UART uart;
        UI ui;
        Backlight backlight;        
        Keyboard keyboard;
        Battery battery;
        BK4819 bk4819;
        RadioNS::Radio radio;

        struct SystemMessages {
            SystemMSG message;
            uint32_t payload;
            Keyboard::KeyCode key;
            Keyboard::KeyState state;
        };

        static constexpr uint8_t queueLenght = 20;
        static constexpr uint16_t itemSize = sizeof(SystemMessages);

        QueueHandle_t systemMessageQueue; // Message queue handle
        StaticQueue_t systemTasksQueue; // Static queue storage area
        uint8_t systemQueueStorageArea[queueLenght * itemSize]; // Static queue storage area

        TimerHandle_t appTimer;
        StaticTimer_t appTimerBuffer;
        TimerHandle_t runTimer;
        StaticTimer_t runTimerBuffer;

        Applications::Welcome welcomeApp;
        Applications::MainVFO mainVFOApp;
        Applications::Menu menuApp;
        Applications::SetVFO setVFOAApp;
        Applications::SetVFO setVFOBApp;
        Applications::SetRadio setRadioApp;

        Applications::Application* currentApplication;
        Applications::Applications currentApp = Applications::Applications::None;

        uint16_t actionTimeout = 2; // Timeout for action 2 seconds
        uint16_t backlightTimeout = 30; // Timeout for backlight 30 seconds default
        uint16_t timeoutCount = 0;
        uint16_t timeoutLightCount = 0;

        void initSystem(void);
        void showScreen(void);
        void statusTaskImpl(void);
        void processSystemNotification(SystemMessages notification);

        void appTimerImpl(void);
        void runTimerImpl(void);

    };

}
