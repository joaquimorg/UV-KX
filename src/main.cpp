
// This file contains the main function and other core functionalities for the application.
// It includes system initializations, task creations, and error handling routines.

#include <cstdlib>
#include <cstddef>

#include "ARMCM0.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "system.h"
#include "misc.h"
#include "uart_hal.h"

#ifdef __cplusplus
extern "C" {
#endif


    // Function to flash the LED
    void flashLED(uint32_t times, uint32_t delayTime) {
        for (uint32_t i = 0; i < times; i++) {
            GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT); // Turn on the LED
            delayMs(delayTime);
            GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT); // Turn off the LED
            delayMs(delayTime);
        }
    }

    void _putchar(__attribute__((unused)) char c) {
        //UART uart;
        //uart.send((uint8_t*)&c, 1);
    }

    void vAssertCalled(__attribute__((unused)) unsigned long ulLine, __attribute__((unused)) const char* const pcFileName) {
        /*UART uart;
        //taskENTER_CRITICAL();
        {

            uart.print("[ASSERT ERROR] %s %s: line=%lu\r\n", __func__, pcFileName, ulLine);

        }
        //taskEXIT_CRITICAL();
        */
        while (1) {
            flashLED(2, 100);
            delayMs(1000);
        }
        
    }

    void vApplicationStackOverflowHook(__attribute__((unused)) TaskHandle_t pxTask, __attribute__((unused)) char* pcTaskName) {
        /*UART uart;
        //taskENTER_CRITICAL();
        {
            //#ifdef ENABLE_UART_DEBUG
            unsigned int stackWm = uxTaskGetStackHighWaterMark(pxTask);
            uart.print("[STACK ERROR] %s task=%s : %i\r\n", __func__, pcTaskName, stackWm);
            //#endif
        }
        //taskEXIT_CRITICAL();
        */        
        while (1) {
            flashLED(5, 100);
            delayMs(1000);
        }
    }

    /*-----------------------------------------------------------*/

    void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
        StackType_t** ppxIdleTaskStackBuffer,
        uint32_t* pulIdleTaskStackSize) {
        static StaticTask_t xIdleTaskTCB;
        static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

        *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
        *ppxIdleTaskStackBuffer = uxIdleTaskStack;
        *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    }

    void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
        StackType_t** ppxTimerTaskStackBuffer,
        uint32_t* pulTimerTaskStackSize)
    {
        static StaticTask_t xTimerTaskTCB;
        static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

        *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
        *ppxTimerTaskStackBuffer = uxTimerTaskStack;
        *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
    }

#ifdef __cplusplus
}
#endif

StackType_t systemTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t systemTaskBuffer;

int main(void) {

    configureSysTick();
    configureSysCon();
    boardGPIOInit();
    boardPORTCONInit();
    boardADCInit();
    CRCInit();

    System::SystemTask systemTask;

    xTaskCreateStatic(
        System::SystemTask::runStatusTask,
        "MAIN",
        ARRAY_SIZE(systemTaskStack),
        &systemTask,
        1 + tskIDLE_PRIORITY,
        systemTaskStack,
        &systemTaskBuffer
    );

    vTaskStartScheduler();

    while (1) {
        // oops, something went wrong
    }

}