// Leon Ozog 2023/2024
#pragma once
#include <stdint.h>

#define SENDER_COMPRESSION_FACTOR 16
#define SENDER_COMPRESSION_BLOCK_MAX 255
#define SENDER_BUFFER_SIZE 30000

typedef uint8_t sender_state;
#define SENDER_OFF 0
#define SENDER_WAITING_FOR_FRAME 1
#define SENDER_RECORDING 2
#define SENDER_SENDING 3

typedef struct {
	void (*callback_end)(void);
	
	volatile sender_state state;
	
	uint16_t img_w;
	uint16_t img_h;
	
	uint16_t pixel_x;
	uint16_t pixel_y;
	uint8_t pixel_l;
	uint16_t pixel_r_sum;
	uint16_t pixel_g_sum;
	uint16_t pixel_b_sum;
	
	uint32_t buffer_l;
	uint16_t pixel_x_last;
	uint16_t pixel_y_last;
  uint8_t buffer[SENDER_BUFFER_SIZE];
} sender;

void sender_init(void (*callback_end)(void));
void sender_send(const char* format);
sender_state sender_get_state();