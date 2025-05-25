/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
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
// This file implements the driver functions for the Analog-to-Digital Converter (ADC)
// peripheral. It provides low-level control for ADC initialization, configuration,
// starting conversions, and reading converted data.
// The implementations directly interact with the ADC hardware registers.
// -----------------------------------------------------------------------------------------


#include "ARMCM0.h" // Core ARM Cortex-M0 definitions
#include "adc.h"    // ADC driver interface (this file's header)
#include "irq.h"    // Interrupt request definitions/handling (NVIC)
#include "saradc.h" // SAR ADC peripheral register definitions
#include "syscon.h" // System controller register definitions (for clock gating/selection)

/**
 * @brief Gets the channel number (0-15) from an ADC channel mask.
 * This function iterates through possible channel masks to find the
 * corresponding channel number. It assumes only one channel bit is set in the Mask.
 * @param Mask The ADC_CH_MASK enum value (e.g., ADC_CH0, ADC_CH1).
 * @return The channel number as a uint8_t. Returns 0 if no known bit is set (or for ADC_CH0).
 */
uint8_t ADC_GetChannelNumber(ADC_CH_MASK Mask)
{
	// Iteratively check each channel bit from highest to lowest.
	// This is a common way to convert a single-bit mask to an index.
	if (Mask & ADC_CH15) return 15U;
	if (Mask & ADC_CH14) return 14U;
	if (Mask & ADC_CH13) return 13U;
	if (Mask & ADC_CH12) return 12U;
	if (Mask & ADC_CH11) return 11U;
	if (Mask & ADC_CH10) return 10U;
	if (Mask & ADC_CH9) return 9U;
	if (Mask & ADC_CH8) return 8U;
	if (Mask & ADC_CH7) return 7U;
	if (Mask & ADC_CH6) return 6U;
	if (Mask & ADC_CH5) return 5U;
	if (Mask & ADC_CH4) return 4U;
	if (Mask & ADC_CH3) return 3U;
	if (Mask & ADC_CH2) return 2U;
	if (Mask & ADC_CH1) return 1U;
	if (Mask & ADC_CH0) return 0U;

	return 0U; // Default or error case
}

/**
 * @brief Disables the ADC peripheral.
 * This is done by clearing the ADC enable bit in the SARADC_CFG register.
 */
void ADC_Disable(void)
{
	// Read-modify-write to clear the ADC_EN bit.
	SARADC_CFG = (SARADC_CFG & ~SARADC_CFG_ADC_EN_MASK) | SARADC_CFG_ADC_EN_BITS_DISABLE;
}

/**
 * @brief Enables the ADC peripheral.
 * This is done by setting the ADC enable bit in the SARADC_CFG register.
 */
void ADC_Enable(void)
{
	// Read-modify-write to set the ADC_EN bit.
	SARADC_CFG = (SARADC_CFG & ~SARADC_CFG_ADC_EN_MASK) | SARADC_CFG_ADC_EN_BITS_ENABLE;
}

/**
 * @brief Performs a software reset of the ADC peripheral.
 * This involves asserting and then de-asserting the soft reset bit in the SARADC_START register.
 */
void ADC_SoftReset(void)
{
	// Assert the soft reset bit.
	SARADC_START = (SARADC_START & ~SARADC_START_SOFT_RESET_MASK) | SARADC_START_SOFT_RESET_BITS_ASSERT;
	// De-assert the soft reset bit to complete the reset sequence.
	SARADC_START = (SARADC_START & ~SARADC_START_SOFT_RESET_MASK) | SARADC_START_SOFT_RESET_BITS_DEASSERT;
}

// Note: The comment below indicates a discrepancy between firmware understanding and TRM (Technical Reference Manual)
// regarding the bit positions for W_SARADC_SMPL_CLK_SEL. The code uses the firmware's understanding.
// The firmware thinks W_SARADC_SMPL_CLK_SEL is at [8:7] but the TRM says it's at [10:9]
#define FW_R_SARADC_SMPL_SHIFT 7
#define FW_R_SARADC_SMPL_MASK (3U << FW_R_SARADC_SMPL_SHIFT)

/**
 * @brief Retrieves the current ADC clock configuration from system control registers.
 * This function reads the system clock selection register and reformats parts of it,
 * potentially to match the bit layout expected by other ADC configuration steps or
 * due to discrepancies noted in the comments (FW_R_SARADC_SMPL_SHIFT).
 * @return A uint32_t value representing parts of the clock configuration.
 */
