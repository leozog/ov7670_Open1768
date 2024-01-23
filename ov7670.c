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

void ov_init(volatile ov7670 *ov)
{
	/* init internal variables */
	ov->state = OV_STOP;
	ov->img_w = 0;
	ov->img_h = 0;
	ov->px_x = 0;
	ov->px_y = 0;
	
	/* reset camera */
	ov_i2c_set(ov, OV_REG_COM7, 0x80);
	ov_sleep(1000000); // need to wait for about 1ms
	
	/* set internal clock to (clkout / 8) */
	ov_i2c_set(ov, OV_REG_CLKRC, 0x07);
	//ov_i2c_set(ov, OV_REG_CLKRC, 0x0F);
	
	
	/* set color output to 565 */
	ov_i2c_set(ov, OV_REG_COM7, 0b100);
	//ov_i2c_set(ov, OV_REG_COM15, 0xF0); //555
	ov_i2c_set(ov, OV_REG_COM15, 0xD0); //565
	
	/* dummy pixel insert in horizontal direction */
	//ov_i2c_set(ov, OV_REG_EXHCH, 0x00);
	//ov_i2c_set(ov, OV_REG_EXHCL, 0x10);
	
	// gain
	//ov_i2c_set(ov, OV_REG_COM9, 0x08);
	
	// UV saturation auto
	//ov_i2c_set(ov, OV_REG_COM13, 0xC8);
	
	//ov_i2c_set(ov, OV_REG_COM16, 0x12);
	//ov_i2c_set(ov, OV_REG_SATCTR, 0xF0);
}

uint8_t ov_i2c_get(volatile ov7670 *ov, uint8_t addr)
{
	uint8_t buf[1];
	while(ov->i2c_get_reg(addr, buf, 1));
	return *buf;
}

void ov_i2c_set(volatile ov7670 *ov, uint8_t addr, uint8_t val)
{
	uint8_t buf[1];
	*buf = val;
	while(ov->i2c_set_reg(addr, buf, 1));
}

void ov_i2c_bit(volatile ov7670 *ov, uint8_t addr, uint8_t mask, uint8_t val)
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

void ov_start(volatile ov7670 *ov)
{
	ov->state = OV_STOP_WAIT_FOR_VS;
}

void ov_stop(volatile ov7670 *ov)
{
	ov->state = OV_STOP;
}

void ov_vsync(volatile ov7670 *ov)
{
	if(ov->state == OV_STOP)
		return;
	
	if(ov->state == OV_STOP_WAIT_FOR_VS)
		ov->state = OV_RUNNING;
	
	ov->px_y = 0;
	
	ov->callback_frame();
}

void ov_href_up(volatile ov7670 *ov)
{
	if(ov->state != OV_RUNNING)
		return;
	if(ov->px_y < ov->img_h)
	{
		if(ov->state != OV_RUNNING)
			return;
		ov->wait_for_pclk();
		for(ov->px_x = 0; ov->px_x < ov->img_w; ov->px_x++)
		{
			if(ov->state != OV_RUNNING)
				return;
			
			ov->wait_for_pclk();
			volatile uint8_t val1 = ov->read_data();
			ov->wait_for_pclk();
			volatile uint8_t val2 = ov->read_data();
			
			ov->buffer[ov->px_x * 2] = val1;
			ov->buffer[ov->px_x * 2 + 1] = val2;
		}
		for(ov->px_x = 0; ov->px_x < ov->img_w; ov->px_x++)
		{
			if(ov->state != OV_RUNNING)
				return;
			
			uint16_t val1 = ov->buffer[ov->px_x * 2];
			uint16_t val2 = ov->buffer[ov->px_x * 2 + 1];
			
			/*uint16_t r = (val2 & 0b00000011)<<3 | (val1 & 0b11100000)>>5; //555
			uint16_t g = (val2 & 0b01111100)>>2;
			uint16_t b = (val1 & 0b00011111);*/
			
			if(ov->img_skip_left <= ov->px_x && ov->px_x < ov->img_w - ov->img_skip_right)
				//ov->callback_pixel(r<<11 | g<<6 | b); //555
				ov->callback_pixel(val2<<8 | val1); //565
		}
		if(ov->img_skip_up <= ov->px_y && ov->px_y < ov->img_h - ov->img_skip_down)
			ov->callback_row();
	}
}

/*void ov_href_down(ov7670 *ov)
{
	
}*/

void ov_set_res(volatile ov7670 *ov, ov_res res)
{
	if(res == OV_RES_LIVE)
	{
		ov->img_w = 320;
		ov->img_h = 240;
		ov->img_skip_left = (ov->img_w - 240) / 2;
		ov->img_skip_right = (ov->img_w - 240) / 2;
		ov->img_skip_up = 0;
		ov->img_skip_down = 0;
		ov_i2c_set(ov, OV_REG_COM3, 0x04);
		ov_i2c_set(ov, OV_REG_COM14, 0x19);
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
		ov->img_skip_left = 0;
		ov->img_skip_right = 0;
		ov->img_skip_up = 0;
		ov->img_skip_down = 0;
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
		ov->img_skip_left = 0;
		ov->img_skip_right = 0;
		ov->img_skip_up = 0;
		ov->img_skip_down = 0;
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
		ov->img_skip_left = 0;
		ov->img_skip_right = 0;
		ov->img_skip_up = 0;
		ov->img_skip_down = 0;
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
		ov->img_skip_left = 0;
		ov->img_skip_right = 0;
		ov->img_skip_up = 0;
		ov->img_skip_down = 0;
		ov_i2c_set(ov, OV_REG_COM3, 0x0C);
		ov_i2c_set(ov, OV_REG_COM14, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_XSC, 0x3A);
		ov_i2c_set(ov, OV_REG_SCALING_YSC, 0x35);
		ov_i2c_set(ov, OV_REG_SCALING_DCWCTR, 0x11);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DIV, 0xF1);
		ov_i2c_set(ov, OV_REG_SCALING_PCLK_DELAY, 0x4F);
	}
}