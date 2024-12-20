#pragma once

#include "ARMCM0.h"
#include "spi.h"
#include "syscon.h"
#include "irq.h"
#include "gpio.h"


typedef struct {
	uint8_t MSTR;
	uint8_t SPR;
	uint8_t CPHA;
	uint8_t CPOL;
	uint8_t LSB;
	uint8_t TF_CLR;
	uint8_t RF_CLR;
	uint8_t TXFIFO_HFULL;
	uint8_t TXFIFO_EMPTY;
	uint8_t RXFIFO_HFULL;
	uint8_t RXFIFO_FULL;
	uint8_t RXFIFO_OVF;
} SPI_Config_t;

// Class for SPI functionality
class SPI {

public:

    SPI(volatile SPI_Port_t *spiPort) {
        SPI_Config_t Config{};

        pPort = spiPort;

        Disable();

        Config.TXFIFO_EMPTY = 0;
        Config.RXFIFO_HFULL = 0;
        Config.RXFIFO_FULL = 0;
        Config.RXFIFO_OVF = 0;
        Config.MSTR = 1;
        Config.SPR = 2;
        Config.CPHA = 1;
        Config.CPOL = 1;
        Config.LSB = 0;
        Config.TF_CLR = 0;
        Config.RF_CLR = 0;
        Config.TXFIFO_HFULL = 0;

        Configure(&Config);

        Enable();
        ToggleMasterMode(false);
    }

    void WaitForUndocumentedTxFifoStatusBit() {
        uint32_t Timeout = 0;

        do {
            if ((pPort->IF & 0x20) == 0) {
                break;
            }
            Timeout++;
        } while (Timeout <= 100000);
    }

    void Disable() {
        pPort->CR = (pPort->CR & ~SPI_CR_SPE_MASK) | SPI_CR_SPE_BITS_DISABLE;
    }

    void Configure(SPI_Config_t *pConfig) {
        if (pPort == SPI0) {
            SYSCON_DEV_CLK_GATE = (SYSCON_DEV_CLK_GATE & ~SYSCON_DEV_CLK_GATE_SPI0_MASK) | SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE;
        } else if (pPort == SPI1) {
            SYSCON_DEV_CLK_GATE = (SYSCON_DEV_CLK_GATE & ~SYSCON_DEV_CLK_GATE_SPI1_MASK) | SYSCON_DEV_CLK_GATE_SPI1_BITS_ENABLE;
        }

        Disable();

        pPort->CR = 0
            | (pPort->CR & ~(SPI_CR_SPR_MASK | SPI_CR_CPHA_MASK | SPI_CR_CPOL_MASK | SPI_CR_MSTR_MASK | SPI_CR_LSB_MASK | SPI_CR_RF_CLR_MASK))
            | ((pConfig->SPR    << SPI_CR_SPR_SHIFT)    & SPI_CR_SPR_MASK)
            | ((pConfig->CPHA   << SPI_CR_CPHA_SHIFT)   & SPI_CR_CPHA_MASK)
            | ((pConfig->CPOL   << SPI_CR_CPOL_SHIFT)   & SPI_CR_CPOL_MASK)
            | ((pConfig->MSTR   << SPI_CR_MSTR_SHIFT)   & SPI_CR_MSTR_MASK)
            | ((pConfig->LSB    << SPI_CR_LSB_SHIFT)    & SPI_CR_LSB_MASK)
            | ((pConfig->RF_CLR << SPI_CR_RF_CLR_SHIFT) & SPI_CR_RF_CLR_MASK)
            | ((pConfig->TF_CLR << SPI_CR_TF_CLR_SHIFT) & SPI_CR_TF_CLR_MASK);

        pPort->IE = 0
            | ((pConfig->RXFIFO_OVF << SPI_IE_RXFIFO_OVF_SHIFT) & SPI_IE_RXFIFO_OVF_MASK)
            | ((pConfig->RXFIFO_FULL << SPI_IE_RXFIFO_FULL_SHIFT) & SPI_IE_RXFIFO_FULL_MASK)
            | ((pConfig->RXFIFO_HFULL << SPI_IE_RXFIFO_HFULL_SHIFT) & SPI_IE_RXFIFO_HFULL_MASK)
            | ((pConfig->TXFIFO_EMPTY << SPI_IE_TXFIFO_EMPTY_SHIFT) & SPI_IE_TXFIFO_EMPTY_MASK)
            | ((pConfig->TXFIFO_HFULL << SPI_IE_TXFIFO_HFULL_SHIFT) & SPI_IE_TXFIFO_HFULL_MASK);

        if (pPort->IE) {
            if (pPort == SPI0) {
                NVIC_EnableIRQ((IRQn_Type)DP32_SPI0_IRQn);
            } else if (pPort == SPI1) {
                NVIC_EnableIRQ((IRQn_Type)DP32_SPI1_IRQn);
            }
        }
    }

    void ToggleMasterMode(bool bIsMaster) {
        if (bIsMaster) {
            pPort->CR = (pPort->CR & ~SPI_CR_MSR_SSN_MASK) | SPI_CR_MSR_SSN_BITS_ENABLE;
        } else {
            pPort->CR = (pPort->CR & ~SPI_CR_MSR_SSN_MASK) | SPI_CR_MSR_SSN_BITS_DISABLE;
        }
    }

    void Enable() {
        pPort->CR = (pPort->CR & ~SPI_CR_SPE_MASK) | SPI_CR_SPE_BITS_ENABLE;
    }

private:
    volatile SPI_Port_t *pPort;
};
