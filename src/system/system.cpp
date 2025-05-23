// This file implements the SystemTask class, which is central to the application's architecture.
// It handles system initialization, message processing, task management, and application lifecycle.
#include "system.h"
#include "sys.h"

using namespace System;

// --- Static methods required by FreeRTOS ---

/**
 * @brief Entry point for the system status FreeRTOS task.
 *        This function calls the statusTaskImpl method of the SystemTask instance.
 * @param pvParameters Pointer to the SystemTask instance.
 */
void SystemTask::runStatusTask(void* pvParameters) {
    // Cast the void pointer to a SystemTask pointer
    SystemTask* systemTask = static_cast<SystemTask*>(pvParameters);
    if (systemTask) {
        // Call the implementation of the status task
        systemTask->statusTaskImpl();
    }
}

/**
 * @brief Callback function for the application timer.
 *        This function is called when the application timer expires.
 * @param xTimer Handle to the timer that expired.
 */
void SystemTask::appTimerCallback(TimerHandle_t xTimer) {
    // Retrieve the SystemTask instance from the timer ID
    SystemTask* systemTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
    if (systemTask) {
        // Call the application timer implementation
        systemTask->appTimerImpl();
    }
}

/**
 * @brief Callback function for the run timer.
 *        This function is called when the run timer expires.
 * @param xTimer Handle to the timer that expired.
 */
void SystemTask::runTimerCallback(TimerHandle_t xTimer) {
    // Retrieve the SystemTask instance from the timer ID
    SystemTask* systemTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
    if (systemTask) {
        // Call the run timer implementation
        systemTask->runTimerImpl();
    }
    // Restart the timer
    xTimerStart(xTimer, 0);
}

// --- SystemTask member functions ---

/**
 * @brief Initializes the system components.
 *        This includes creating the message queue, initializing the display, and printing firmware info.
 */
void SystemTask::initSystem(void) {
    // Create the static message queue for system tasks
    systemMessageQueue = xQueueCreateStatic(queueLenght, itemSize, systemQueueStorageArea, &systemTasksQueue);

    /* Error handling for queue creation (currently commented out)
    if (systemMessageQueue == NULL) {
        // need to haldle error
        // uart.sendLog("systemMessageQueue is NULL");
        return;
    }*/
        
    delayMs(10); // Brief delay

    st7565.begin(); // Initialize the ST7565 display controller
    // Print firmware information to UART
    uart.print("UV-Kx Open Firmware - " AUTHOR_STRING " - " VERSION_STRING "\n");
}

/**
 * @brief Configures the radio hardware and loads settings.
 *        If radio settings are invalid or not found, default settings are applied.
 */
void SystemTask::setupRadio(void) {
    // If the radio is already initialized, return
    if (radio.isRadioReady()) return;

    // Setup BK4819 radio chip registers
    bk4819.setupRegisters();

    delayMs(10); // Brief delay
    // Get radio settings from EEPROM
    settings.getRadioSettings();
    uart.print("[DEBUG] EEPROM Version : %x\r\n", settings.getSettingsVersion());
    delayMs(10); // Brief delay

    // Validate EEPROM settings version
    if (!settings.validateSettingsVersion()) {
        // If settings are invalid, apply default radio settings
        settings.setRadioSettingsDefault();

        // Set default VFO A and VFO B settings
        radio.setVFO(Settings::VFOAB::VFOA, 44616875, 44616875, 0, ModType::MOD_FM);
        radio.setVFO(Settings::VFOAB::VFOB, 43932500, 43932500, 0, ModType::MOD_FM);

        // Store the default VFO settings in the settings structure
        settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOA] = radio.getVFO(Settings::VFOAB::VFOA);
        settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOB] = radio.getVFO(Settings::VFOAB::VFOB);
        //settings.setRadioSettings(); // Potentially save settings back to EEPROM (commented out)
    } else {
        // Load settings from EEPROM if valid
        // TODO: need to validate if load VFO or Memory
        radio.setVFO(Settings::VFOAB::VFOA, settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOA]);
        radio.setVFO(Settings::VFOAB::VFOB, settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOB]);
    }
    
    // Setup the radio to the selected VFO
    radio.setupToVFO(settings.radioSettings.vfoSelected);
    
    // Mark the radio as ready
    radio.setRadioReady(true);
}

