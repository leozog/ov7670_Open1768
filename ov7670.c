// Leon Ozog 2023/2024
#include "ov7670.h"

char* ov_error_what(ov_error err)
{
	switch(err)
	{
		case OV_A_OKAY: return "OV_A_OKAY";
		case OV_ERROR_I2C_FAIL: return "OV_ERROR_I2C_FAIL";
	}
	return "OV_ERROR_UNKNOWN";
}

void ov_init(ov7670 *ov)
{
	/* init internal variables */
	ov->ready = 3; // wait for this many frames before starting
	ov->img_w = 0;
	ov->img_h = 0;
	ov->px_x = 0;
	ov->px_y = 0;
	
	/* reset camera */
	ov_i2c_set(ov, OV_REG_COM7, 0x80);
	ov_sleep(1000000); // need to wait for about 1ms
	
	/* set internal clock to (clkout / 14) */
	ov_i2c_set(ov, OV_REG_CLKRC, 0x0B);
	
	
	/* set color output to 555 */
	ov_i2c_set(ov, OV_REG_COM7, 0b100);
	ov_i2c_set(ov, OV_REG_COM15, 0xF0);
	
	// gain
	//ov_i2c_set(ov, OV_REG_COM9, 0x08);
	
	// UV saturation auto
	//ov_i2c_set(ov, OV_REG_COM13, 0xC8);
	
	//ov_i2c_set(ov, OV_REG_COM16, 0x12);
	//ov_i2c_set(ov, OV_REG_SATCTR, 0xF0);
}

uint8_t ov_i2c_get(ov7670 *ov, uint8_t addr)
{
	uint8_t buf[1];
	while(ov->i2c_get_reg(addr, buf, 1));
	return *buf;
}

void ov_i2c_set(ov7670 *ov, uint8_t addr, uint8_t val)
{
	uint8_t buf[1];
	*buf = val;
	while(ov->i2c_set_reg(addr, buf, 1));
}

void ov_i2c_bit(ov7670 *ov, uint8_t addr, uint8_t mask, uint8_t val)
{
	uint8_t buf = ov_i2c_get(ov, addr);
	buf &= ~mask;
	buf |= mask & val;
	ov_i2c_set(ov, addr, buf);
}

void ov_sleep(uint32_t iter)
{
	volatile uint32_t i;
  for (i = iter; i > 0; i--) {;}
}

void ov_vsync(ov7670 *ov)
{
	if(ov->ready > 0) --ov->ready;
	
	ov->px_y = 0;
	
	ov->callback_frame();
}

void ov_href_up(ov7670 *ov)
{
	if(ov->ready == 0 && ov->px_y < ov->img_h)
	{
		ov->px_x = 0;
		ov->wait_for_pclk();
		for(int i = 0; i < ov->img_w; i++)
		{
			ov->wait_for_pclk();
			uint16_t val2 = ov->read_data();
			ov->wait_for_pclk();
			uint16_t val1 = ov->read_data();
			uint16_t r = (val1 & 0b00000011)<<3 | (val2 & 0b11100000)>>5;
			uint16_t g = (val1 & 0b01111100)>>2;
			uint16_t b = (val2 & 0b00011111);
			
			ov->callback_pixel(r<<11 | g<<6 | b);
		}
		ov->callback_row();
	}
}

void ov_href_down(ov7670 *ov)
{
	
}

void ov_set_res(ov7670 *ov, ov_res res)
{
	if(res == OV_RES_LIVE)
	{
		ov->img_w = 280;
		ov->img_h = 288;
		ov_i2c_set(ov, OV_REG_COM3, 0x08);
		ov_i2c_set(ov, OV_REG_COM14, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF1);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x02);
	}
	if(res == OV_RES_VGA)
	{
		ov->img_w = 640;
		ov->img_h = 480;
		ov_i2c_set(ov, OV_REG_COM3, 0x00);
		ov_i2c_set(ov, OV_REG_COM14, 0x00);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF0);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x02);
	}
	if(res == OV_RES_QVGA)
	{
		ov->img_w = 320;
		ov->img_h = 240;
		ov_i2c_set(ov, OV_REG_COM3, 0x04);
		ov_i2c_set(ov, OV_REG_COM14, 0x19);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF1);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x02);
	}
	if(res == OV_RES_CIF)
	{
		ov->img_w = 352;
		ov->img_h = 288;
		ov_i2c_set(ov, OV_REG_COM3, 0x08);
		ov_i2c_set(ov, OV_REG_COM14, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF1);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x02);
	}
	if(res == OV_RES_QCIF)
	{
		ov->img_w = 176;
		ov->img_h = 144;
		ov_i2c_set(ov, OV_REG_COM3, 0x0C);
		ov_i2c_set(ov, OV_REG_COM14, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF1);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x4F);
	}
}