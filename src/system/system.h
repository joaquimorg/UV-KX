#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "sys.h"
#include "spi_hal.h"
#include "uart_hal.h"
#include "keypad.h"
#include "backlight.h"
#include "battery.h"
#include "bk4819.h"
#include "radio.h"
#include "ui.h"

#include "apps.h"

#include "welcome.h"
#include "main_vfo.h"
#include "menu.h"

// ------------------------------------------------------------------------------------------------------------
namespace System
{

    class SystemTask {
    public:
        enum class SystemMSG {
            MSG_TIMEOUT,
            MSG_BKCLIGHT,
            MSG_RADIO_RX,
            MSG_APP_LOAD,
        };

        SystemTask() :
            spi0(SPI0),
            st7565(),
            ui(st7565),
            backlight(),
            uart(),
            keypad(),
            battery(),
            bk4819(),
            radio(*this, uart, bk4819),
            welcomeApp(*this, ui, keypad),
            mainVFOApp(*this, ui, keypad, radio),
            menuApp(*this, ui, keypad)
        {
            initSystem(); // Initialize system
        }

        void loadApplication(Applications::Applications app);
        void pushMessage(SystemMSG msg, uint32_t value);

        Battery getBattery() { return battery; }

        // Static methods (required by FreeRTOS)
        static void runStatusTask(void* pvParameters);
        static void appTimerCallback(TimerHandle_t xTimer);
        static void runTimerCallback(TimerHandle_t xTimer);

        void debug(const char* msg) { uart.sendLog(msg); };

    private:

        SPI spi0;
        ST7565 st7565;
        UI ui;
        Backlight backlight;
        UART uart;
        Keypad keypad;
        Battery battery;
        BK4819 bk4819;
        RadioNS::Radio radio;

        struct SystemMessages {
            SystemMSG message;
            uint32_t payload;
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

        Applications::Application* currentApplication;
        Applications::Applications currentApp = Applications::Applications::None;

        static constexpr uint16_t actionTimeout = 5; // Timeout for action 5 seconds
        uint16_t backlightTimeout = 15; // Timeout for backlight 15 seconds
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
