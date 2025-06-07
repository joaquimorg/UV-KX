// This file defines the SystemTask class, which is central to the application's architecture.
// It manages system-level operations, including task scheduling, message handling,
// and interaction between different modules like UI, radio, and settings.
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
#include "reset_init.h"
#include "main_vfo.h"
#include "menu.h"
#include "set_vfo.h"
#include "set_radio.h"

// ------------------------------------------------------------------------------------------------------------
namespace System
{

    /**
     * @brief Manages core system functionalities, including task coordination,
     *        message passing, and hardware interactions.
     * 
     * The SystemTask class is responsible for initializing and managing various
     * hardware components and software modules. It handles system-level events,
     * processes messages from different parts of the application, and orchestrates
     * the overall behavior of the system.
     */
    class SystemTask {
    public:
        /**
         * @brief Defines the types of messages that can be processed by the SystemTask.
         */
        enum class SystemMSG {
            MSG_TIMEOUT,        // Indicates a general timeout event.
            MSG_BKCLIGHT,       // Controls the backlight.
            MSG_KEYPRESSED,     // Indicates a key press event.
            MSG_LOW_BATTERY,    // Signals a low battery condition.
            MSG_PLAY_BEEP,      // Requests to play a beep sound.
            MSG_POWER_SAVE,     // Initiates power-saving mode.
            MSG_RADIO_IDLE,     // Indicates the radio is idle.
            MSG_RADIO_RX,       // Indicates the radio is in receive mode.
            MSG_RADIO_TX,       // Indicates the radio is in transmit mode.
            MSG_APP_LOAD,       // Requests to load a specific application.
        };

        /**
         * @brief Constructor for the SystemTask class.
         *        Initializes various hardware components and software modules.
         */
        SystemTask() :
            spi0(SPI0), // Initialize SPI0 interface
            settings(*this), // Initialize settings module
            st7565(), // Initialize ST7565 display controller
            uart(settings), // Initialize UART communication
            ui(st7565, uart), // Initialize user interface
            backlight(), // Initialize backlight control          
            keyboard(*this), // Initialize keyboard input
            battery(), // Initialize battery monitoring
            bk4819(), // Initialize BK4819 radio chip
            radio(*this, uart, bk4819, settings), // Initialize radio module
            welcomeApp(*this, ui), // Initialize welcome application
            resetInitApp(*this, ui, settings, true), // Initialize reset/init application
            resetEEPROMApp(*this, ui, settings, false), // Initialize EEPROM reset application
            mainVFOApp(*this, ui, radio), // Initialize main VFO application
            menuApp(*this, ui), // Initialize menu application
            setVFOAApp(*this, ui, Settings::VFOAB::VFOA, settings, radio), // Initialize Set VFO A application
            setVFOBApp(*this, ui, Settings::VFOAB::VFOB, settings, radio), // Initialize Set VFO B application
            setRadioApp(*this, ui, settings) // Initialize Set Radio application
        {
            initSystem(); // Perform initial system setup
        }

        /**
         * @brief Loads and switches to a specified application.
         * @param app The application to load.
         */
        void loadApplication(Applications::Applications app);

        /**
         * @brief Pushes a system message to the message queue.
         * @param msg The system message to push.
         * @param value An optional value associated with the message.
         */
        void pushMessage(SystemMSG msg, uint32_t value);

        /**
         * @brief Pushes a key press message to the message queue.
         * @param key The key code of the pressed key.
         * @param state The state of the key (pressed, released, etc.).
         */
        void pushMessageKey(Keyboard::KeyCode key, Keyboard::KeyState state);

        /**
         * @brief Checks if the F key was pressed.
         * @return True if the F key was pressed, false otherwise.
         */
        bool wasFKeyPressed() const { return keyboard.wasFKeyPressed(); };

        /**
         * @brief Sets the timeout for actions.
         * @param timeout The timeout duration in seconds.
         */
        void setActionTimeout( uint16_t timeout ) {
            actionTimeout = timeout; // Set the action timeout value
        }

        /**
         * @brief Gets the current battery status.
         * @return A Battery object representing the current battery state.
         */
        Battery getBattery() { return battery; }

        /**
         * @brief Plays a beep sound.
         * @param beep The type of beep to play.
         */
        void playBeep(Settings::BEEPType beep) { radio.playBeep(beep); }

