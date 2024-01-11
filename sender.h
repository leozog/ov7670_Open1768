// Leon Ozog 2023/2024
#pragma once
#include <stdint.h>

#define SENDER_COMPRESSION_FACTOR 32
#define SENDER_COMPRESSION_BLOCK_MAX 255
#define SENDER_UART_SPEED 115200

typedef struct {
	void (*callback_end)(void);
	
	volatile uint8_t frame_n;
	
	uint16_t img_w;
	uint16_t img_h;
	
	uint16_t pixel_x;
	uint8_t pixel_l;
	uint16_t pixel_r_sum;
	uint16_t pixel_g_sum;
	uint16_t pixel_b_sum;
} sender;

void sender_init(void (*callback_end)(void));
void sender_send(const char* format);