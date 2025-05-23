
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

    /**
     * @brief Flashes an LED a specified number of times with a given delay.
     * 
     * @param times The number of times to flash the LED.
     * @param delayTime The delay time in milliseconds between flashes.
     */
    void flashLED(uint32_t times, uint32_t delayTime) {
        for (uint32_t i = 0; i < times; i++) {
            GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT); // Turn on the LED
            delayMs(delayTime); // Wait for the specified delay time
            GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_FLASHLIGHT); // Turn off the LED
            delayMs(delayTime); // Wait for the specified delay time
        }
    }

    /**
     * @brief Placeholder function for character output.
     *        Currently, it's commented out, but it can be used to send characters via UART.
     * 
     * @param c The character to output.
     */
    void _putchar(__attribute__((unused)) char c) {
        //UART uart;
        //uart.send((uint8_t*)&c, 1);
    }

    /**
     * @brief This function is called when an assertion fails.
     *        It prints an error message (currently commented out) and flashes an LED indefinitely.
     * 
     * @param ulLine The line number where the assertion failed.
     * @param pcFileName The file name where the assertion failed.
     */
    void vAssertCalled(__attribute__((unused)) unsigned long ulLine, __attribute__((unused)) const char* const pcFileName) {
        /*UART uart;
        //taskENTER_CRITICAL();
        {

            uart.print("[ASSERT ERROR] %s %s: line=%lu\r\n", __func__, pcFileName, ulLine);

        }
        //taskEXIT_CRITICAL();
        */
        // Infinite loop to indicate an assertion failure
        while (1) {
            flashLED(2, 100); // Flash LED twice with 100ms delay
            delayMs(1000); // Wait for 1 second
        }
        
    }

    /**
     * @brief This function is called when a stack overflow is detected.
     *        It prints an error message (currently commented out) and flashes an LED indefinitely.
     * 
     * @param pxTask Handle to the task that caused the stack overflow.
     * @param pcTaskName Name of the task that caused the stack overflow.
     */
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
        // Infinite loop to indicate a stack overflow
        while (1) {
            flashLED(5, 100); // Flash LED five times with 100ms delay
            delayMs(1000); // Wait for 1 second
        }
    }

    /*-----------------------------------------------------------*/

    /**
     * @brief Provides memory for the Idle task.
     *        This function is required by FreeRTOS when configSUPPORT_STATIC_ALLOCATION is set to 1.
     * 
     * @param ppxIdleTaskTCBBuffer Pointer to a variable that will hold the Idle task's TCB.
     * @param ppxIdleTaskStackBuffer Pointer to a variable that will hold the Idle task's stack.
     * @param pulIdleTaskStackSize Pointer to a variable that will hold the Idle task's stack size.
     */
    void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
        StackType_t** ppxIdleTaskStackBuffer,
        uint32_t* pulIdleTaskStackSize) {
        // Static allocation for the Idle task's TCB and stack
        static StaticTask_t xIdleTaskTCB;
        static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

        // Pass out a pointer to the StaticTask_t structure in which the Idle task's
        // state will be stored.
        *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

        // Pass out the array that will be used as the Idle task's stack.
        *ppxIdleTaskStackBuffer = uxIdleTaskStack;

        // Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
        // Note that, as the array is necessarily of type StackType_t,
        // configMINIMAL_STACK_SIZE is specified in words, not bytes.
        *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    }

    /**
     * @brief Provides memory for the Timer task.
     *        This function is required by FreeRTOS when configSUPPORT_STATIC_ALLOCATION is set to 1.
     * 
     * @param ppxTimerTaskTCBBuffer Pointer to a variable that will hold the Timer task's TCB.
     * @param ppxTimerTaskStackBuffer Pointer to a variable that will hold the Timer task's stack.
     * @param pulTimerTaskStackSize Pointer to a variable that will hold the Timer task's stack size.
     */
    void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
        StackType_t** ppxTimerTaskStackBuffer,
        uint32_t* pulTimerTaskStackSize)
    {
        // Static allocation for the Timer task's TCB and stack
        static StaticTask_t xTimerTaskTCB;
        static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

        // Pass out a pointer to the StaticTask_t structure in which the Timer
        // task's state will be stored.
        *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

        // Pass out the array that will be used as the Timer task's stack.
        *ppxTimerTaskStackBuffer = uxTimerTaskStack;

        // Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
        // Note that, as the array is necessarily of type StackType_t,
        // configTIMER_TASK_STACK_DEPTH is specified in words, not bytes.
        *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
    }

#ifdef __cplusplus
}
#endif

// Stack and task buffer for the system task
StackType_t systemTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t systemTaskBuffer;

/**
 * @brief Main function of the application.
 *        Initializes the system, creates tasks, and starts the FreeRTOS scheduler.
 * 
 * @return int Returns 0 if successful, though this function should never return.
 */
int main(void) {

    // Configure system tick and system control
    configureSysTick();
    configureSysCon();
    // Initialize board GPIO, PORTCON, ADC, and CRC
    boardGPIOInit();
    boardPORTCONInit();
    boardADCInit();
    CRCInit();

    // Create an instance of the system task
    System::SystemTask systemTask;

    // Create the main system task using static allocation
    xTaskCreateStatic(
        System::SystemTask::runStatusTask, // Function to be executed by the task
        "MAIN",                            // Name of the task
        ARRAY_SIZE(systemTaskStack),       // Stack size for the task
        &systemTask,                       // Parameter passed to the task
        1 + tskIDLE_PRIORITY,              // Priority of the task
        systemTaskStack,                   // Array to be used as the task's stack
        &systemTaskBuffer                  // Variable to hold the task's data structure
    );

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // This part should never be reached.
    // If it is, it indicates an error in the scheduler setup.
    while (1) {
        // oops, something went wrong
    }

}