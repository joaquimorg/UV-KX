#pragma once

#include <cstdint>

#include "dma.h"
#include "syscon.h"
#include "uart.h"
#include "printf.h"

class UART {
private:
    uint8_t DMA_Buffer[256];

public:
    UART() {
        // Constructor initializes UART
        init();
        print("\n\n");
    }

    void init() {
        uint32_t Delta;
        uint32_t Positive;
        uint32_t Frequency;

        UART1->CTRL = (UART1->CTRL & ~UART_CTRL_UARTEN_MASK) | UART_CTRL_UARTEN_BITS_DISABLE;
        Delta = SYSCON_RC_FREQ_DELTA;
        Positive = (Delta & SYSCON_RC_FREQ_DELTA_RCHF_SIG_MASK) >> SYSCON_RC_FREQ_DELTA_RCHF_SIG_SHIFT;
        Frequency = (Delta & SYSCON_RC_FREQ_DELTA_RCHF_DELTA_MASK) >> SYSCON_RC_FREQ_DELTA_RCHF_DELTA_SHIFT;

        if (Positive) {
            Frequency += 48000000U;
        } else {
            Frequency = 48000000U - Frequency;
        }

        // 48M, the baud rate is set to 115200, then UARTDIV=48000000/115200=416.6, 417 can be selected based on rounding.
        UART1->BAUD = Frequency / 115200;

        UART1->CTRL = UART_CTRL_RXEN_BITS_ENABLE | UART_CTRL_TXEN_BITS_ENABLE | UART_CTRL_RXDMAEN_BITS_ENABLE;
        UART1->RXTO = 4;
        UART1->FC = 0;
        UART1->FIFO = UART_FIFO_RF_LEVEL_BITS_8_BYTE | UART_FIFO_RF_CLR_BITS_ENABLE | UART_FIFO_TF_CLR_BITS_ENABLE;
        UART1->IE = UART_IE_RXFIFO_BITS_ENABLE | UART_IE_RXTO_BITS_ENABLE;

        DMA_CTR = (DMA_CTR & ~DMA_CTR_DMAEN_MASK) | DMA_CTR_DMAEN_BITS_DISABLE;

        DMA_CH0->MSADDR = (uint32_t)(uintptr_t)&UART1->RDR;
        DMA_CH0->MDADDR = (uint32_t)(uintptr_t)DMA_Buffer;
        DMA_CH0->MOD = 0
            | DMA_CH_MOD_MS_ADDMOD_BITS_NONE
            | DMA_CH_MOD_MS_SIZE_BITS_8BIT
            | DMA_CH_MOD_MS_SEL_BITS_HSREQ_MS1
            | DMA_CH_MOD_MD_ADDMOD_BITS_INCREMENT
            | DMA_CH_MOD_MD_SIZE_BITS_8BIT
            | DMA_CH_MOD_MD_SEL_BITS_SRAM;

        DMA_INTEN = 0;
        DMA_INTST = 0
            | DMA_INTST_CH0_TC_INTST_BITS_SET
            | DMA_INTST_CH1_TC_INTST_BITS_SET
            | DMA_INTST_CH2_TC_INTST_BITS_SET
            | DMA_INTST_CH3_TC_INTST_BITS_SET
            | DMA_INTST_CH0_THC_INTST_BITS_SET
            | DMA_INTST_CH1_THC_INTST_BITS_SET
            | DMA_INTST_CH2_THC_INTST_BITS_SET
            | DMA_INTST_CH3_THC_INTST_BITS_SET;

        DMA_CH0->CTR = 0
            | DMA_CH_CTR_CH_EN_BITS_ENABLE
            | ((0xFF << DMA_CH_CTR_LENGTH_SHIFT) & DMA_CH_CTR_LENGTH_MASK)
            | DMA_CH_CTR_LOOP_BITS_ENABLE
            | DMA_CH_CTR_PRI_BITS_MEDIUM;

        UART1->IF = UART_IF_RXTO_BITS_SET;

        DMA_CTR = (DMA_CTR & ~DMA_CTR_DMAEN_MASK) | DMA_CTR_DMAEN_BITS_ENABLE;

        UART1->CTRL |= UART_CTRL_UARTEN_BITS_ENABLE;
    }

    void send(const void* buffer, uint32_t size) {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer);

        for (uint32_t i = 0; i < size; ++i) {
            UART1->TDR = data[i];
            while ((UART1->IF & UART_IF_TXFIFO_FULL_MASK) != UART_IF_TXFIFO_FULL_BITS_NOT_SET) {
            }
        }
    }

    void print(const char* format, ...) {
        char text[128];
        va_list args;
        uint32_t len;

        va_start(args, format);
        len = (uint32_t)vsnprintf(text, sizeof(text), format, args);
        va_end(args);

        send(text, len);
    }

    void sendLog(const char* message) {
        // Prepend log marker and send the message
        const char* logPrefix = "[UV-Kx LOG] ";
        print("%s%s\n", logPrefix, message);
    }
};
