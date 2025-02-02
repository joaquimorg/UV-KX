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

void delay250ns(const uint32_t delay);
void delayUs(uint32_t delay);
void delayMs(uint32_t delay);

void AESEncrypt(const void *pKey, const void *pIv, const void *pIn, void *pOut, uint8_t NumBlocks);

void CRCInit(void);
uint16_t CRCCalculate(const void *pBuffer, uint16_t Size);

