#ifndef I2C_INCLUDE
#define I2C_INCLUDE

#define ERR_I2C_OPEN -1
#define ERR_I2C_SETA -2

#ifdef __cplusplus
extern "C" {
#endif

int16_t  i2c_init(uint8_t  p_address);
int16_t  i2c_close();
void i2c_reset();

uint16_t i2c_read(uint8_t p_reg,void * buff,uint16_t length);
uint16_t i2c_write(uint8_t p_reg,void *buff,uint16_t length); 

#ifdef __cplusplus
}
#endif

#endif