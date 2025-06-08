// This file implements the SystemTask class, which is central to the application's architecture.
// It handles system initialization, message processing, task management, and application lifecycle.
#include "system.h"
#include "sys.h"

using namespace System;

// Static methods (required by FreeRTOS)

void SystemTask::runStatusTask(void* pvParameters) {
    SystemTask* systemTask = static_cast<SystemTask*>(pvParameters);
    if (systemTask) {
        systemTask->statusTaskImpl();
    }
}

void SystemTask::appTimerCallback(TimerHandle_t xTimer) {
    SystemTask* systemTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
    if (systemTask) {
        systemTask->appTimerImpl();
    }
}

void SystemTask::runTimerCallback(TimerHandle_t xTimer) {
    SystemTask* systemTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
    if (systemTask) {
        systemTask->runTimerImpl();
    }
    xTimerStart(xTimer, 0);
}

void SystemTask::initSystem(void) {
    // Create message queue
    systemMessageQueue = xQueueCreateStatic(queueLenght, itemSize, systemQueueStorageArea, &systemTasksQueue);

    /*if (systemMessageQueue == NULL) {
        // need to haldle error
        // uart.sendLog("systemMessageQueue is NULL");
        return;
    }*/
        
    delayMs(10);

    st7565.begin();
    uart.print("UV-Kx Open Firmware - " AUTHOR_STRING " - " VERSION_STRING "\n");
}

void SystemTask::setupRadio(void) {
    if (radio.isRadioReady()) return;

    bk4819.setupRegisters();

    delayMs(10);
    settings.getRadioSettings();
    uart.print("[DEBUG] EEPROM Version : %x\r\n", settings.getSettingsVersion());
    delayMs(10);

    if (!settings.validateSettingsVersion()) {
        settings.setRadioSettingsDefault();

        radio.setVFO(Settings::VFOAB::VFOA, 44616875, 44616875, 0, ModType::MOD_FM);
        radio.setVFO(Settings::VFOAB::VFOB, 43932500, 43932500, 0, ModType::MOD_FM);

        settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOA] = radio.getVFO(Settings::VFOAB::VFOA);
        settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOB] = radio.getVFO(Settings::VFOAB::VFOB);
        //settings.setRadioSettings();
    } else {
        // Load settings from EEPROM
        backlight.setBrightness(settings.radioSettings.backlightLevel);
        settings.applyRadioSettings();
        // TODO: need to validate if load VFO or Memory
        radio.setVFO(Settings::VFOAB::VFOA, settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOA]);
        radio.setVFO(Settings::VFOAB::VFOB, settings.radioSettings.vfo[(uint8_t)Settings::VFOAB::VFOB]);
    }
    
    radio.setupToVFO(settings.radioSettings.vfoSelected);
    
    radio.setRadioReady(true);
}

void SystemTask::pushMessage(SystemMSG msg, uint32_t value) {
    SystemMessages appMSG = { msg, value, (Keyboard::KeyCode)0, (Keyboard::KeyState)0 };
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(systemMessageQueue, (void*)&appMSG, &xHigherPriorityTaskWoken);
}

void SystemTask::pushMessageKey(Keyboard::KeyCode key, Keyboard::KeyState state) {
    SystemMessages appMSG = { SystemMSG::MSG_KEYPRESSED, 0, key, state };
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;
    xQueueSendFromISR(systemMessageQueue, (void*)&appMSG, &xHigherPriorityTaskWoken);
}

