#pragma once

#include <cstdint>
#include <cstring>

#include "dma.h"
#include "syscon.h"
#include "uart.h"
#include "printf.h"
#include "sys.h"

extern uint8_t UART_DMA_Buffer[256];
class UART {
private:
    static constexpr uint8_t Obfuscation[16] = {
        0x16, 0x6C, 0x14, 0xE6, 0x2E, 0x91, 0x0D, 0x40,
        0x21, 0x35, 0xD5, 0x40, 0x13, 0x03, 0xE9, 0x80
    };

    static constexpr size_t BufferSize = 256;

    // Data Structures
    struct Header_t {
        uint16_t id;
        uint16_t size;
    };

    struct Footer_t {
        uint8_t padding[2];
        uint16_t id;
    };

    union CommandBuffer_t {
        uint8_t buffer[256];
        struct {
            Header_t header;
            uint8_t data[256 - sizeof(Header_t)];
        } command;
    };

    // Variables
    CommandBuffer_t commandBuffer;
    uint32_t timestamp;
    uint16_t writeIndex;
    bool isEncrypted;
    bool sendScreenData = false;

public:
    UART() : isEncrypted(false) {
        memset(UART_DMA_Buffer, 0, BufferSize);
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
        }
        else {
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
        DMA_CH0->MDADDR = (uint32_t)(uintptr_t)UART_DMA_Buffer;
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


private:

    void sendReply(void* pReply, uint16_t size) {
        Header_t header;
        Footer_t footer;

        // Encrypt the reply data if encryption is enabled
        if (isEncrypted) {
            uint8_t* bytes = static_cast<uint8_t*>(pReply);
            for (uint16_t i = 0; i < size; i++) {
                bytes[i] ^= Obfuscation[i % sizeof(Obfuscation)];
            }
        }

        // Prepare the header
        header.id = 0xCDAB;
        header.size = size;

        // Send the header
        send(&header, sizeof(header));

        // Send the reply data
        send(pReply, size);

        // Prepare the footer
        if (isEncrypted) {
            footer.padding[0] = Obfuscation[size % sizeof(Obfuscation)] ^ 0xFF;
            footer.padding[1] = Obfuscation[(size + 1) % sizeof(Obfuscation)] ^ 0xFF;
        }
        else {
            footer.padding[0] = 0xFF;
            footer.padding[1] = 0xFF;
        }
        footer.id = 0xBADC;

        // Send the footer
        send(&footer, sizeof(footer));
    }

    void sendVersion() {
        // Prepare the reply structure
        struct {
            Header_t header;
            struct {
                char Version[16];
                bool bHasCustomAesKey;
                bool bIsInLockScreen;
                uint8_t Padding[2];
                uint32_t Challenge[4];
            } Data;
        } reply;

        // Fill in the reply data
        reply.header.id = 0x0515;
        reply.header.size = sizeof(reply.Data);
        memcpy(reply.Data.Version, AUTHOR_NAME " " VERSION_STRING, sizeof(reply.Data.Version));
        reply.Data.bHasCustomAesKey = false;
        reply.Data.bIsInLockScreen = false;
        reply.Data.Challenge[0] = 0xFFFFFFFF;
        reply.Data.Challenge[1] = 0xFFFFFFFF;
        reply.Data.Challenge[2] = 0xFFFFFFFF;
        reply.Data.Challenge[3] = 0xFFFFFFFF;

        // Send the reply
        sendReply(&reply, sizeof(reply));
    }

    /*bool isBadChallenge(const uint32_t* key, const uint32_t* challenge, const uint32_t* response) {
        uint32_t iv[4] = { 0 };

        AESEncrypt(key, iv, challenge, iv, true);

        for (size_t i = 0; i < 4; i++) {
            if (iv[i] != response[i]) {
                return true;
            }
        }
        return false;
    }*/

    void handleCmd0514(const uint8_t* pBuffer) {
        // Define the structure of the incoming command
        struct CMD_0514_t {
            Header_t Header;
            uint32_t Timestamp;
        };

        // Cast the buffer to the command structure
        const CMD_0514_t* pCmd = reinterpret_cast<const CMD_0514_t*>(pBuffer);

        // Update the session timestamp
        timestamp = pCmd->Timestamp;

        // Send the version response
        sendVersion();
    }


    void decryptCommand(uint8_t* buffer, uint16_t size) {
        for (uint16_t i = 0; i < size; i++) {
            buffer[i] ^= Obfuscation[i % sizeof(Obfuscation)];
        }
    }

    bool checkCRC(uint8_t* buffer, uint16_t size, uint16_t expectedCRC) {
        // Compare calculated CRC with expected CRC
        uint16_t crc = CRCCalculate(buffer, size);
        //print("CRC: %d = %d\n", crc, expectedCRC);
        return (crc == expectedCRC);
    }

    void sendScreen() {
        sendScreenData = true;
    }


public:

    void sendScreenBuffer(const void* buffer, uint32_t size) {
        const uint16_t screenDumpIdByte = 0xEDAB;
        if (sendScreenData) {
            send(&screenDumpIdByte, 2);
            send(buffer, size);
        }
    }

    bool isCommandAvailable() {
        uint16_t index;
        uint16_t tailIndex;
        uint16_t size;
        uint16_t crc;
        uint16_t dmaLength = DMA_CH0->ST & 0xFFFU;

        while (true) {
            // Check if DMA buffer is empty
            if (writeIndex == dmaLength) {
                return false;
            }

            // Advance to the next potential command start
            while (writeIndex != dmaLength && UART_DMA_Buffer[writeIndex] != 0xABU) {
                writeIndex = (writeIndex + 1) % BufferSize;
            }

            if (writeIndex == dmaLength) {
                return false;
            }

            // Calculate command length
            index = (writeIndex + 2) % BufferSize;
            size = (UART_DMA_Buffer[index + 1] << 8) | UART_DMA_Buffer[index];

            // Check if command length exceeds buffer size
            if ((size + 8u) > BufferSize) {
                writeIndex = dmaLength; // Skip this invalid command
                return false;
            }

            // Check if there is enough data in the buffer for a complete command
            uint16_t availableData = (uint16_t)((writeIndex < dmaLength) ? (dmaLength - writeIndex) :
                (BufferSize - writeIndex + dmaLength));
            if (availableData < size + 8) {
                return false;
            }

            // Verify the footer
            index = (index + 2) % BufferSize;
            tailIndex = (index + size + 2) % BufferSize;
            //print("tailIndex: %d\n", UART_DMA_Buffer[tailIndex]);
            if (UART_DMA_Buffer[tailIndex] != 0xDC || UART_DMA_Buffer[(tailIndex + 1) % BufferSize] != 0xBA) {
                writeIndex = dmaLength; // Skip this invalid command
                return false;
            }

            // Copy command into the class buffer
            if (tailIndex < index) {
                uint16_t chunkSize = BufferSize - index;
                memcpy(commandBuffer.buffer, UART_DMA_Buffer + index, chunkSize);
                memcpy(commandBuffer.buffer + chunkSize, UART_DMA_Buffer, tailIndex);
            }
            else {
                memcpy(commandBuffer.buffer, UART_DMA_Buffer + index, tailIndex - index);
            }

            // Zero out the processed portion of the buffer
            uint16_t processedEnd = (tailIndex + 2) % BufferSize;
            if (processedEnd < writeIndex) {
                memset(UART_DMA_Buffer + writeIndex, 0, BufferSize - writeIndex);
                memset(UART_DMA_Buffer, 0, processedEnd);
            }
            else {
                memset(UART_DMA_Buffer + writeIndex, 0, processedEnd - writeIndex);
            }
            writeIndex = processedEnd;

            if (commandBuffer.command.header.id == 0x0514)
                isEncrypted = false;

            if (commandBuffer.command.header.id == 0x6902)
                isEncrypted = true;

            // Decrypt if necessary
            if (isEncrypted) {
                decryptCommand(commandBuffer.buffer, size + 2);
            }

            // Validate CRC            
            crc = (commandBuffer.buffer[size] | (commandBuffer.buffer[size + 1] << 8));

            if (!checkCRC(commandBuffer.buffer, size, crc)) {
                continue; // Invalid CRC, skip to the next potential command
            }

            return true; // Valid command is available
        }
    }

    void handleCommand() {
        switch (commandBuffer.command.header.id) {
        case 0x0514:
            handleCmd0514(commandBuffer.command.data);
            break;

        case 0x0527:
            //handleRssiRead();
            break;

        case 0x0529:
            //handleAdcRead();
            break;

        case 0x05DD: // Reset command
            //handleReset();
            break;
        case 0x0A03:
            sendScreenData = true;
            break;
        case 0x0A04:
            sendScreenData = false;
            break;            
        }

    }

};
