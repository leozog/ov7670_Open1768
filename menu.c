// Leon Ozog 2023/2024
#include "menu.h"
#include "lcd_ctrl.h"

void menu_init(volatile menu *me, void (*callback_exit)(void))
{
	me->callback_exit = callback_exit;
	me->state = MENU_STATE_OFF;
	me->settings_n = 0;
	me->selected = -1;
}

uint8_t menu_is_on(volatile menu *me)
{
	return me->state;
}

volatile menu_setting *menu_add_setting(volatile menu *me, const char *name)
{
	menu_setting s;
	s.options_n = 0;
	s.selected = 0;
	s.name = name;
	me->settings[me->settings_n] = s;
	me->settings_n++;
	return &(me->settings[me->settings_n - 1]);
}

void menu_setting_add_option(volatile menu_setting *s, const char *name)
{
	s->option_names[s->options_n] = name;
	s->options_n++;
}

void menu_draw_clean(volatile menu *me, uint16_t x, uint16_t y)
{
	for(int i = 0; i < MENU_WIDTH; i++)
		lcd_write(x + lcd_column(i), y, MENU_COLOR_1, MENU_COLOR_2, " ");
}

void menu_draw_setting(volatile menu *me, uint16_t x, uint16_t y, uint8_t i, uint8_t selected)
{
	menu_draw_clean(me, x, y);
	menu_setting s = me->settings[i];
	if(selected)
		lcd_write(x, y, MENU_COLOR_2, MENU_COLOR_1, "> %s: %s ", s.name, s.option_names[s.selected]);
	else
		lcd_write(x, y, MENU_COLOR_1, MENU_COLOR_2, " %s: %s ", s.name, s.option_names[s.selected]);
}

void menu_draw_back(volatile menu *me, uint16_t x, uint16_t y, uint8_t selected)
{
	menu_draw_clean(me, x, y);
	if(selected)
		lcd_write(x, y, MENU_COLOR_2, MENU_COLOR_1, "> back ");
	else
		lcd_write(x, y, MENU_COLOR_1, MENU_COLOR_2, " back ");
}

void menu_draw(volatile menu *me, uint8_t selected)
{
	if(me->state == MENU_STATE_OFF)
	{
		me->state = MENU_STATE_ON;
		lcd_flush(MENU_COLOR_0);
		menu_draw_back(me, MENU_OFFSET, MENU_OFFSET + lcd_row(0), selected);
		for(int i = 0; i < me->settings_n; i++)
		{
			menu_draw_setting(me, MENU_OFFSET, MENU_OFFSET + lcd_row(i + 1), i, 0);
		}
	}
	else
	{
		if(me->selected == -1)
			menu_draw_back(me, MENU_OFFSET, MENU_OFFSET + lcd_row(0), selected);
		else
		{
			menu_draw_setting(me, MENU_OFFSET, MENU_OFFSET + lcd_row(me->selected + 1), me->selected, selected);
		}
	}
}

void menu_next_setting(volatile menu *me)
{
	menu_draw(me, 0);
	me->selected++;
	me->selected = me->selected == me->settings_n ? -1 : me->selected;
	menu_draw(me, 1);
}

void menu_next_option(volatile menu *me)
{
	if(me->selected == -1)
	{
		me->state = MENU_STATE_OFF;
		me->callback_exit();
	}
	else
	{
		volatile menu_setting *s = &(me->settings[me->selected]);
		s->selected++;
		s->selected = s->selected == s->options_n ? 0 : s->selected;
		menu_draw(me, 1);
	}
}

uint8_t menu_get_selected(volatile menu *me, uint8_t i)
{
	return me->settings[i].selected;
}