void SystemTask::statusTaskImpl() {
    SystemMessages notification;;

    //uart.sendLog("System task started");

    battery.getReadings(); // Update battery readings

    appTimer = xTimerCreateStatic("app", pdMS_TO_TICKS(100), pdTRUE, this, SystemTask::appTimerCallback, &appTimerBuffer);
    runTimer = xTimerCreateStatic("run", pdMS_TO_TICKS(500), pdFALSE, this, SystemTask::runTimerCallback, &runTimerBuffer);    

    backlight.setBacklight(Backlight::backLightState::ON); // Turn on backlight    

    keyboard.init(); // Initialize the keyboard

    playBeep(Settings::BEEPType::BEEP_880HZ_200MS);

    /*
    // Validate the EEPROM content and initialize if necessary
    if (!settings.validateSettingsVersion()) {
        pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::RESETINIT);
    }
    else {
        // Load the Welcome application
        pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::Welcome);
    } 
    */
    pushMessage(SystemMSG::MSG_APP_LOAD, (uint32_t)Applications::Applications::Welcome);

    xTimerStart(appTimer, 0);
    xTimerStart(runTimer, 0);
    
    //uart.print("lenght : %i\n", settings.stringLength(ui.generateCTDCList(Settings::DCSOptions, 104, false)));
    for (;;) {
        // Wait for notifications or messages
        if (xQueueReceive(systemMessageQueue, &notification, pdMS_TO_TICKS(5)) == pdTRUE) {
            // Process system notifications
            processSystemNotification(notification);
        }

        taskENTER_CRITICAL();
        if (uart.isCommandAvailable()) {
            uart.handleCommand();
        }
        taskEXIT_CRITICAL();

        radio.checkRadioInterrupts(); // Check for radio interrupts
        radio.runDualWatch(); // Run dual watch
        //vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void SystemTask::processSystemNotification(SystemMessages notification) {
    // Handle different system notifications

    switch (notification.message) {
    case SystemMSG::MSG_TIMEOUT:
        timeoutCount = 0;
        ui.timeOut();
        //uart.sendLog("MSG_TIMEOUT\n");
        battery.getReadings(); // Update battery readings
        if (battery.isLowBattery()) {
            pushMessage(SystemMSG::MSG_LOW_BATTERY, 0);
        }
        if (currentApp != Applications::Applications::None) {
            currentApplication->timeout();
        }
        if (keyboard.wasFKeyPressed()) {
            keyboard.clearFKeyPressed();
        }        
        break;
    case SystemMSG::MSG_POWER_SAVE:
        powerSaveCount = 0;
        if (radio.getState() == Settings::RadioState::IDLE) {
            radio.setPowerSaveMode();
        }        
        break;
    case SystemMSG::MSG_BKCLIGHT:
        //uart.sendLog("MSG_BKCLIGHT\n");
        timeoutLightCount = 0;
        backlight.setBacklight((Backlight::backLightState)notification.payload);
        break;
    case SystemMSG::MSG_BKCLIGHT_LEVEL:                
        backlight.setBrightness((uint8_t)notification.payload);
        //uart.print("MSG_BKCLIGHT_LEVEL: %d\n", (uint8_t)notification.payload);
        break;
    case SystemMSG::MSG_PLAY_BEEP:
        playBeep((Settings::BEEPType)notification.payload);
        break;
    case SystemMSG::MSG_RADIO_IDLE:
        powerSaveCount = 0;
        //uart.sendLog("MSG_RADIO_IDLE\n");
        break;
    case SystemMSG::MSG_RADIO_RX:
        //uart.sendLog("MSG_RADIO_RX\n");
        radio.setNormalPowerMode();
        powerSaveCount = 0;
        pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        break;
    case SystemMSG::MSG_LOW_BATTERY:        
        ui.setInfoMessage(UI::InfoMessageType::LOW_BATTERY);
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_880HZ_60MS_TRIPLE_BEEP);
        break;
    case SystemMSG::MSG_RADIO_TX:
        //uart.sendLog("MSG_RADIO_TX\n");
        //pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        ui.setInfoMessage(UI::InfoMessageType::TX_DISABLED);
        break;
    case SystemMSG::MSG_KEYPRESSED: {
        //uart.sendLog("MSG_KEYPRESSED");
        //uart.print("Key: %d\n", notification.key);
        //uart.print("State: %d\n", notification.state);
        Keyboard::KeyCode key = notification.key;
        Keyboard::KeyState state = notification.state;        

        radio.setNormalPowerMode();
        powerSaveCount = 0;
        if (currentApp != Applications::Applications::None) {
            currentApplication->action(key, state);
        }

        if (state == Keyboard::KeyState::KEY_PRESSED || state == Keyboard::KeyState::KEY_LONG_PRESSED) {
            timeoutCount = 0;
            timeoutLightCount = 0;
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
            if (key != Keyboard::KeyCode::KEY_PTT) {
                playBeep(Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL);
                //pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL);
            }
            else if ( radio.isRadioReady() ){
                pushMessage(SystemMSG::MSG_RADIO_TX, 0);
            }
        }
        break;
    }
    case SystemMSG::MSG_SAVESETTINGS:
        settings.requestSaveRadioSettings();
        break;
    
    case SystemMSG::MSG_APP_LOAD:
        loadApplication((Applications::Applications)notification.payload);
        break;

    default:
        break;
    }
}

