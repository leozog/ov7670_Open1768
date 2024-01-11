// Leon Ozog 2023/2024
#include "sender.h"
#include "ov7670_LPC1768.h"
#include <stdlib.h>

#include "Driver_USART.h"
extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

sender s;

void sender_init(void (*callback_end)(void))
{
	USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
				  ARM_USART_DATA_BITS_8 |
				  ARM_USART_PARITY_NONE |
				  ARM_USART_STOP_BITS_1 |
				  ARM_USART_FLOW_CONTROL_NONE, SENDER_UART_SPEED);
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
  USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
	//USARTdrv->Send("\n\rSTART\n\r", 10);
	s.callback_end = callback_end;
}

void sender_callback_frame()
{
	s.frame_n++;
	if(s.frame_n == 1)
	{
		USARTdrv->Send("go", 2);
		USARTdrv->Send("\n", 1);
	}
	else if(s.frame_n > 1)
	{
		s.callback_end();
	}
}

void sender_callback_row()
{
	s.pixel_x = 0;
	s.pixel_l = 0;
}

void color_to_rgb(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
{
	*r = color>>8 & 0b11111000;
	*g = color>>3 & 0b11111100;
	*b = color<<3 & 0b11111000;
}

void sender_send_pixel()
{
	uint16_t r, g, b;
	r = s.pixel_r_sum / s.pixel_l;
	g = s.pixel_g_sum / s.pixel_l;
	b = s.pixel_b_sum / s.pixel_l;
	r = r > 255 ? 255 : r;
	g = g > 255 ? 255 : g;
	b = b > 255 ? 255 : b;
	
	uint8_t buf[4];
	buf[0] = s.pixel_l;
	buf[1] = r;
	buf[2] = g;
	buf[3] = b;
	USARTdrv->Send(buf, 4);
	s.pixel_l = 0;
}

void sender_callback_pixel(uint16_t color)
{
	uint8_t r, g, b;
	color_to_rgb(color, &r, &g, &b);
	
	s.pixel_x++;
	if(s.pixel_l == 0)
	{
		s.pixel_l++;
		s.pixel_r_sum = r;
		s.pixel_g_sum = g;
		s.pixel_b_sum = b;
	}
	else if(
		(abs((int)(s.pixel_r_sum / (s.pixel_l - 1)) - r) > SENDER_COMPRESSION_FACTOR) ||
		(abs((int)(s.pixel_g_sum / (s.pixel_l - 1)) - g) > SENDER_COMPRESSION_FACTOR) ||
		(abs((int)(s.pixel_b_sum / (s.pixel_l - 1)) - b) > SENDER_COMPRESSION_FACTOR)
	)
	{
		sender_send_pixel();
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
		sender_send_pixel();
	}
}

void sender_send(const char* format)
{
	ov_lpc1768_register_callbacks(sender_callback_frame, sender_callback_row, sender_callback_pixel);
	volatile ov7670* ov = ov_lpc1768_get_handle();
	s.img_w = ov->img_w;
	s.img_h = ov->img_h;
	
	USARTdrv->Send("img", 3);
	USARTdrv->Send("\n", 1);
	USARTdrv->Send(format, 3);
	USARTdrv->Send("\n", 1);
	
	uint8_t buf[2];
	buf[0] = s.img_w>>8;
	buf[1] = s.img_w;
	USARTdrv->Send(buf, 2);
	USARTdrv->Send("\n", 1);
	buf[0] = s.img_h>>8;
	buf[1] = s.img_h;
	USARTdrv->Send(buf, 2);
	USARTdrv->Send("\n", 1);
	
	s.frame_n = 0;
	
	ov_lpc1768_start();
}