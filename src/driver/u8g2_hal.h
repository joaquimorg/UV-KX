#pragma once

#include "u8g2.h"
#include "U8g2lib.h"

#ifdef __cplusplus
extern "C" {
#endif

    uint8_t u8x8_gpio_and_delay_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);
    uint8_t u8x8_hw_spi_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

#ifdef __cplusplus
}
#endif


class ST7565 : public U8G2 {
public:
    ST7565() : U8G2() {
        u8g2_Setup_st7565_64128n_f(&u8g2, U8G2_R0, u8x8_hw_spi_cb, u8x8_gpio_and_delay_cb);
    }
};

