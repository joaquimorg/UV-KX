/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
// -----------------------------------------------------------------------------------------
// This file contains the application-specific configuration settings for the FreeRTOS kernel.
// These settings tailor the RTOS behavior to the specific hardware and application requirements.
// For detailed information on each parameter, refer to the FreeRTOS documentation:
// http://www.freertos.org/a00110.html
// -----------------------------------------------------------------------------------------
/* USER CODE BEGIN Header */
/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
/* USER CODE END Header */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */


// Core RTOS behavior settings
#define configUSE_PREEMPTION                     1  // Enable preemptive scheduling. Tasks can be interrupted by higher priority tasks.
#define configSUPPORT_STATIC_ALLOCATION          1  // Enable support for static allocation of RTOS objects (tasks, queues, etc.).
#define configSUPPORT_DYNAMIC_ALLOCATION         0  // Disable support for dynamic allocation (heap usage by FreeRTOS itself).
#define configUSE_IDLE_HOOK                      0  // Disable the idle hook function.
#define configUSE_TICK_HOOK                      0  // Disable the tick hook function.
#define configCPU_CLOCK_HZ                       ( 48000000U ) // Define the CPU clock frequency (48MHz for this target).
#define configTICK_RATE_HZ                       ((TickType_t)1000U) // Set the RTOS tick rate to 1000Hz (1ms tick period).
#define configMAX_PRIORITIES                     ( 3 )  // Define the maximum number of task priorities.
#define configMINIMAL_STACK_SIZE                 ((uint16_t)200) // Define the minimum stack size (in words) for any task.
//#define configTOTAL_HEAP_SIZE                    ((size_t)3072) // Total heap size if dynamic allocation were enabled.
#define configMAX_TASK_NAME_LEN                  ( 6 )  // Maximum length of a task's name string.
#define configUSE_TRACE_FACILITY                 0  // Disable the trace facility for debugging.
#define configUSE_16_BIT_TICKS                   0  // Use 32-bit tick counter.

// Synchronization and Inter-task Communication
#define configUSE_MUTEXES                        1  // Enable the use of mutexes.
#define configQUEUE_REGISTRY_SIZE                8  // Define the size of the queue registry for debugging.
#define configUSE_RECURSIVE_MUTEXES              0  // Disable recursive mutexes.
#define configUSE_COUNTING_SEMAPHORES            1  // Enable counting semaphores.
#define configUSE_QUEUE_SETS					 1  // Enable queue sets.

// Performance and Optimization
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0  // Disable port-optimized task selection (uses generic C implementation).

// Debugging and Error Handling
#define configCHECK_FOR_STACK_OVERFLOW			 1  // Enable stack overflow detection (requires a hook function to be defined).

/* Co-routine definitions. */
// Co-routines are a legacy feature and generally not recommended for new designs.
#define configUSE_CO_ROUTINES                    0  // Disable co-routines.
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                         1  // Enable software timers.
#define configTIMER_TASK_PRIORITY                ( 1 )  // Set the priority for the timer service task.
#define configTIMER_QUEUE_LENGTH                 25 // Define the length of the timer command queue.
#define configTIMER_TASK_STACK_DEPTH             400// Define the stack size for the timer service task.

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
// These definitions control which optional FreeRTOS API functions are included in the build.
// Setting to 0 can reduce code size if the function is not used.
#define INCLUDE_vTaskPrioritySet            0  // Exclude vTaskPrioritySet.
#define INCLUDE_uxTaskPriorityGet           0  // Exclude uxTaskPriorityGet.
#define INCLUDE_vTaskDelete                 0  // Exclude vTaskDelete.
#define INCLUDE_vTaskCleanUpResources       0  // Exclude vTaskCleanUpResources (deprecated).
#define INCLUDE_vTaskSuspend                0  // Exclude vTaskSuspend.
#define INCLUDE_vTaskDelayUntil             0  // Exclude vTaskDelayUntil.
#define INCLUDE_vTaskDelay                  1  // Include vTaskDelay.
#define INCLUDE_xTaskGetSchedulerState      0  // Exclude xTaskGetSchedulerState.
#define INCLUDE_xTimerPendFunctionCall      0  // Exclude xTimerPendFunctionCall.
#define INCLUDE_xQueueGetMutexHolder        1  // Include xQueueGetMutexHolder.
#define INCLUDE_uxTaskGetStackHighWaterMark 1  // Include uxTaskGetStackHighWaterMark (useful for debugging stack usage).
#define INCLUDE_eTaskGetState               1  // Include eTaskGetState.

/*
 * The CMSIS-RTOS V2 FreeRTOS wrapper is dependent on the heap implementation used
 * by the application thus the correct define need to be enabled below
 */
//#define USE_FreeRTOS_HEAP_4 // Example for selecting a specific heap implementation (heap_4.c).

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
/* USER CODE BEGIN 1 */
// Custom assertion handler. If configASSERT is defined, FreeRTOS will call this on assertion failures.
//#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );} // Basic assert.

// More advanced assert that calls a function to report file and line number.
void vAssertCalled( unsigned long ulLine, const char * const pcFileName );
#define configASSERT( x ) if( (x) == 0 ) vAssertCalled( __LINE__, __FILE__ )
/* USER CODE END 1 */

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
// These map FreeRTOS interrupt handler names to the standard CMSIS handler names for the ARM Cortex-M port.
#define vPortSVCHandler    HandlerSVCall    // SVCall handler for context switching.
#define xPortPendSVHandler HandlerPendSV    // PendSV handler for context switching.

/* IMPORTANT: This define is commented when used with STM32Cube firmware, when the timebase source is SysTick,
              to prevent overwriting SysTick_Handler defined within STM32Cube HAL */
// This maps the FreeRTOS SysTick handler. It's often managed by the STM32Cube HAL.
#define xPortSysTickHandler SystickHandler

/* USER CODE BEGIN Defines */
/* Section where parameter definitions can be added (for instance, to override default ones in FreeRTOS.h) */
/* USER CODE END Defines */


#endif /* FREERTOS_CONFIG_H */