void SystemTask::runTimerImpl(void) {

    // 0.5 second timer
    
    /*if (currentApp != Applications::Applications::None) {
    }*/

    if (timeoutCount > (actionTimeout * 2)) {
        //timeoutCount = 0;
        pushMessage(SystemMSG::MSG_TIMEOUT, 0);
    }
    else {
        timeoutCount++;
    }

    if (!radio.isPowerSaveMode()) {
        
        if (powerSaveCount > (powerSaveTimeout * 2)) {        
            pushMessage(SystemMSG::MSG_POWER_SAVE, 0);
        }
        else {
            powerSaveCount++;
        }
    }

    if (backlight.getBacklightState() == Backlight::backLightState::ON) {
        if (timeoutLightCount > (backlightTimeout * 2)) {
            //timeoutLightCount = 0;
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::OFF);
        }
        else {
            timeoutLightCount++;
        }
    }

    settings.handleSaveTimers();
}

void SystemTask::appTimerImpl(void) {
    // Update the current application    

    if (currentApp != Applications::Applications::None) {
        currentApplication->update();
    }
}

void SystemTask::loadApplication(Applications::Applications app) {
    if (app == Applications::Applications::None) return;
    //taskENTER_CRITICAL();

    currentApp = Applications::Applications::None;
    timeoutCount = 0;
    xTimerStop(appTimer, 0);
    setActionTimeout(2);
    switch (app) {
    case Applications::Applications::Welcome:
        currentApplication = &welcomeApp;
        break;
    case Applications::Applications::RESETINIT:
        currentApplication = &resetInitApp;
        setActionTimeout(1);
        break;
    case Applications::Applications::RESETEEPROM:
        currentApplication = &resetEEPROMApp;
        setActionTimeout(1);
        break;
    case Applications::Applications::MainVFO:
        setupRadio();
        currentApplication = &mainVFOApp;
        break;
    case Applications::Applications::Menu:
        currentApplication = &menuApp;
        break;
    case Applications::Applications::SETVFOA:
        currentApplication = &setVFOAApp;
        setActionTimeout(5);
        break;
    case Applications::Applications::SETVFOB:
        currentApplication = &setVFOBApp;
        setActionTimeout(5);
        break;
    case Applications::Applications::SETRADIO:
        currentApplication = &setRadioApp;
        setActionTimeout(5);
        break;
    case Applications::Applications::MESSENGER:
        // TODO: Implement Messenger
        //currentApplication = &messengerApp;
        currentApplication = &mainVFOApp;
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
        break;
    case Applications::Applications::SCANNER:
        // TODO: Implement Scanner
        //currentApplication = &scannerApp;
        currentApplication = &mainVFOApp;
        pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
        break;
    case Applications::Applications::ABOUT:
        currentApplication = &welcomeApp;
        break;
    default:
        break;
    }
    currentApp = app;
    xTimerStart(appTimer, 0);
    currentApplication->init();    

    //taskEXIT_CRITICAL();
}



