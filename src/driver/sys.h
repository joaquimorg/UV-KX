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

void delayUs(uint32_t Delay);
void delayMs(uint32_t Delay);

/*
// DUMMY functions
int     _close(int);
int     _fstat(int, struct stat*);
int     _isatty(int);
_off_t  _lseek(int, _off_t, int);
int     _write(int, const void*, size_t);
int     _read(int, void*, size_t);
*/
