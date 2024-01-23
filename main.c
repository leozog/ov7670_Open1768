// Leon Ozog 2023/2024
#include "LPC17xx.h"

#include "ov7670.h"
#include "ov7670_LPC1768.h"
#include "lcd_ctrl.h"
#include "menu.h"
#include "sender.h"

lcd_img img;
volatile menu options_menu;

/* display callbacks */
void display_callback_frame()
{
	volatile ov7670 *ov = ov_lpc1768_get_handle();
	lcd_img_start(&img, (LCD_W - ov->img_w + ov->img_skip_left + ov->img_skip_right)>>1, 
	(LCD_H - ov->img_h + ov->img_skip_up + ov->img_skip_down)>>1);
}

void display_callback_row()
{
	lcd_img_row(&img);
}

void display_callback_pixel(uint16_t color)
{
	lcd_img_pixel(&img, color);
}
/* display callbacks */

/* button interupts */
#define PINSEL_EINT0    20
#define PINSEL_EINT1    22
#define SBIT_EINT0      0
#define SBIT_EINT1      1  
#define SBIT_EXTMODE0   0
#define SBIT_EXTMODE1   1
#define SBIT_EXTPOLAR0  0
#define SBIT_EXTPOLAR1  1

void EINT0_IRQHandler(void)
{
	if(menu_is_on(&options_menu))
	{
		menu_next_setting(&options_menu);
	}
	else
	{
		ov_lpc1768_stop();
		menu_draw(&options_menu, 1);
	}
	LPC_SC->EXTINT = (1<<SBIT_EINT0); // Clear Interrupt Flag
}

volatile int eint1_req = 0;

void EINT1_IRQHandler(void)
{
	if(menu_is_on(&options_menu))
	{
		menu_next_option(&options_menu);
	}
	else
	{
		if(!sender_get_state())
		{
			
			/* take photo */
			ov_lpc1768_stop();
			eint1_req = 1; // cant use i2c in interrupt so i need to do a hacky solution
		}
	}
	LPC_SC->EXTINT = (1<<SBIT_EINT1); // Clear Interrupt Flag
}
/* button interupts */

void callback_options_menu_exit(void)
{
	lcd_flush(LCD_black);
	ov_lpc1768_start();
}

void sender_callback_end()
{
	eint1_req = 2;
}

int main()
{
	/* disable interupts */
	NVIC_DisableIRQ(EINT0_IRQn);    
  NVIC_DisableIRQ(EINT1_IRQn);
	
	lcd_init();
	lcd_flush(LCD_blue);
	lcd_flush(LCD_black);
	
	/* initialize camera */
	ov_error err = ov_lpc1768_init(display_callback_frame, display_callback_row, display_callback_pixel);
	lcd_write(5, lcd_row(0), LCD_white, LCD_black, "init: %s", ov_error_what(err));
	
	/* initialize image sender */
	sender_init(sender_callback_end);
	
	/* initialize options menu */
	menu_init(&options_menu, callback_options_menu_exit);
	{
		volatile menu_setting* res = menu_add_setting(&options_menu, "resolution");
		menu_setting_add_option(res, "VGA 640x480");
		menu_setting_add_option(res, "QVGA 320x240");
		menu_setting_add_option(res, "CIF 352x288");
		menu_setting_add_option(res, "QCIF 176x144");
		volatile menu_setting* img_fromat = menu_add_setting(&options_menu, "image format");
		menu_setting_add_option(img_fromat, "jpg");
		menu_setting_add_option(img_fromat, "png");
		menu_setting_add_option(img_fromat, "bmp");
		volatile menu_setting* exp = menu_add_setting(&options_menu, "exposure");
		menu_setting_add_option(exp, "auto");
		menu_setting_add_option(exp, "1.0");
	}
	
	
	
	/* register button interupts */
  LPC_PINCON->PINSEL4 |= (1<<PINSEL_EINT0)  | (1<<PINSEL_EINT1);    // Configure P2_10, P2_11 as EINT0, EINT1
  LPC_SC->EXTMODE     |= (1<<SBIT_EXTMODE0) | (1<<SBIT_EXTMODE1);   // Configure EINT0, EINT1 as edge-sensitive
	LPC_SC->EXTPOLAR    |= (1<<SBIT_EXTPOLAR0) | (1<<SBIT_EXTPOLAR1); // Configure EINT0, EINT1 as falling-edge triggered
	LPC_SC->EXTINT      = (1<<SBIT_EINT0)    | (1<<SBIT_EINT1);	      // Clear Pending interrupts
  
	/* enable interrupts */
	NVIC_EnableIRQ(EINT0_IRQn);	
  NVIC_EnableIRQ(EINT1_IRQn);
	
	
	/* start cammera */
	ov_lpc1768_start();
	
	int iter = 0;
	while(1)
	{
		if(++iter % 1000 && eint1_req)
		{
			if(eint1_req == 1)
			{
				NVIC_DisableIRQ(EINT0_IRQn);
				NVIC_DisableIRQ(EINT1_IRQn);
				lcd_flush(LCD_white);
				
				uint8_t m_res = menu_get_selected(&options_menu, 0);
				switch(m_res)
				{
					case 0: ov_lpc1768_set_res(OV_RES_VGA);  break;
					case 1: ov_lpc1768_set_res(OV_RES_QVGA); break;
					case 2: ov_lpc1768_set_res(OV_RES_CIF);  break;
					case 3: ov_lpc1768_set_res(OV_RES_QCIF); break;
				}
				
				uint8_t m_for = menu_get_selected(&options_menu, 1);
				
				switch(m_for)
				{
					case 0: sender_send("jpg"); break;
					case 1: sender_send("png"); break;
					case 2: sender_send("bmp"); break;
				}
			}
			else if(eint1_req == 2)
			{
				lcd_flush(LCD_black);
				ov_lpc1768_set_res(OV_RES_LIVE);
				ov_lpc1768_register_callbacks(display_callback_frame, display_callback_row, display_callback_pixel);
				
				LPC_SC->EXTINT = (1<<SBIT_EINT1); // Clear Interrupt Flag
				NVIC_EnableIRQ(EINT0_IRQn);
				NVIC_EnableIRQ(EINT1_IRQn);
				
				ov_lpc1768_start();
			}
			eint1_req = 0;
		}
	}
}
