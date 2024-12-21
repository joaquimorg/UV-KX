#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstddef>

uint32_t getElapsedMilliseconds(void);

void configureSysTick(void);
void configureSysCon(void);
void boardGPIOInit(void);
void boardPORTCONInit(void);

void boardADCInit(void);
void boardADCGetBatteryInfo(uint16_t *pVoltage, uint16_t *pCurrent);

void delayUs(uint32_t delay);
void delayMs(uint32_t delay);


