/*
 * interrupt.h
 *
 *  Created on: 14 oct. 2013
 *      Author: denm
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

#ifdef __cplusplus
extern "C" {
#endif

extern int gpio_export(unsigned int gpio);
extern int gpio_unexport(unsigned int gpio);
extern int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
extern int gpio_set_value(unsigned int gpio, unsigned int value);
extern int gpio_get_value(unsigned int gpio, unsigned int *value);
extern int gpio_set_edge(unsigned int gpio, char *edge);
extern int gpio_fd_open(unsigned int gpio);
extern int gpio_fd_close(int fd);

#ifdef __cplusplus
}
#endif


#endif /* INTERRUPT_H_ */
