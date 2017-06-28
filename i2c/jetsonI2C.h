#ifndef _JETSON_I2C_H
#define _JETSON_I2C_H

#include "../Common/inc/jetsonStatus.h"
#include "../Common/inc/jetsonConstants.h"

#define I2C_DIR "/dev/i2c-"
#define DEFAULT_I2C_BUS I2C_1

typedef unsigned char I2CDevAddr;

typedef enum
{
	I2C_0 = 0,	// Pin 3  I2C_GP_DAT
	I2C_1		// Pin 27 I2C_GP1_DAT
} TX2_I2C_BUS;

// Need device register values to read/write

Status i2c_open (I2CDevAddr addr, TX2_I2C_BUS bus, int *fd);
Status i2c_close (int *fd);
#endif
