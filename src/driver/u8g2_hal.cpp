#include "u8g2_hal.h"

#include "sys.h"
#include "gpio.h"
#include "spi.h"
#include "gpio_hal.h"

uint8_t u8x8_gpio_and_delay_cb(__attribute__((unused)) u8x8_t* u8g2, uint8_t msg, uint8_t arg_int, __attribute__((unused)) void* arg_ptr) {
    switch (msg)
    {
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
        delayMs(arg_int);
        break;
    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
        if(arg_int) {
            GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
        } else {
            GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_A0);
        }
        break;
    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
        if(arg_int) {
            GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
        } else {
            GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_ST7565_RES);
        }
        break;
    default:
        break;
    }
    return 1;
}

void SPI_WaitForUndocumentedTxFifoStatusBit(void) {
	uint32_t Timeout;

	Timeout = 0;
	do {
		// Undocumented bit!
		if ((SPI0->IF & 0x20) == 0) {
			break;
		}
		Timeout++;
	} while (Timeout <= 100000);
}

uint8_t u8x8_hw_spi_cb(u8x8_t* u8g2, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    uint8_t* data;
    switch (msg) {
    case U8X8_MSG_BYTE_SEND: // write data to display
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0) {
            while ((SPI0->FIFOST & SPI_FIFOST_TFF_MASK) != SPI_FIFOST_TFF_BITS_NOT_FULL) {}
            SPI0->WDR = (uint8_t)*data;
            data++;
            arg_int--;
        }
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        //SPI0->CR = (SPI0->CR & ~SPI_CR_MSR_SSN_MASK) | SPI_CR_MSR_SSN_BITS_DISABLE;
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        //SPI0->CR = (SPI0->CR & ~SPI_CR_MSR_SSN_MASK) | SPI_CR_MSR_SSN_BITS_ENABLE;
        SPI_WaitForUndocumentedTxFifoStatusBit();
        break;
    case U8X8_MSG_BYTE_SET_DC:
        u8x8_gpio_SetDC(u8g2, arg_int);
        break;
    default:
        return 0;
    }
    return 1;
}
