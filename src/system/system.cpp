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

    if (systemMessageQueue == NULL) {
        // need to haldle error
    }

    st7565.begin();
    bk4819.setupRegisters();

    radio.setVFO(Settings::VFOAB::VFOA, 44616875, 44616875, 0, ModType::MOD_FM);
    radio.setVFO(Settings::VFOAB::VFOB, 43932500, 43932500, 0, ModType::MOD_FM);
    radio.setupToVFO(Settings::VFOAB::VFOA);

    uart.print("UV-Kx Open Firmware - " AUTHOR_STRING " - " VERSION_STRING "\n");
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
    SystemMessages notification;

    //uart.sendLog("System task started\n");

    battery.getReadings(); // Update battery readings

    appTimer = xTimerCreateStatic("app", pdMS_TO_TICKS(100), pdTRUE, this, SystemTask::appTimerCallback, &appTimerBuffer);
    runTimer = xTimerCreateStatic("run", pdMS_TO_TICKS(500), pdFALSE, this, SystemTask::runTimerCallback, &runTimerBuffer);

    // Load the Welcome application        
    loadApplication(Applications::Applications::Welcome);

    backlight.setBacklight(Backlight::backLightState::ON); // Turn on backlight    

    keyboard.init(); // Initialize the keyboard

    playBeep(Settings::BEEPType::BEEP_880HZ_200MS);

    xTimerStart(appTimer, 0);
    xTimerStart(runTimer, 0);
    for (;;) {
        // Wait for notifications or messages
        if (xQueueReceive(systemMessageQueue, &notification, pdMS_TO_TICKS(5)) == pdTRUE) {
            // Process system notifications
            processSystemNotification(notification);
        }

        if (uart.isCommandAvailable()) {
            uart.handleCommand();
        }

        radio.checkRadioInterrupts(); // Check for radio interrupts
        radio.runDualWatch(); // Run dual watch
        //vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void SystemTask::processSystemNotification(SystemMessages notification) {
    // Handle different system notifications

    switch (notification.message) {
    case SystemMSG::MSG_TIMEOUT:
        //uart.sendLog("MSG_TIMEOUT\n");
        if (currentApp != Applications::Applications::None) {
            currentApplication->timeout();
        }
        if (keyboard.wasFKeyPressed()) {
            keyboard.clearFKeyPressed();
        }
        break;
    case SystemMSG::MSG_BKCLIGHT:
        //uart.sendLog("MSG_BKCLIGHT\n");
        timeoutLightCount = 0;
        backlight.setBacklight((Backlight::backLightState)notification.payload);
        break;
    case SystemMSG::MSG_PLAY_BEEP:
        playBeep((Settings::BEEPType)notification.payload);
        break;
    case SystemMSG::MSG_RADIO_RX:
        //uart.sendLog("MSG_RADIO_RX\n");        
        pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        break;
    case SystemMSG::MSG_RADIO_TX:
        //uart.sendLog("MSG_RADIO_TX\n");
        //pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        break;
    case SystemMSG::MSG_KEYPRESSED: {
        //uart.sendLog("MSG_KEYPRESSED\n");

        Keyboard::KeyCode key = notification.key;
        Keyboard::KeyState state = notification.state;

        if (currentApp != Applications::Applications::None) {
            currentApplication->action(key, state);
        }

        if (state == Keyboard::KeyState::KEY_PRESSED || state == Keyboard::KeyState::KEY_LONG_PRESSED) {
            timeoutCount = 0;
            timeoutLightCount = 0;
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
            if (key != Keyboard::KeyCode::KEY_PTT) {
                pushMessage(SystemMSG::MSG_PLAY_BEEP, (uint32_t)Settings::BEEPType::BEEP_1KHZ_60MS_OPTIONAL);
            }
            else {
                pushMessage(SystemMSG::MSG_RADIO_TX, 0);
            }
        }        

        break;
    }
    case SystemMSG::MSG_APP_LOAD:
        loadApplication((Applications::Applications)notification.payload);
        break;

    default:
        break;
    }
}

void SystemTask::runTimerImpl(void) {

    // 0.5 second timer

    battery.getReadings(); // Update battery readings
    if (currentApp != Applications::Applications::None) {
    }

    if (timeoutCount > (actionTimeout * 2)) {
        timeoutCount = 0;
        pushMessage(SystemMSG::MSG_TIMEOUT, 0);
    }
    else {
        timeoutCount++;
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
    xTimerStop(appTimer, 0);
    setActionTimeout(2);
    switch (app) {
    case Applications::Applications::Welcome:
        currentApplication = &welcomeApp;
        break;
    case Applications::Applications::MainVFO:
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



