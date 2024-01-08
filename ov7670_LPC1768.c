// Leon Ozog 2023/2024
#include "ov7670_LPC1768.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "Driver_I2C.h" 

volatile ov7670 ov;

extern ARM_DRIVER_I2C            Driver_I2C0; //P0.27, P0.28
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C0;

int32_t ov_lpc1768_i2c_get_reg(uint8_t addr, uint8_t *buf, uint32_t len)
{
  uint8_t a[1];
  a[0] = addr;
  I2Cdrv->MasterTransmit (OV_I2C_ADDR, a, 1, false);
 
  // Wait until transfer completed 
  while (I2Cdrv->GetStatus().busy);
  // Check if all data transferred 
  if (I2Cdrv->GetDataCount () != 1) return 1;
 
  if (I2Cdrv->MasterReceive (OV_I2C_ADDR, buf, len, false) != 0) return 2;
  // Wait until transfer completed 
  while (I2Cdrv->GetStatus().busy);
  // Check if all data transferred 
  if (I2Cdrv->GetDataCount() != len) return 3;
	
  return 0;
}

int32_t ov_lpc1768_i2c_set_reg(uint8_t addr, uint8_t *buf, uint32_t len)
{
  uint8_t a[17];
  a[0] = addr;
	for(int i = 0; i < len; i++)
		a[1 + i] = buf[i];
  I2Cdrv->MasterTransmit (OV_I2C_ADDR, a, 1 + len, false);
 
  // Wait until transfer completed 
  while (I2Cdrv->GetStatus().busy);
  // Check if all data transferred 
  if (I2Cdrv->GetDataCount () != 1 + len) return 1;
	
  return 0;
}

void wait_for_pclk()
{
	volatile int i = 0;
	volatile uint8_t pclk = 0;
	do
	{
		pclk = LPC_GPIO0->FIOPIN>>18 & 0x1;
		i++;
	} while(pclk == 1); // wait for low
	do
	{
		pclk = LPC_GPIO0->FIOPIN>>18 & 0x1;
		i++;
	} while(pclk == 0); // wait for high
}

volatile uint8_t read_data()
{
	return (((LPC_GPIO1->FIOPIN>>4) & 0b00000001) | ((LPC_GPIO1->FIOPIN>>7) & 0b00001110) | ((LPC_GPIO1->FIOPIN>>10) & 0b11110000));
}

#define P0_16_IRQ (1<<16)
#define P0_17_IRQ (1<<17)

extern void EINT3_IRQHandler(void)
{  
	/* raising P0_16_IRQ */
	if( LPC_GPIOINT->IO0IntStatR & P0_16_IRQ)
	{
		LPC_GPIOINT->IO0IntClr = P0_16_IRQ; // Clear the interrupt.
		ov_vsync(&ov);
	}
		
	/* raising P0_17_IRQ */
	if( LPC_GPIOINT->IO0IntStatR & P0_17_IRQ)
	{
		LPC_GPIOINT->IO0IntClr = P0_17_IRQ; // Clear the interrupt.
		ov_href_up(&ov);
	}
	
	/* falling P0_17_IRQ */
	/*if( LPC_GPIOINT->IO0IntStatF & P0_17_IRQ)
	{
		LPC_GPIOINT->IO0IntClr = P0_17_IRQ; // Clear the interrupt.
		ov_href_down(&ov);
	}*/
}

ov_error ov_lpc1768_init(void (*callback_frame)(), void (*callback_row)(), void (*callback_pixel)(uint16_t))
{
	/* disable interupts */
	NVIC_DisableIRQ(EINT3_IRQn);
	
	
	/* clkout on 1.27 */
	LPC_PINCON->PINSEL3 &=~(3<<22);
	LPC_PINCON->PINSEL3 |= (1<<22);
	LPC_SC->CLKOUTCFG = (1<<8)|(3<<4); //enable and divide by 4 to get 25MHz
	
	
	/* initialize I2C */
	I2Cdrv->Initialize (NULL);
	I2Cdrv->PowerControl (ARM_POWER_FULL);
	I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
	
	/* register i2c functions */
	ov.i2c_get_reg = ov_lpc1768_i2c_get_reg;
	ov.i2c_set_reg = ov_lpc1768_i2c_set_reg;

	/* check if i2c works */
	uint8_t check[1];
	ov.i2c_get_reg(0x0A, check, 1);
	if(*check != 0x76)
		return OV_ERROR_I2C_FAIL;
	
	/* register data read functions */
	ov.wait_for_pclk = wait_for_pclk;
	ov.read_data = read_data;
	
	/* register callbacks */
	ov.callback_frame = callback_frame;
	ov.callback_row = callback_row;
	ov.callback_pixel = callback_pixel;
	
	
	/* init structure */
	ov_init(&ov);
	
	/* set resolution to LIVE */
	ov_set_res(&ov, OV_RES_LIVE);
	
	
	/* init GPIO */
	/* P0.16 as vsync interupt */
	LPC_PINCON->PINSEL1 &= ~(0x3 << 0);  // mode 00 GPIO
	LPC_PINCON->PINMODE1 |= (0x3 << 0);  // pull-down
	LPC_GPIO0->FIODIR   &= ~(0x1 << 16); // input
	LPC_GPIOINT->IO0IntEnR |= P0_16_IRQ; // interupt enable raising
	
	/* P0.17 as href interupt */
	LPC_PINCON->PINSEL1 &= ~(0x3 << 2);  // mode 00 GPIO
	LPC_PINCON->PINMODE1 |= (0x3 << 2);  // pull-down
	LPC_GPIO0->FIODIR   &= ~(0x1 << 17); // input
	LPC_GPIOINT->IO0IntEnR |= P0_17_IRQ; // interupt enable raising
	//LPC_GPIOINT->IO0IntEnF |= P0_17_IRQ; // interupt enable faling
	
	/* P0.18 as pclk */
	LPC_PINCON->PINSEL1 &= ~(0x3 << 4);  // mode 00 GPIO
	LPC_PINCON->PINMODE1 |= (0x3 << 4);  // pull-down
	LPC_GPIO0->FIODIR   &= ~(0x1 << 18); // input
	
	/* P1.4, P1.8, P1.9, P1.10, P1.14, P1.15, P1.16, P1.17 as data */
	PIN_Configure(1, 4, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 8, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 9, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 10, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 14, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 15, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 16, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	PIN_Configure(1, 17, PIN_FUNC_0, PIN_PINMODE_REPEATER, PIN_PINMODE_NORMAL);
	
	
	/* reset interupts */
	LPC_SC->EXTINT = 0xF;
	
	/* enable interupts */
	NVIC_EnableIRQ(EINT3_IRQn);
	
	return OV_A_OKAY;
}

volatile ov7670* ov_lpc1768_get_handle()
{
	return &ov;
}

void ov_lpc1768_start()
{
	LPC_GPIOINT->IO0IntClr = P0_16_IRQ; // Clear the interrupts.
	LPC_GPIOINT->IO0IntClr = P0_17_IRQ; //
	NVIC_EnableIRQ(EINT3_IRQn);
	ov_start(&ov);
}

void ov_lpc1768_stop()
{
	ov_stop(&ov);
	NVIC_DisableIRQ(EINT3_IRQn);
	LPC_GPIOINT->IO0IntClr = P0_16_IRQ; // Clear the interrupts.
	LPC_GPIOINT->IO0IntClr = P0_17_IRQ; //
}
