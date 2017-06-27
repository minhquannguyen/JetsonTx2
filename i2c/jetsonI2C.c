#include <stdio.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "jetsonI2C.h"

Status
i2c_open
(
	I2CBus bus
)
{
	int fd, length;
	char i2cBuf[BUF_SIZE];

	length = snprintf(i2cBuf, sizeof(i2cBuf), I2C_DIR "%d", bus);
}

int main()
{
	return 0;
}