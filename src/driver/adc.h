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
// This file defines the interface for the Analog-to-Digital Converter (ADC) driver.
// It provides enumerations for channel selection, a structure for ADC configuration,
// and function prototypes for initializing, controlling, and reading values from the ADC.
// -----------------------------------------------------------------------------------------

#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enumeration of ADC channel masks.
 * Each value represents a bitmask for selecting one or more ADC channels.
 */
enum ADC_CH_MASK {
	ADC_CH0  = 0x0001U, ///< ADC Channel 0
	ADC_CH1  = 0x0002U, ///< ADC Channel 1
	ADC_CH2  = 0x0004U, ///< ADC Channel 2
	ADC_CH3  = 0x0008U, ///< ADC Channel 3
	ADC_CH4  = 0x0010U, ///< ADC Channel 4
	ADC_CH5  = 0x0020U, ///< ADC Channel 5
	ADC_CH6  = 0x0040U, ///< ADC Channel 6
	ADC_CH7  = 0x0080U, ///< ADC Channel 7
	ADC_CH8  = 0x0100U, ///< ADC Channel 8
	ADC_CH9  = 0x0200U, ///< ADC Channel 9
	ADC_CH10 = 0x0400U, ///< ADC Channel 10
	ADC_CH11 = 0x0800U, ///< ADC Channel 11
	ADC_CH12 = 0x1000U, ///< ADC Channel 12
	ADC_CH13 = 0x2000U, ///< ADC Channel 13
	ADC_CH14 = 0x4000U, ///< ADC Channel 14
	ADC_CH15 = 0x8000U, ///< ADC Channel 15
};

typedef enum ADC_CH_MASK ADC_CH_MASK; ///< Typedef for ADC_CH_MASK enum.

/**
 * @brief Structure defining the ADC configuration parameters.
 * This structure holds all the settings required to configure the ADC peripheral.
 */
typedef struct {
	uint16_t EXTTRIG_SEL;        ///< External trigger selection.
	uint16_t IE_CHx_EOC;         ///< Interrupt enable for end-of-conversion on specific channels.
	ADC_CH_MASK CH_SEL;          ///< Channel selection mask.
	uint8_t CLK_SEL;             ///< Clock selection for ADC.
	uint8_t AVG;                 ///< Averaging setting (e.g., number of samples to average).
	uint8_t CONT;                ///< Continuous conversion mode (0 = single, 1 = continuous).
	uint8_t MEM_MODE;            ///< Memory mode (e.g., overwrite, accumulate).
	uint8_t SMPL_CLK;            ///< Sample clock selection.
	uint8_t SMPL_SETUP;          ///< Sample setup time.
	uint8_t SMPL_WIN;            ///< Sample window duration.
	uint8_t ADC_TRIG;            ///< ADC trigger source selection.
	uint8_t DMA_EN;              ///< DMA enable for ADC.
	uint8_t IE_FIFO_HFULL;       ///< Interrupt enable for FIFO half full.
	uint8_t IE_FIFO_FULL;        ///< Interrupt enable for FIFO full.
	bool CALIB_OFFSET_VALID;     ///< Calibration offset valid flag.
	bool CALIB_KD_VALID;         ///< Calibration KD factor valid flag.
	uint8_t _pad[1];             ///< Padding for alignment.
} ADC_Config_t;

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Gets the channel number from an ADC channel mask.
	 * Assumes only one bit is set in the mask.
	 * @param Mask The ADC channel mask (e.g., ADC_CH0).
	 * @return The channel number (0-15).
	 */
	uint8_t ADC_GetChannelNumber(ADC_CH_MASK Mask);

	/**
	 * @brief Disables the ADC peripheral.
	 */
	void ADC_Disable(void);

	/**
	 * @brief Enables the ADC peripheral.
	 */
	void ADC_Enable(void);

	/**
	 * @brief Performs a software reset of the ADC peripheral.
	 */
	void ADC_SoftReset(void);

	/**
	 * @brief Gets the current ADC clock configuration.
	 * @return A value representing the clock configuration (specifics depend on hardware).
	 */
	uint32_t ADC_GetClockConfig(void);

	/**
	 * @brief Configures the ADC peripheral with the provided settings.
	 * @param pAdc Pointer to an ADC_Config_t structure containing the desired configuration.
	 */
	void ADC_Configure(ADC_Config_t* pAdc);

	/**
	 * @brief Starts an ADC conversion sequence.
	 */
	void ADC_Start(void);

	/**
	 * @brief Checks if the end-of-conversion (EOC) flag is set for a specific channel.
	 * @param Mask The ADC channel mask to check.
	 * @return True if conversion is complete for the specified channel, false otherwise.
	 */
	bool ADC_CheckEndOfConversion(ADC_CH_MASK Mask);

	/**
	 * @brief Gets the converted digital value from a specific ADC channel.
	 * It's recommended to check ADC_CheckEndOfConversion before calling this.
	 * @param Mask The ADC channel mask from which to read the value.
	 * @return The 16-bit digital value from the ADC conversion.
	 */
	uint16_t ADC_GetValue(ADC_CH_MASK Mask);

#ifdef __cplusplus
}
#endif // DRIVER_ADC_H

#endif