uint32_t ADC_GetClockConfig(void)
{
	uint32_t Value;

	Value = SYSCON_CLK_SEL; // Read the system clock selection register.

	// Re-arrange bits related to PLL and SARADC sample clock selection.
	// This appears to be converting read bit positions to write bit positions
	// for some fields, considering the firmware's interpretation of bit locations.
	Value = 0
		| (Value & ~(SYSCON_CLK_SEL_R_PLL_MASK | FW_R_SARADC_SMPL_MASK)) // Clear current PLL and firmware-interpreted sample clock bits
		| (((Value & SYSCON_CLK_SEL_R_PLL_MASK) >> SYSCON_CLK_SEL_R_PLL_SHIFT) << SYSCON_CLK_SEL_W_PLL_SHIFT) // Remap PLL read bits to write position
		| (((Value & FW_R_SARADC_SMPL_MASK) >> FW_R_SARADC_SMPL_SHIFT) << SYSCON_CLK_SEL_W_SARADC_SMPL_SHIFT) // Remap firmware-interpreted sample clock bits to write position
		;

	return Value;
}

/**
 * @brief Configures the ADC peripheral with the settings provided in pAdc.
 * This involves enabling the ADC clock gate, setting ADC clock source,
 * configuring various ADC operational parameters (channel selection, averaging,
 * sampling times, trigger modes, DMA, interrupts), and calibration settings.
 * @param pAdc Pointer to an ADC_Config_t structure containing the desired ADC configuration.
 */
void ADC_Configure(ADC_Config_t* pAdc)
{
	// Enable the clock gate for the SARADC peripheral in the system controller.
	SYSCON_DEV_CLK_GATE = (SYSCON_DEV_CLK_GATE & ~SYSCON_DEV_CLK_GATE_SARADC_MASK) | SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE;

	ADC_Disable(); // Disable ADC before reconfiguring it.

	// Set the ADC sample clock source based on pAdc->CLK_SEL.
	SYSCON_CLK_SEL = (ADC_GetClockConfig() & ~SYSCON_CLK_SEL_W_SARADC_SMPL_MASK) | ((pAdc->CLK_SEL << SYSCON_CLK_SEL_W_SARADC_SMPL_SHIFT) & SYSCON_CLK_SEL_W_SARADC_SMPL_MASK);

	// Configure the main ADC settings register (SARADC_CFG).
	// This is a read-modify-write operation to preserve reserved/other bits.
	SARADC_CFG = 0
		| (SARADC_CFG & ~(0 // Start with current SARADC_CFG and clear specific fields
			| SARADC_CFG_CH_SEL_MASK        // Channel selection
			| SARADC_CFG_AVG_MASK           // Averaging
			| SARADC_CFG_CONT_MASK          // Continuous mode
			| SARADC_CFG_SMPL_SETUP_MASK    // Sample setup time
			| SARADC_CFG_MEM_MODE_MASK      // Memory mode
			| SARADC_CFG_SMPL_CLK_MASK      // Sample clock divider
			| SARADC_CFG_SMPL_WIN_MASK      // Sample window duration
			| SARADC_CFG_ADC_TRIG_MASK      // ADC trigger source
			| SARADC_CFG_DMA_EN_MASK        // DMA enable
			))
		// Apply new settings from pAdc structure, shifting and masking each field.
		| ((pAdc->CH_SEL << SARADC_CFG_CH_SEL_SHIFT) & SARADC_CFG_CH_SEL_MASK)
		| ((pAdc->AVG << SARADC_CFG_AVG_SHIFT) & SARADC_CFG_AVG_MASK)
		| ((pAdc->CONT << SARADC_CFG_CONT_SHIFT) & SARADC_CFG_CONT_MASK)
		| ((pAdc->SMPL_SETUP << SARADC_CFG_SMPL_SETUP_SHIFT) & SARADC_CFG_SMPL_SETUP_MASK)
		| ((pAdc->MEM_MODE << SARADC_CFG_MEM_MODE_SHIFT) & SARADC_CFG_MEM_MODE_MASK)
		| ((pAdc->SMPL_CLK << SARADC_CFG_SMPL_CLK_SHIFT) & SARADC_CFG_SMPL_CLK_MASK)
		| ((pAdc->SMPL_WIN << SARADC_CFG_SMPL_WIN_SHIFT) & SARADC_CFG_SMPL_WIN_MASK)
		| ((pAdc->ADC_TRIG << SARADC_CFG_ADC_TRIG_SHIFT) & SARADC_CFG_ADC_TRIG_MASK)
		| ((pAdc->DMA_EN << SARADC_CFG_DMA_EN_SHIFT) & SARADC_CFG_DMA_EN_MASK)
		;

	// Configure external trigger selection (currently commented out).
	//SARADC_EXTTRIG_SEL = pAdc->EXTTRIG_SEL;

	// Configure calibration offset valid flag.
	if (pAdc->CALIB_OFFSET_VALID) {
		SARADC_CALIB_OFFSET = (SARADC_CALIB_OFFSET & ~SARADC_CALIB_OFFSET_VALID_MASK) | SARADC_CALIB_OFFSET_VALID_BITS_YES;
	}
	else {
		SARADC_CALIB_OFFSET = (SARADC_CALIB_OFFSET & ~SARADC_CALIB_OFFSET_VALID_MASK) | SARADC_CALIB_OFFSET_VALID_BITS_NO;
	}
	// Configure calibration KD factor valid flag.
	if (pAdc->CALIB_KD_VALID) {
		SARADC_CALIB_KD = (SARADC_CALIB_KD & ~SARADC_CALIB_KD_VALID_MASK) | SARADC_CALIB_KD_VALID_BITS_YES;
	}
	else {
		SARADC_CALIB_KD = (SARADC_CALIB_KD & ~SARADC_CALIB_KD_VALID_MASK) | SARADC_CALIB_KD_VALID_BITS_NO;
	}

	// Clear all ADC interrupt flags by writing 1s to them.
	SARADC_IF = 0xFFFFFFFF;
	// Configure ADC interrupt enable register (SARADC_IE).
	SARADC_IE = 0
		| (SARADC_IE & ~(0 // Start with current SARADC_IE and clear specific fields
			| SARADC_IE_CHx_EOC_MASK       // End-of-conversion interrupt per channel
			| SARADC_IE_FIFO_FULL_MASK     // FIFO full interrupt
			| SARADC_IE_FIFO_HFULL_MASK    // FIFO half-full interrupt
			))
		// Apply new interrupt enable settings from pAdc.
		| ((pAdc->IE_CHx_EOC << SARADC_IE_CHx_EOC_SHIFT) & SARADC_IE_CHx_EOC_MASK)
		| ((pAdc->IE_FIFO_FULL << SARADC_IE_FIFO_FULL_SHIFT) & SARADC_IE_FIFO_FULL_MASK)
		| ((pAdc->IE_FIFO_HFULL << SARADC_IE_FIFO_HFULL_SHIFT) & SARADC_IE_FIFO_HFULL_MASK)
		;

	// Enable or disable the ADC interrupt in the NVIC based on whether any ADC interrupts are enabled.
	if (SARADC_IE == 0) {
		NVIC_DisableIRQ((IRQn_Type)DP32_SARADC_IRQn);
	}
	else {
		NVIC_EnableIRQ((IRQn_Type)DP32_SARADC_IRQn);
	}
}

