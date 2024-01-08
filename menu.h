// Leon Ozog 2023/2024
#pragma once
#include <stdint.h>

#define MENU_SETTINGS_MAX 4
#define MENU_OPTIONS_MAX 4
#define MENU_WIDTH 28
#define MENU_OFFSET 8

#define MENU_COLOR_0 0x0000
#define MENU_COLOR_1 0xFFFF
#define MENU_COLOR_2 0x8410


typedef uint8_t menu_state;
#define MENU_STATE_OFF 0
#define MENU_STATE_ON 1

typedef struct {
	uint8_t options_n;
	uint8_t selected;
	const char *name;
	const char *option_names[MENU_OPTIONS_MAX];
} menu_setting;

typedef struct {
	void (*callback_exit)(void);
	menu_state state;
	uint8_t settings_n;
	int8_t selected;
	menu_setting settings[MENU_SETTINGS_MAX];
} menu;

void menu_init(volatile menu *me, void (*callback_exit)(void));
uint8_t menu_is_on(volatile menu *me);

volatile menu_setting *menu_add_setting(volatile menu *me, const char *name);
void menu_setting_add_option(volatile menu_setting *s, const char *name);

void menu_draw_clean(volatile menu *me, uint16_t x, uint16_t y);
void menu_draw_setting(volatile menu *me, uint16_t x, uint16_t y, uint8_t i, uint8_t selected);
void menu_draw_back(volatile menu *me, uint16_t x, uint16_t y, uint8_t selected);

void menu_draw(volatile menu *me, uint8_t selected);
void menu_next_setting(volatile menu *me);
void menu_next_option(volatile menu *me);

uint8_t menu_get_selected(volatile menu *me, uint8_t i);