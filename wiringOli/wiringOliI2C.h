#ifndef I2C_INCLUDE
#define I2C_INCLUDE

#define ERR_I2C_OPEN -1
#define ERR_I2C_SETA -2

#ifdef __cplusplus
extern "C" {
#endif

extern int wiringOliI2CRead       (int fd) ;
extern int wiringOliI2CReadReg8   (int fd, int reg) ;
extern int wiringOliI2CReadReg16  (int fd, int reg) ;
extern int wiringOliI2CWrite      (int fd, int data) ;
extern int wiringOliI2CWriteReg8  (int fd, int reg, int data) ;
extern int wiringOliI2CWriteReg16 (int fd, int reg, int data) ;

extern int wiringOliI2CSetup             (int devId) ;

#ifdef __cplusplus
}
#endif

#endif