        // Static methods (required by FreeRTOS for task creation)
        /**
         * @brief Static method to run the system status task.
         *        This is the entry point for the FreeRTOS task.
         * @param pvParameters Parameters passed to the task.
         */
        static void runStatusTask(void* pvParameters);

        /**
         * @brief Static callback function for the application timer.
         * @param xTimer Handle to the timer that expired.
         */
        static void appTimerCallback(TimerHandle_t xTimer);

        /**
         * @brief Static callback function for the run timer.
         * @param xTimer Handle to the timer that expired.
         */
        static void runTimerCallback(TimerHandle_t xTimer);

        /**
         * @brief Sends a debug message via UART.
         * @param msg The debug message to send.
         */
        void debug(const char* msg) { uart.sendLog(msg); };

        /**
         * @brief Configures the radio settings.
         */
        void setupRadio(void);

    private:

        // Hardware abstraction layer instances
        SPI spi0;           // SPI0 interface instance
        Settings settings;   // Settings module instance
        ST7565 st7565;       // ST7565 display controller instance
        UART uart;           // UART communication instance
        UI ui;               // User interface instance
        Backlight backlight;  // Backlight control instance      
        Keyboard keyboard;     // Keyboard input instance
        Battery battery;      // Battery monitoring instance
        BK4819 bk4819;       // BK4819 radio chip instance
        RadioNS::Radio radio; // Radio module instance

        /**
         * @brief Structure to hold system messages.
         */
        struct SystemMessages {
            SystemMSG message;          // The type of system message.
            uint32_t payload;           // Optional payload associated with the message.
            Keyboard::KeyCode key;      // Key code for key press messages.
            Keyboard::KeyState state;   // Key state for key press messages.
        };

        // Constants for message queue configuration
        static constexpr uint8_t queueLenght = 20; // Maximum number of messages in the queue
        static constexpr uint16_t itemSize = sizeof(SystemMessages); // Size of each message item

        // FreeRTOS message queue components
        QueueHandle_t systemMessageQueue; // Handle for the system message queue
        StaticQueue_t systemTasksQueue;   // Static storage for the message queue
        uint8_t systemQueueStorageArea[queueLenght * itemSize]; // Buffer for the static queue storage

        // FreeRTOS timer components
        TimerHandle_t appTimer;         // Handle for the application timer
        StaticTimer_t appTimerBuffer;   // Static storage for the application timer
        TimerHandle_t runTimer;         // Handle for the run timer
        StaticTimer_t runTimerBuffer;   // Static storage for the run timer

        // Application instances
        Applications::Welcome welcomeApp;             // Welcome screen application
        Applications::ResetInit resetInitApp;         // Reset/Initialize application
        Applications::ResetInit resetEEPROMApp;       // Reset EEPROM application
        Applications::MainVFO mainVFOApp;             // Main VFO screen application
        Applications::Menu menuApp;                   // Menu application
        Applications::SetVFO setVFOAApp;              // Set VFO A application
        Applications::SetVFO setVFOBApp;              // Set VFO B application
        Applications::SetRadio setRadioApp;           // Set Radio application

        // Pointers and state for managing the current application
        Applications::Application* currentApplication; // Pointer to the currently active application
        Applications::Applications currentApp = Applications::Applications::None; // Enum representing the current application

        // Timeout related variables
        uint16_t actionTimeout = 2;     // Timeout for general actions (in seconds)
        uint16_t powerSaveTimeout = 10; // Timeout for entering power save mode (in seconds)
        uint16_t backlightTimeout = 30; // Timeout for turning off the backlight (in seconds, default)
        uint16_t timeoutCount = 0;      // Counter for general timeouts
        uint16_t timeoutLightCount = 0; // Counter for backlight timeout
        uint16_t powerSaveCount = 0;    // Counter for power save timeout

        /**
         * @brief Initializes the system components and FreeRTOS objects.
         */
        void initSystem(void);

        /**
         * @brief Updates and displays the current screen/UI.
         */
        void showScreen(void);

        /**
         * @brief Implementation of the system status task.
         *        This function contains the main loop for processing messages and events.
         */
        void statusTaskImpl(void);

        /**
         * @brief Processes a system notification/message from the queue.
         * @param notification The system message to process.
         */
        void processSystemNotification(SystemMessages notification);

        /**
         * @brief Implementation for the application timer callback.
         */
        void appTimerImpl(void);

        /**
         * @brief Implementation for the run timer callback.
         */
        void runTimerImpl(void);

    };

}
