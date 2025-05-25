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
#include <string.h> // Required for memcpy or similar, though not directly used in this snippet.

// External symbols defined by the linker script, indicating memory section boundaries.
extern uint32_t __bss_start__[];   ///< Start address of the .bss section in SRAM.
extern uint32_t __bss_end__[];     ///< End address of the .bss section in SRAM.
extern uint8_t flash_data_start[]; ///< Start address of the initialized data section in Flash.
extern uint8_t sram_data_start[];  ///< Start address of the .data section in SRAM.
extern uint8_t sram_data_end[];    ///< End address of the .data section in SRAM.

// Function prototypes for initialization routines.
// These are typically called by the startup code.
extern "C" void DATA_Init();
extern "C" void BSS_Init();

/**
 * @brief DMA (Direct Memory Access) buffer for UART communication.
 * This buffer is likely used by the UART driver to store incoming or outgoing data
 * when DMA is utilized for UART transfers, reducing CPU load.
 * The size is 256 bytes.
 */
uint8_t UART_DMA_Buffer[256];

/**
 * @brief Initializes the .bss section by zeroing it out.
 * The .bss section contains uninitialized global and static variables.
 * C/C++ standards require these variables to be initialized to zero before
 * the `main` function is called. This function iterates from `__bss_start__`
 * to `__bss_end__` (linker-defined symbols) and writes zeros to this memory region.
 */
void BSS_Init(void) {
	for (uint32_t* pBss = __bss_start__; pBss < __bss_end__; pBss++) {
		*pBss = 0; // Dereference and assign 0 to clear the memory content.
	}
}

/**
 * @brief Initializes the .data section by copying data from Flash to SRAM.
 * The .data section contains global and static variables that are initialized
 * with non-zero values. These initial values are stored in Flash memory and
 * must be copied to their corresponding locations in SRAM at startup.
 * This function copies data word by word (32-bit) from `flash_data_start`
 * to `sram_data_start` for the size of the .data section.
 */
void DATA_Init(void) {
	volatile uint32_t* pDataRam = (volatile uint32_t*)sram_data_start;    // Destination: SRAM .data section start.
	volatile uint32_t* pDataFlash = (volatile uint32_t*)flash_data_start; // Source: Flash .data section start.
	uint32_t Size = (uint32_t)sram_data_end - (uint32_t)sram_data_start; // Calculate size of .data section in bytes.

	// Copy data in 4-byte (uint32_t) chunks.
	for (unsigned int i = 0; i < (Size / 4); i++) {
		*pDataRam++ = *pDataFlash++; // Copy from Flash to SRAM and increment pointers.
	}
}