/**
 * @brief Pushes a system message to the system message queue from an ISR.
 * @param msg The system message to send.
 * @param value An optional payload value for the message.
 */
void SystemTask::pushMessage(SystemMSG msg, uint32_t value) {
    // Create a system message structure
    SystemMessages appMSG = { msg, value, (Keyboard::KeyCode)0, (Keyboard::KeyState)0 };
    BaseType_t xHigherPriorityTaskWoken = pdTRUE; // Flag to check if a higher priority task was woken
    // Send the message to the queue from an ISR context
    xQueueSendFromISR(systemMessageQueue, (void*)&appMSG, &xHigherPriorityTaskWoken);
}

/**
 * @brief Pushes a key press message to the system message queue from an ISR.
 * @param key The key code of the pressed key.
 * @param state The state of the key (pressed, released, etc.).
 */
void SystemTask::pushMessageKey(Keyboard::KeyCode key, Keyboard::KeyState state) {
    // Create a key press message structure
    SystemMessages appMSG = { SystemMSG::MSG_KEYPRESSED, 0, key, state };
    BaseType_t xHigherPriorityTaskWoken = pdTRUE; // Flag to check if a higher priority task was woken
    // Send the message to the queue from an ISR context
    xQueueSendFromISR(systemMessageQueue, (void*)&appMSG, &xHigherPriorityTaskWoken);
}

/**
 * @brief Implementation of the main system status task.
 *        This task initializes system components, timers, and then enters an infinite loop
 *        to process messages, handle UART commands, and manage radio operations.
 */
void SystemTask::statusTaskImpl() {
    SystemMessages notification; // Variable to store received notifications

    //uart.sendLog("System task started"); // Debug log

    battery.getReadings(); // Initial battery reading

    // Create static timers for application and run-time events
    appTimer = xTimerCreateStatic("app", pdMS_TO_TICKS(100), pdTRUE, this, SystemTask::appTimerCallback, &appTimerBuffer);
    runTimer = xTimerCreateStatic("run", pdMS_TO_TICKS(500), pdFALSE, this, SystemTask::runTimerCallback, &runTimerBuffer);    

    backlight.setBacklight(Backlight::backLightState::ON); // Turn on the backlight initially

    keyboard.init(); // Initialize the keyboard

    playBeep(Settings::BEEPType::BEEP_880HZ_200MS); // Play a startup beep

    /* Commented out EEPROM validation and initial app load logic
    // Validate the EEPROM content and initialize if necessary
    if (!settings.validateSettingsVersion()) {
        pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::RESETINIT);
    }
    else {
        // Load the Welcome application
        pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::Welcome);
    } 
    */
    // Load the Welcome application by default
    pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::Welcome);

    // Start the timers
    xTimerStart(appTimer, 0);
    xTimerStart(runTimer, 0);
    
    //uart.print("lenght : %i\n", settings.stringLength(ui.generateCTDCList(Settings::DCSOptions, 104, false))); // Debug print
    
    // Main task loop
    for (;;) {
        // Wait for messages from the queue with a timeout
        if (xQueueReceive(systemMessageQueue, &notification, pdMS_TO_TICKS(5)) == pdTRUE) {
            // Process the received system notification
            processSystemNotification(notification);
        }

        // Critical section to handle UART commands
        taskENTER_CRITICAL();
        if (uart.isCommandAvailable()) {
            uart.handleCommand(); // Process incoming UART commands
        }
        taskEXIT_CRITICAL();

        radio.checkRadioInterrupts(); // Check for radio interrupts (e.g., RX/TX completion)
        radio.runDualWatch();         // Manage dual watch functionality
        //vTaskDelay(pdMS_TO_TICKS(1)); // Optional small delay
    }
}

/**
 * @brief Processes system notifications received from the message queue.
 *        This function acts as a state machine based on the message type.
 * @param notification The system message to process.
 */
