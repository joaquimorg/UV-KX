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

    radio.setVFO(0, 44601875, 44601875, 0, ModType::MOD_FM);
    radio.setupToVFO(0);

    uart.print("UV-Kx Open Firmware - " AUTHOR_STRING " - " VERSION_STRING "\n");
}

void SystemTask::pushMessage(SystemMSG msg, uint32_t value) {
    SystemMessages appMSG = { msg, value };
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(systemMessageQueue, (void*)&appMSG, &xHigherPriorityTaskWoken);
}

void SystemTask::statusTaskImpl() {
    SystemMessages notification;

    //uart.sendLog("System task started\n");

    battery.getReadings(); // Update battery readings

    appTimer = xTimerCreateStatic("app", pdMS_TO_TICKS(200), pdTRUE, this, SystemTask::appTimerCallback, &appTimerBuffer);
    runTimer = xTimerCreateStatic("run", pdMS_TO_TICKS(1000), pdFALSE, this, SystemTask::runTimerCallback, &runTimerBuffer);

    // Load the Welcome application        
    loadApplication(Applications::Applications::Welcome);

    backlight.setBacklight(Backlight::backLightState::ON); // Turn on backlight    

    xTimerStart(appTimer, 0);
    xTimerStart(runTimer, 0);

    for (;;) {
        // Wait for notifications or messages
        if (xQueueReceive(systemMessageQueue, &notification, pdMS_TO_TICKS(5)) == pdTRUE) {
            // Process system notifications
            processSystemNotification(notification);
        }

        // main app action
        if (currentApp != Applications::Applications::None) {
            currentApplication->action();
        }

        keypad.update(); // Update keypad readings        
        radio.checkRadioInterrupts(); // Check for radio interrupts
        if (keypad.isPressed()) {
            timeoutCount = 0;
            timeoutLightCount = 0;
            pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
        break;
    case SystemMSG::MSG_BKCLIGHT:
        //uart.sendLog("MSG_BKCLIGHT\n");
        timeoutLightCount = 0;
        backlight.setBacklight((Backlight::backLightState)notification.payload);
        break;
    case SystemMSG::MSG_RADIO_RX:
        //uart.sendLog("MSG_RADIO_RX\n");        
        pushMessage(SystemMSG::MSG_BKCLIGHT, (uint32_t)Backlight::backLightState::ON);
        break;
    case SystemMSG::MSG_APP_LOAD:
        loadApplication((Applications::Applications)notification.payload);
        break;

    default:
        break;
    }
}

void SystemTask::runTimerImpl(void) {

    // 1 second timer

    battery.getReadings(); // Update battery readings
    if (currentApp != Applications::Applications::None) {
    }

    if (timeoutCount > actionTimeout) {
        timeoutCount = 0;
        pushMessage(SystemMSG::MSG_TIMEOUT, 0);
    }
    else {
        timeoutCount++;
    }

    if (backlight.getBacklightState() == Backlight::backLightState::ON) {
        if (timeoutLightCount > backlightTimeout) {
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
    xTimerStop(appTimer, 0);
    currentApp = Applications::Applications::None;
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
    default:
        break;
    }
    currentApp = app;
    currentApplication->init();
    vTaskDelay(pdMS_TO_TICKS(10));
    xTimerStart(appTimer, 0);
}



