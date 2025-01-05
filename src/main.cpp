
#include <cstdlib>
#include <cstddef>

#include "ARMCM0.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "system.h"
#include "misc.h"

#ifdef __cplusplus
extern "C" {
#endif

void _putchar(__attribute__((unused)) char c) {
	//UART_Send((uint8_t *)&c, 1);
}

void vAssertCalled( __attribute__((unused)) unsigned long ulLine, __attribute__((unused)) const char * const pcFileName ) {

    /*taskENTER_CRITICAL();
    {
        
		LogUartf("[ASSERT ERROR] %s %s: line=%lu\r\n", __func__, pcFileName, ulLine);
		
    }
    taskEXIT_CRITICAL();*/
}

void vApplicationStackOverflowHook( __attribute__((unused)) TaskHandle_t pxTask, __attribute__((unused)) char *pcTaskName ) {  	

	/*taskENTER_CRITICAL();
    {
        #ifdef ENABLE_UART_DEBUG
			unsigned int stackWm = uxTaskGetStackHighWaterMark(pxTask);
			LogUartf("[STACK ERROR] %s task=%s : %i\r\n", __func__, pcTaskName, stackWm);
		#endif
    }
    taskEXIT_CRITICAL();*/

}

/*-----------------------------------------------------------*/

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize ) {
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#ifdef __cplusplus
}
#endif

/*
void* operator new(size_t size) {
    return malloc(size);
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    free(ptr);
}
*/

StackType_t systemTaskStack[configMINIMAL_STACK_SIZE + 200];
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

    while(1) {
		// oops, something went wrong
  	}

}