void SystemTask::processSystemNotification(SystemMessages notification) {
    // Handle different system notifications based on message type
    switch (notification.message) {
    case SystemMSG::MSG_TIMEOUT:
        timeoutCount = 0; // Reset timeout counter
        ui.timeOut();     // Notify UI about the timeout
        //uart.sendLog("MSG_TIMEOUT\n"); // Debug log
        battery.getReadings(); // Update battery readings
        if (battery.isLowBattery()) {
            // If battery is low, send a low battery message
            pushMessage(SystemMSG::MSG_LOW_BATTERY, 0);
        }
        if (currentApp != Applications::Applications::None) {
            // Notify the current application about the timeout
            currentApplication->timeout();
        }
        if (keyboard.wasFKeyPressed()) {
            // Clear the F key pressed state
            keyboard.clearFKeyPressed();
        }        
        break;
    case SystemMSG::MSG_POWER_SAVE:
        powerSaveCount = 0; // Reset power save counter
        if (radio.getState() == Settings::RadioState::IDLE) {
            // If radio is idle, enter power save mode
            radio.setPowerSaveMode();
        }        
        break;
    case SystemMSG::MSG_BKCLIGHT:
        //uart.sendLog("MSG_BKCLIGHT\n"); // Debug log
        timeoutLightCount = 0; // Reset backlight timeout counter
        // Set the backlight state based on the payload
        backlight.setBacklight((Backlight::backLightState)notification.payload);
        break;
    case SystemMSG::MSG_PLAY_BEEP:
        // Play a beep sound based on the payload
        playBeep((Settings::BEEPType)notification.payload);
        break;
    case SystemMSG::MSG_RADIO_IDLE:
        powerSaveCount = 0; // Reset power save counter
        //uart.sendLog("MSG_RADIO_IDLE\n"); // Debug log
        break;
    case SystemMSG::MSG_RADIO_RX:
        //uart.sendLog("MSG_RADIO_RX\n"); // Debug log
        radio.setNormalPowerMode(); // Exit power save mode
        powerSaveCount = 0;         // Reset power save counter
        // Turn on the backlight when radio receives
        pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        break;
    case SystemMSG::MSG_LOW_BATTERY:        
        ui.setInfoMessage(UI::InfoMessageType::LOW_BATTERY); // Display low battery message on UI
        // Play a triple beep to indicate low battery
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP);
        break;
    case SystemMSG::MSG_RADIO_TX:
        //uart.sendLog("MSG_RADIO_TX\n"); // Debug log
        //pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON); // Optionally turn on backlight during TX
        ui.setInfoMessage(UI::InfoMessageType::TX_DISABLED); // Display TX disabled message (can be updated for actual TX indication)
        break;
    case SystemMSG::MSG_KEYPRESSED: {
        //uart.sendLog("MSG_KEYPRESSED"); // Debug log
        //uart.print("Key: %d\n", notification.key); // Debug print key code
        //uart.print("State: %d\n", notification.state); // Debug print key state
        Keyboard::KeyCode key = notification.key;
        Keyboard::KeyState state = notification.state;        

        radio.setNormalPowerMode(); // Exit power save mode on key press
        powerSaveCount = 0;         // Reset power save counter
        if (currentApp != Applications::Applications::None) {
            // Pass the key event to the current application
            currentApplication->action(key, state);
        }

        // Handle actions for key pressed or long pressed states
        if (state == Keyboard::KeyState::KEY_PRESSED || state == Keyboard::KeyState::KEY_LONG_PRESSED) {
            timeoutCount = 0;      // Reset general timeout
            timeoutLightCount = 0; // Reset backlight timeout
            // Turn on the backlight
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
            if (key != Keyboard::KeyCode::KEY_PTT) {
                // Play a beep for non-PTT key presses
                playBeep(Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL);
                //pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL);
            }
            else if ( radio.isRadioReady() ){
                // If PTT is pressed and radio is ready, send TX message
                pushMessage(SystemMSG::MSG_RADIO_TX, 0);
            }
        }
        
        break;
    }
    case SystemMSG::MSG_APP_LOAD:
        // Load a new application based on the payload
        loadApplication((Applications::Applications)notification.payload);
        break;

    default:
        // Unknown message type, do nothing
        break;
    }
}

/**
 * @brief Implementation for the run timer (0.5-second interval).
 *        Manages timeouts for actions, power saving, and backlight.
 */
