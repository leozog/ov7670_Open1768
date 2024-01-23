// Leon Ozog 2023/2024
#pragma once
#include "ov7670.h"

// P0.27, P0.28 I2C 
// P1.27 clkout
// P0.16 vsync interupt
// P0.17 href interupt
// P0.18 pclk
// P1.4, P1.8, P1.9, P1.10, P1.14, P1.15, P1.16, P1.17 as data
// where P1.4 is D0

// 3.3V reset
// GND PWDN

ov_error ov_lpc1768_init(void (*callback_new_frame)(), void (*callback_row_ready)(), void (*callback_pixel)(uint16_t));
void ov_lpc1768_register_callbacks(void (*callback_frame)(), void (*callback_row)(), void (*callback_pixel)(uint16_t));
volatile ov7670* ov_lpc1768_get_handle(void);
void ov_lpc1768_start(void);
void ov_lpc1768_stop(void);
void ov_lpc1768_set_res(ov_res res);
