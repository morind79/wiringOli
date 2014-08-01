/*
 * wiringOliI2C.h
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#ifdef __cplusplus
extern "C" {
#endif

extern int wiringOliI2CRead       (int fd);
extern int wiringOliI2CReadReg8   (int fd, int reg);
extern int wiringOliI2CReadReg16  (int fd, int reg);
extern int wiringOliI2CWrite      (int fd, int data);
extern int wiringOliI2CWriteReg8  (int fd, int reg, int data);
extern int wiringOliI2CWriteReg16 (int fd, int reg, int data);

extern int wiringOliI2CSetup      (int device, int address);

extern void I2C_Close(int fd);
extern void I2C_Send(int fd, unsigned char *buffer, unsigned char num_bytes);
extern void I2C_Read(int fd, unsigned char *buffer, unsigned char num_bytes);

#ifdef __cplusplus
}
#endif

