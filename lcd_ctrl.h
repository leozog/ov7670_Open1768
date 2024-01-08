// Leon Ozog 2023/2024
#pragma once
#include <stdint.h>

#define LCD_W 240
#define LCD_H 320

#define LCD_white           0xFFFF
#define LCD_black           0x0000
#define LCD_grey            0xa534
#define LCD_blue            0x001F
#define LCD_blueSea         0x05BF
#define LCD_pastelblue      0x051F
#define LCD_violet          0xB81F
#define LCD_magenta         0xF81F
#define LCD_red             0xF800
#define LCD_ginger          0xFAE0
#define LCD_green           0x07E0
#define LCD_cyan            0x7FFF
#define LCD_yellow          0xFFE0

void lcd_init();
void lcd_write_str(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2, const char *str);
void lcd_write(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2, const char *format, ...);
void lcd_flush(uint16_t color);
int lcd_row(int n);
int lcd_column(int n);
uint16_t lcd_rgb(uint8_t r, uint8_t g, uint8_t b);

typedef struct {
	uint16_t start_x;
	uint16_t start_y;
	uint16_t px_x;
	uint16_t px_y;
} lcd_img;
void lcd_img_start(lcd_img *img, uint16_t x, uint16_t y);
void lcd_img_row(lcd_img *img);
void lcd_img_pixel(lcd_img *img, uint16_t color);
