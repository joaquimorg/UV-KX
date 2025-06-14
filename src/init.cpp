/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 * Copyright 2023 Manuel Jedinger
 * https://github.com/manujedi
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

// -----------------------------------------------------------------------------------------
// This file contains low-level initialization routines that are typically executed
// during the C runtime startup, before the main application code begins.
// These routines are crucial for setting up the memory segments (BSS and DATA)
// for correct C/C++ program execution.
// -----------------------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

extern uint32_t __bss_start__[];
extern uint32_t __bss_end__[];
extern uint8_t flash_data_start[];
extern uint8_t sram_data_start[];
extern uint8_t sram_data_end[];

extern "C" void DATA_Init();
extern "C" void BSS_Init();

uint8_t UART_DMA_Buffer[256];

void BSS_Init(void) {
	for (uint32_t* pBss = __bss_start__; pBss < __bss_end__; pBss++) {
		*pBss = 0;
	}
}

void DATA_Init(void) {
	volatile uint32_t* pDataRam = (volatile uint32_t*)sram_data_start;
	volatile uint32_t* pDataFlash = (volatile uint32_t*)flash_data_start;
	uint32_t           Size = (uint32_t)sram_data_end - (uint32_t)sram_data_start;

	for (unsigned int i = 0; i < (Size / 4); i++) {
		*pDataRam++ = *pDataFlash++;
	}
}
