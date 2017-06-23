#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "jetsonTX2Gpio.h"

Status gpioExport(tx2GPIO gpio)
{
	Status 	status = SUCCESS;
	char 	gpioBuffer[BUF_SIZE];
	int 	fd, length;

	fd = open(GPIO_DIR "/export", O_WRONLY);
	if (fd < 0)
	{
		printf("%s: Error opening gpio/export\n", __FUNCTION__);
		status = GENERIC_ERROR;
		goto cleanup;
	}

	length = snprintf(gpioBuffer, sizeof(gpioBuffer), "%d", gpio);
	if (write(fd, gpioBuffer, length) != length)
	{
		printf("%s: Error writing to gpio/export\n", __FUNCTION__);
		status = GENERIC_ERROR;
		goto cleanup;
	}

cleanup:
	close(fd);
	return status;
}

Status gpioUnexport(tx2GPIO gpio)
{
	Status 	status = SUCCESS;
	char	gpioBuffer[BUF_SIZE];
	int 	fd, length;
	
	fd = open(GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0)
	{
		printf("%s: Error opening gpio/unexport\n", __FUNCTION__);
		status = GENERIC_ERROR;
		goto cleanup;
	}
	
	length = snprintf(gpioBuffer, sizeof(gpioBuffer), "%d", gpio);
	if (write(fd, gpioBuffer, length) != length)
	{
		printf("%s: Error writing to gpio/unexport\n", __FUNCTION__);
		status = GENERIC_ERROR;
		goto cleanup;
	}
	
cleanup:
	close(fd);
	return status;
}

Status gpioSetDirection(tx2GPIO gpio, pinDirection direction)
{
	Status 	status = SUCCESS;
	char 	dirBuffer[BUF_SIZE];
	int		fd, length;
	
	length = snprintf(dirBuffer, 
					  sizeof(dirBuffer), 
					  GPIO_DIR "/gpio%d/direction", 
					  gpio);
	if (length < 0)
	{
		printf("%s: Error opening file\n", __FUNCTION__);
		return GENERIC_ERROR;
	}
	
	fd = open(dirBuffer, O_WRONLY);
	if (fd < 0)
	{
		printf("%s: Error opening gpio%d/direction\n", __FUNCTION__, gpio);
		status = GENERIC_ERROR;
		goto cleanup;
	}
	
	switch(direction)
	{
		case input:
			if (write(fd, "in", 3) != 3)
			{
				printf("%s: Error writing to gpio%d/direction\n",
						__FUNCTION__, gpio);
				status = GENERIC_ERROR;
				goto cleanup;
			}
			break;
		case output:
			if (write(fd, "out", 4) != 4)
			{
				printf("%s: Error writing to gpio%d/direction\n", 
						__FUNCTION__, gpio);
				status = GENERIC_ERROR;
				goto cleanup;
			}
			break;
	}

cleanup:
	close(fd);
	return status;
}

Status gpioSetValue(tx2GPIO gpio, pinValue value);

Status gpioGetValue(tx2GPIO gpio, pinValue *value);

Status gpioSetEdge(tx2GPIO gpio, char *edge);

Status gpioOpen(tx2GPIO gpio, int fd);

Status gpioClose(int fd);

Status gpioActiveLow();

int main()
{
	return 0;
}
