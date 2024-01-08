// Leon Ozog 2023/2024
#include <stdint.h>

/* i2c address */
#define OV_I2C_ADDR 0x21

/* i2c registers */
#define OV_REG_CLKRC 								0x11
#define OV_REG_COM7  								0x12
#define OV_REG_COM3  								0x0C
#define OV_REG_COM9									0x14
#define OV_REG_COM14 								0x3E
#define OV_REG_COM13 								0x3D
#define OV_REG_COM15								0x40
#define OV_REG_COM16								0x41
#define OV_REG_SCALING_XSC  				0x70
#define OV_REG_SCALING_YSC  				0x71
#define OV_REG_SCALING_DCWCTR   		0x72
#define OV_REG_SCALING_PCLK_DIV  		0x73
#define OV_REG_SCALING_PCLK_DELAY  	0xA2
#define OV_REG_SATCTR 							0xC9


/* error codes */
typedef uint8_t ov_error;
#define OV_A_OKAY 0
#define OV_ERROR_I2C_FAIL 100
char* ov_error_what(ov_error err);

typedef struct {
	int (*i2c_get_reg)(uint8_t addr, uint8_t *buf, uint32_t len);
	int (*i2c_set_reg)(uint8_t addr, uint8_t *buf, uint32_t len);
	void (*wait_for_pclk)();
	uint8_t (*read_data)();
	void (*callback_frame)();
	void (*callback_row)();
	void (*callback_pixel)(uint16_t color);

	uint8_t ready;
	uint16_t img_w;
	uint16_t img_h;
	uint16_t px_x;
	uint16_t px_y;
} ov7670;

void ov_init(ov7670 *ov);

uint8_t ov_i2c_get(ov7670 *ov, uint8_t addr);
void ov_i2c_set(ov7670 *ov, uint8_t addr, uint8_t val);
void ov_i2c_bit(ov7670 *ov, uint8_t addr, uint8_t mask, uint8_t val);
void ov_sleep(uint32_t iter);

void ov_vsync(ov7670 *ov);
void ov_href_up(ov7670 *ov);
void ov_href_down(ov7670 *ov);

typedef uint8_t ov_res;
#define OV_RES_LIVE 0
#define OV_RES_VGA 1
#define OV_RES_QVGA 2
#define OV_RES_CIF 3
#define OV_RES_QCIF 4
void ov_set_res(ov7670 *ov, ov_res res);