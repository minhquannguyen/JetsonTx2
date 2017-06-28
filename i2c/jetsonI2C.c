#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "jetsonI2C.h"

/*!
 *  Function to open i2c bus specified and initialize
 *	communication with i2c device
 *
 *  param[in]   addr    I2CDevAddr i2c device address
 *  param[in]   bus    	TX2_I2C_BUS i2c bus to open
 * 	param[out]  fd    	File descriptor pointer
 *
 *  returns SUCCESS
 */
Status
i2c_open
(
	I2CDevAddr addr,
	TX2_I2C_BUS bus,
	int *fd
)
{
	int length;
	char i2cBuf[BUF_SIZE];

	length = snprintf(i2cBuf, sizeof(i2cBuf), I2C_DIR "%d", bus);
    if (length < 0)
    {
        printf("%s: Error formatting directory\n", __FUNCTION__);
        return FILE_HANDLING_ERROR;
    }

    *fd = open(i2cBuf, O_RDWR);
    if (*fd < 0)
    {
       printf("%s: Error opening %s\n", __FUNCTION__, i2cBuf);
       return FILE_HANDLING_ERROR;
    }

	if (ioctl(*fd, I2C_SLAVE, addr) < 0)
	{
        printf("%s: Error with ioctl to address = 0x%8x\n",
            __FUNCTION__, addr);
		return IOCTL_ERROR;
	}

	return SUCCESS;
}

/*!
 *  Function to close i2c bus specified
 *
 * 	param[in]  fd 	File descriptor pointer
 *
 *  returns SUCCESS
 */
Status
i2c_close
(
	int *fd
)
{
	Status status = close(*fd);
	if (status != SUCCESS)
	{
        printf("%s: Error closing i2c file status = %d\n",
            __FUNCTION__, status);
	}

	return status;
}

/*!
 *  Simple test to open/close i2c-1
 */
Status test1()
{
	int fd;
	Status status = SUCCESS;
	I2CDevAddr addr = 0x40;

	status = i2c_open(addr, DEFAULT_I2C_BUS, &fd);
	if (status != SUCCESS)
	{
		printf("%s: i2c_open failed\n",
				__FUNCTION__);
		status = FILE_HANDLING_ERROR;
	}

	status = i2c_close(&fd);
	if (status != SUCCESS)
	{
		printf("%s: i2c_close failed\n",
				__FUNCTION__);
		status = FILE_HANDLING_ERROR;
	}

	return status;
}

int main()
{
	Status status = SUCCESS;
	status = test1();
	return status;
}
