// Leon Ozog 2023/2024
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
ov7670* ov_get_handle();