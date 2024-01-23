// Leon Ozog 2023/2024
#include "sender.h"
#include "uart.h"
#include "ov7670_LPC1768.h"
#include <stdlib.h>

sender s;

void sender_init(void (*callback_end)(void))
{
	uart_init(LPC_UART0);
	uart_send_strln(LPC_UART0, "ov_sender_start");
	s.callback_end = callback_end;
	s.state = SENDER_OFF;
}

void convert_565_to_rgb(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
{
	*r = color>>8 & 0b11111000;
	*g = color>>3 & 0b11111100;
	*b = color<<3 & 0b11111000;
}

void convert_rgb_to_565(uint16_t *color, uint8_t r, uint8_t g, uint8_t b)
{
	*color = r<<8 & 0b1111100000000000 | g<<3 & 0b11111100000 | b>>3 & 0b11111;
}

void sender_send_buffer()
{
	ov_lpc1768_stop();
	s.state = SENDER_SENDING;
	uart_send_strln(LPC_UART0, "row");
	uart_send_strln(LPC_UART0, "%d", s.pixel_x_last);
	uart_send_strln(LPC_UART0, "%d", s.pixel_y_last);
	uart_send_strln(LPC_UART0, "%d", s.buffer_l / 3);
	for(int i = 0; i < s.buffer_l; i += 3)
	{
		uint16_t color = s.buffer[i + 1]<<8 | s.buffer[i + 2];
		uint8_t r, g, b;
		convert_565_to_rgb(color, &r, &g, &b);
		uint8_t buf[4];
		buf[0] = s.buffer[i];
		buf[1] = r;
		buf[2] = g;
		buf[3] = b;
		uart_send_data(LPC_UART0, buf, 4);
	}
	s.buffer_l = 0;
	s.pixel_x_last = s.pixel_x;
	s.pixel_y_last = s.pixel_y;
	if(s.pixel_y_last >= s.img_h - 1)
	{
		s.callback_end();
		ov_lpc1768_start();
		s.state = SENDER_OFF;
	}
	else
	{
		s.state = SENDER_WAITING_FOR_FRAME;
		ov_lpc1768_start();
	}
}

void sender_callback_frame()
{
	if(s.state == SENDER_WAITING_FOR_FRAME)
	{
		s.pixel_x = 0;
		s.pixel_y = 0;
		s.buffer_l = 0;
		s.state = SENDER_RECORDING;
	}
}


void sender_callback_row()
{
	if(s.state == SENDER_WAITING_FOR_FRAME)
	{
		s.pixel_x = 0;
		s.pixel_y = 0;
		s.pixel_l = 0;
		s.state = SENDER_RECORDING;
	}
	if(s.state == SENDER_RECORDING)
	{
		s.pixel_x = 0;
		s.pixel_y += 1;
		s.pixel_l = 0;
	}
}

void sender_save_pixel()
{
	if(s.pixel_y < s.pixel_y_last || (s.pixel_y == s.pixel_y_last && s.pixel_x < s.pixel_x_last))
	{
		s.pixel_l = 0;
		return;
	}
	if(s.buffer_l < SENDER_BUFFER_SIZE - 3)
	{
		uint16_t r, g, b;
		r = s.pixel_r_sum / s.pixel_l;
		g = s.pixel_g_sum / s.pixel_l;
		b = s.pixel_b_sum / s.pixel_l;
		r = r > 255 ? 255 : r;
		g = g > 255 ? 255 : g;
		b = b > 255 ? 255 : b;
		s.buffer[s.buffer_l] = s.pixel_l;
		uint16_t color;
		convert_rgb_to_565(&color, r, g, b);
		s.buffer[s.buffer_l + 1] = color>>8;
		s.buffer[s.buffer_l + 2] = color & 0xFF;
		s.buffer_l += 3;
		s.pixel_l = 0;
	}
	if(s.buffer_l >= SENDER_BUFFER_SIZE - 3)
	{
		sender_send_buffer();
	}
}

void sender_callback_pixel(uint16_t color)
{
	if(s.state == SENDER_RECORDING)
	{
		uint8_t r, g, b;
		convert_565_to_rgb(color, &r, &g, &b);
		
		s.pixel_x++;
		if(s.pixel_l == 0)
		{
			s.pixel_l++;
			s.pixel_r_sum = r;
			s.pixel_g_sum = g;
			s.pixel_b_sum = b;
		}
		else if(
			(abs((int)(s.pixel_r_sum / (s.pixel_l)) - r) > SENDER_COMPRESSION_FACTOR) ||
			(abs((int)(s.pixel_g_sum / (s.pixel_l)) - g) > SENDER_COMPRESSION_FACTOR) ||
			(abs((int)(s.pixel_b_sum / (s.pixel_l)) - b) > SENDER_COMPRESSION_FACTOR)
		)
		{
			sender_save_pixel();
			s.pixel_l = 1;
			s.pixel_r_sum = r;
			s.pixel_g_sum = g;
			s.pixel_b_sum = b;
		}
		else
		{
			s.pixel_l++;
			s.pixel_r_sum += r;
			s.pixel_g_sum += g;
			s.pixel_b_sum += b;
		}
		if(s.pixel_x == s.img_w || s.pixel_l == SENDER_COMPRESSION_BLOCK_MAX)
		{
			sender_save_pixel();
		}
	}
}

void sender_send(const char* format)
{
	ov_lpc1768_register_callbacks(sender_callback_frame, sender_callback_row, sender_callback_pixel);
	volatile ov7670* ov = ov_lpc1768_get_handle();
	s.img_w = ov->img_w;
	s.img_h = ov->img_h;
	
	uart_send_strln(LPC_UART0, "img");
	uart_send_strln(LPC_UART0, "%s", format);
	uart_send_strln(LPC_UART0, "%d", s.img_w);
	uart_send_strln(LPC_UART0, "%d", s.img_h);
	
	s.pixel_x_last = 0;
	s.pixel_y_last = 0;
	
	s.state = SENDER_WAITING_FOR_FRAME;
	
	ov_lpc1768_start();
}

sender_state sender_get_state()
{
	return s.state;
}