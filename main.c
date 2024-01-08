// Leon Ozog 2023/2024
#include "LPC17xx.h"
#include "Driver_USART.h"
extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART0;

#include "ov7670_LPC1768.h"
#include "lcd_ctrl.h"
#include "menu.h"

lcd_img img;
volatile menu options_menu;

/* img recording */
void callback_frame()
{
	volatile ov7670 *ov = ov_lpc1768_get_handle();
	lcd_img_start(&img, (LCD_W - ov->img_w + ov->img_skip_left + ov->img_skip_right)>>1, 
	(LCD_H - ov->img_h + ov->img_skip_up + ov->img_skip_down)>>1);
}

void callback_row()
{
	lcd_img_row(&img);
}

void callback_pixel(uint16_t color)
{
	lcd_img_pixel(&img, color);
}
/* img recording */

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


void EINT1_IRQHandler(void)
{
	if(menu_is_on(&options_menu))
	{
		menu_next_option(&options_menu);
	}
	else
	{
		/* take photo */
		/*NVIC_DisableIRQ(EINT0_IRQn);
		NVIC_DisableIRQ(EINT1_IRQn);
		volatile ov7670 *ov;
		ov_stop(ov);
		
		NVIC_EnableIRQ(EINT0_IRQn);
		NVIC_EnableIRQ(EINT1_IRQn);*/
	}
	LPC_SC->EXTINT = (1<<SBIT_EINT1); // Clear Interrupt Flag
}
/* button interupts */

void callback_options_menu_exit(void)
{
	lcd_flush(LCD_black);
	ov_lpc1768_start();
}

int main()
{
	/* disable interupts */
	NVIC_DisableIRQ(EINT0_IRQn);    
  NVIC_DisableIRQ(EINT1_IRQn);
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
	
	/* initialize camera */
	ov_error err = ov_lpc1768_init(callback_frame, callback_row, callback_pixel);
	lcd_write(5, lcd_row(0), LCD_white, LCD_black, "init: %s", ov_error_what(err));
	
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
	//volatile ov7670 *ov = ov_get_handle();
	//ov_start(ov);
	
	while(1)
	{
		;
	}
}