void SystemTask::runTimerImpl(void) {
    // This timer typically runs every 0.5 seconds

    // Check for general action timeout
    if (timeoutCount > (actionTimeout * 2)) { // actionTimeout is in seconds, timer ticks every 0.5s
        //timeoutCount = 0; // Resetting here might be too early if MSG_TIMEOUT handles it
        pushMessage(SystemMSG::MSG_TIMEOUT, 0);
    }
    else {
        timeoutCount++;
    }

    // Check for power save timeout if not already in power save mode
    if (!radio.isPowerSaveMode()) {
        if (powerSaveCount > (powerSaveTimeout * 2)) { // powerSaveTimeout is in seconds       
            pushMessage(SystemMSG::MSG_POWER_SAVE, 0);
        }
        else {
            powerSaveCount++;
        }
    }

    // Check for backlight timeout if backlight is currently on
    if (backlight.getBacklightState() == Backlight::backLightState::ON) {
        if (timeoutLightCount > (backlightTimeout * 2)) { // backlightTimeout is in seconds
            //timeoutLightCount = 0; // Resetting here might be too early
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::OFF);
        }
        else {
            timeoutLightCount++;
        }
    }
}

/**
 * @brief Implementation for the application timer (typically 100ms interval).
 *        Calls the update method of the current application.
 */
void SystemTask::appTimerImpl(void) {
    // This timer typically runs at a faster rate (e.g., 100ms) for UI updates

    // If there is a current application running, call its update method
    if (currentApp != Applications::Applications::None) {
        currentApplication->update();
    }
}

/**
 * @brief Loads and initializes a new application.
 *        Stops the current application's timer, sets up the new application,
 *        and starts its timer.
 * @param app The application to load.
 */
void SystemTask::loadApplication(Applications::Applications app) {
    // Do nothing if the requested application is None
    if (app == Applications::Applications::None) return;
    //taskENTER_CRITICAL(); // Optional critical section

    currentApp = Applications::Applications::None; // Mark current app as none temporarily
    timeoutCount = 0; // Reset timeout counter
    xTimerStop(appTimer, 0); // Stop the application timer for the old application
    setActionTimeout(2); // Set default action timeout

    // Switch to the new application
    switch (app) {
    case Applications::Applications::Welcome:
        currentApplication = &welcomeApp;
        break;
    case Applications::Applications::RESETINIT:
        currentApplication = &resetInitApp;
        setActionTimeout(1); // Shorter timeout for init screen
        break;
    case Applications::Applications::RESETEEPROM:
        currentApplication = &resetEEPROMApp;
        setActionTimeout(1); // Shorter timeout for init screen
        break;
    case Applications::Applications::MainVFO:
        setupRadio(); // Ensure radio is configured for VFO mode
        currentApplication = &mainVFOApp;
        break;
    case Applications::Applications::Menu:
        currentApplication = &menuApp;
        break;
    case Applications::Applications::SETVFOA:
        currentApplication = &setVFOAApp;
        setActionTimeout(5); // Longer timeout for VFO settings
        break;
    case Applications::Applications::SETVFOB:
        currentApplication = &setVFOBApp;
        setActionTimeout(5); // Longer timeout for VFO settings
        break;
    case Applications::Applications::SETRADIO:
        currentApplication = &setRadioApp;
        setActionTimeout(5); // Longer timeout for radio settings
        break;
    case Applications::Applications::MESSENGER:
        // TODO: Implement Messenger application
        //currentApplication = &messengerApp;
        currentApplication = &mainVFOApp; // Fallback to MainVFO for now
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
        break;
    case Applications::Applications::SCANNER:
        // TODO: Implement Scanner application
        //currentApplication = &scannerApp;
        currentApplication = &mainVFOApp; // Fallback to MainVFO for now
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
        break;
    case Applications::Applications::ABOUT:
        currentApplication = &welcomeApp; // Show welcome/about screen
        break;
    default:
        // Unknown application, do nothing or load a default
        return; // Or load a default like MainVFO
    }
    currentApp = app; // Set the new current application
    xTimerStart(appTimer, 0); // Start the application timer for the new application
    currentApplication->init();    // Initialize the new application

    //taskEXIT_CRITICAL(); // End optional critical section
}