/**
 * @brief Starts an ADC conversion.
 * This is done by setting the START bit in the SARADC_START register.
 */
void ADC_Start(void)
{
	// Set the START bit to begin ADC conversion(s).
	SARADC_START = (SARADC_START & ~SARADC_START_START_MASK) | SARADC_START_START_BITS_ENABLE;
}

/**
 * @brief Checks if an ADC conversion has completed for the specified channel.
 * @param Mask The ADC_CH_MASK enum value for the channel to check.
 * @return True if the End-Of-Conversion (EOC) flag is set for the channel, false otherwise.
 */
bool ADC_CheckEndOfConversion(ADC_CH_MASK Mask)
{
	// Base address of the channel status/data registers.
	volatile ADC_Channel_t* pChannels = (volatile ADC_Channel_t*)&SARADC_CH0;
	uint8_t Channel = ADC_GetChannelNumber(Mask); // Convert mask to channel index.

	// Read the EOC bit from the specific channel's status register.
	return (pChannels[Channel].STAT & ADC_CHx_STAT_EOC_MASK) >> ADC_CHx_STAT_EOC_SHIFT;
}

/**
 * @brief Retrieves the converted ADC value for the specified channel.
 * After reading, it clears the interrupt flag for that channel.
 * @param Mask The ADC_CH_MASK enum value for the channel from which to read the data.
 * @return The 16-bit converted ADC value.
 */
uint16_t ADC_GetValue(ADC_CH_MASK Mask)
{
	// Base address of the channel status/data registers.
	volatile ADC_Channel_t* pChannels = (volatile ADC_Channel_t*)&SARADC_CH0;
	uint8_t Channel = ADC_GetChannelNumber(Mask); // Convert mask to channel index.

	// Clear the interrupt flag for this channel by writing 1 to its bit in SARADC_IF.
	// TODO: The comment "Or just use 'Mask'" suggests `SARADC_IF = Mask;` might also work if Mask is a single bit.
	SARADC_IF = 1 << Channel; 

	// Read the data from the specific channel's data register and mask/shift to get the value.
	return (pChannels[Channel].DATA & ADC_CHx_DATA_DATA_MASK) >> ADC_CHx_DATA_DATA_SHIFT;
}

