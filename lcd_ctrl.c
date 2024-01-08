// Leon Ozog 2023/2024
#include "lcd_ctrl.h"

#include "Open1768_LCD.h"
#include "LCD_ILI9325.h"
#include "asciiLib.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void lcd_init()
{
	lcdConfiguration();
	init_ILI9325();
}

void lcd_write_str(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2, const char *str)
{
	uint8_t c_buffer[16];
	while(*str)
	{
		GetASCIICode(ASCII_8X16_System, c_buffer, *str);
		uint8_t *c = c_buffer;
		for(uint8_t i = 0; i < 16; i++)
		{
			lcdWriteReg(ADRX_RAM, x);
			lcdWriteReg(ADRY_RAM, ++y);
			lcdWriteIndex(DATA_RAM);
			for(uint8_t j = 0; j < 8; j++)
			{
				if(*c>>(7-j) & 1)
					lcdWriteData(color1);
				else
					lcdWriteData(color2);
			}
			++c;
		}
		x += 8;
		y -= 16;
		str++;
	}
}

void lcd_write(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2, const char *format, ...)
{
	char str_buffer[64];
	va_list args;
  va_start(args, format);
	vsprintf(str_buffer, format, args);
	lcd_write_str(x, y, color1, color2, str_buffer);
	va_end(args);
}

void lcd_flush(uint16_t color)
{
	lcdWriteReg(ADRX_RAM, 0);
	lcdWriteReg(ADRY_RAM, 0);
	lcdWriteIndex(DATA_RAM);
	for (int i = 0; i < LCD_MAX_X * LCD_MAX_Y; i++)
		lcdWriteData(color);
}

int lcd_row(int n)
{
	return n * 16;
}

void lcd_img_start(lcd_img *img, uint16_t x, uint16_t y)
{
	img->start_x = x;
	img->start_y = y;
	img->px_x = 0;
	img->px_y = 0;
	lcdWriteReg(ADRX_RAM, x);
	lcdWriteReg(ADRY_RAM, y);
	lcdWriteIndex(DATA_RAM);
}

void lcd_img_row(lcd_img *img)
{
	img->px_x = 0;
	img->px_y++;
	lcdWriteReg(ADRX_RAM, img->start_x);
	lcdWriteReg(ADRY_RAM, img->start_y + img->px_y);
	lcdWriteIndex(DATA_RAM);
}

void lcd_img_pixel(lcd_img *img, uint16_t color)
{
	img->px_x++;
	lcdWriteData(color);
}