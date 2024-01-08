#include "ov7670_LPC1768.h"
#include "lcd_ctrl.h"

#include "Driver_USART.h"
extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

#include <stdio.h>
#include <string.h>

lcd_img img;

void callback_frame()
{
	ov7670* ov = ov_get_handle();
	lcd_img_start(&img, (LCD_W - ov->img_w)>>1, (LCD_H - ov->img_h)>>1);
}

void callback_row()
{
	lcd_img_row(&img);
}

void callback_pixel(uint16_t color)
{
	lcd_img_pixel(&img, color);
}

int main()
{
	/*USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
				  ARM_USART_DATA_BITS_8 |
				  ARM_USART_PARITY_NONE |
				  ARM_USART_STOP_BITS_1 |
				  ARM_USART_FLOW_CONTROL_NONE, 9600);
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
	USARTdrv->Send("\n\rSTART\n\r", 10);*/
	lcd_init();
	lcd_flush(LCD_blue);
	lcd_flush(LCD_black);
	
	ov_error err = ov_lpc1768_init(callback_frame, callback_row, callback_pixel);
	lcd_write(5, lcd_row(0), LCD_white, LCD_black, "init: %s", ov_error_what(err));
	
	while(1)
	{
		;
	}
}
