#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "jetsonTX2Gpio.h"

/*!
 *	Function to export GPIO pin specified
 *
 *  param[in]	gpio 	tx2GPIO Gpio pin
 *
 *	returns SUCCESS
 */
Status gpio_export(tx2GPIO gpio)
{
	int 	fd, length;
	char 	gpioBuffer[BUF_SIZE];
	Status 	status = SUCCESS;

	fd = open(GPIO_DIR "/export", O_WRONLY);
	if (fd < 0)
	{
		printf("%s: Error opening gpio/export\n", __FUNCTION__);
		status = FILE_HANDLING_ERROR;
		goto cleanup;
	}

	length = snprintf(gpioBuffer, sizeof(gpioBuffer), "%d", gpio);
	if (write(fd, gpioBuffer, length) != length)
	{
		printf("%s: Error writing to gpio/export\n", __FUNCTION__);
		status = FILE_HANDLING_ERROR;
		goto cleanup;
	}

cleanup:
	close(fd);
	return status;
}

/*!
 *	Function to unexport GPIO pin specified
 *
 *  param[in]	gpio 	tx2GPIO Gpio pin
 *
 *	returns SUCCESS
 */
Status gpio_unexport(tx2GPIO gpio)
{
	int 	fd, length;
	char	gpioBuffer[BUF_SIZE];
	Status 	status = SUCCESS;
	
	fd = open(GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0)
	{
		printf("%s: Error opening gpio/unexport\n", __FUNCTION__);
		status = FILE_HANDLING_ERROR;
		goto cleanup;
	}
	
	length = snprintf(gpioBuffer, sizeof(gpioBuffer), "%d", gpio);
	if (write(fd, gpioBuffer, length) != length)
	{
		printf("%s: Error writing to gpio/unexport\n", __FUNCTION__);
		status = FILE_HANDLING_ERROR;
		goto cleanup;
	}
	
cleanup:
	close(fd);
	return status;
}

/*!
 *	Function to set direction of GPIO pin specified
 *
 *  param[in] 	gpio      	tx2GPIO Gpio pin
 *	param[in] 	direction	Pin Direction (input, output)
 *
 *	returns SUCCESS
 */
Status gpio_set_direction(tx2GPIO gpio, pinDirection direction)
{
	int		fd;
	Status 	status = SUCCESS;
	
	status = gpio_open_file(gpio, "/direction", &fd);
	if (status != SUCCESS)
	{
		printf("%s: gpioOpenFile failed status = %d\n",
			__FUNCTION__, status);
		goto cleanup;
	}
	
	switch (direction)
	{
		case input:
			if (write(fd, "in", 3) != 3)
			{
				printf("%s: Error writing to gpio%d/direction\n",
						__FUNCTION__, gpio);
				status = FILE_HANDLING_ERROR;
				goto cleanup;
			}
			break;
		case output:
			if (write(fd, "out", 4) != 4)
			{
				printf("%s: Error writing to gpio%d/direction\n", 
						__FUNCTION__, gpio);
				status = FILE_HANDLING_ERROR;
				goto cleanup;
			}
			break;
	}

cleanup:
	close(fd);
	return status;
}

/*!
 *	Function to set value of GPIO pin specified
 *
 *  param[in] 	gpio    tx2GPIO Gpio pin
 *	param[in] 	value	Pin Value (low/0, high/1)
 *
 *	returns SUCCESS
 */
Status gpio_set_value(tx2GPIO gpio, pinValue value)
{
	int 	fd;
	Status 	status = SUCCESS;

	status = gpio_open_file(gpio, "/value", &fd);
	if (status != SUCCESS)
	{
		printf("%s: gpioOpenFile failed status = %d\n",
			__FUNCTION__, status);
		goto cleanup;
	}

	switch (value)
	{
		case low:
			if (write(fd, "0", 2) != 2)
			{
				printf("%s: Error writing to gpio%d/value\n",
						__FUNCTION__, gpio);
				status = FILE_HANDLING_ERROR;
				goto cleanup;
			}
			break;
		case high:
			if (write(fd, "1", 2) != 2)
			{
				printf("%s: Error writing to gpio%d/value\n",
						__FUNCTION__, gpio);
				status = FILE_HANDLING_ERROR;
				goto cleanup;
			}
			break;
	}

cleanup:
	close(fd);
	return status;
}

/*!
 *	Function to get value of GPIO pin specified
 *
 *  param[in] 	gpio    tx2GPIO Gpio pin
 *	param[out] 	value	Pin Value (low/0, high/1)
 *
 *	returns SUCCESS
 */
Status gpio_get_value(tx2GPIO gpio, pinValue *value)
{
	int 	fd;
	char	val;
	Status 	status = SUCCESS;
	*value = 0;

	// status = gpioOpenFile(gpio, "/value", &fd);
	status = gpio_open(gpio, &fd);
	if (status != SUCCESS)
	{
		printf("%s: gpioOpenFile failed status = %d\n",
			__FUNCTION__, status);
		goto cleanup;
	}

	if (read(fd, &val, 1) != 1)
	{
		printf("%s: failed to read gpio%d value\n",
			__FUNCTION__, gpio);
		status = FILE_HANDLING_ERROR;
		goto cleanup;
	}

	if (!isdigit(val))
	{
		printf("%s: gpio%d value is not numerical\n",
			__FUNCTION__, gpio);
		status = INVALID_DATA;
		goto cleanup;
	}

	// convert ascii to integer
	*value = val - '0';

cleanup:
	close(fd);
	return status;
}

// TODO
Status gpio_set_edge(tx2GPIO gpio, char *edge)
{
	int 	fd;
	Status 	status = SUCCESS;

	status = gpio_open_file(gpio, "/edge", &fd);
	if (status != SUCCESS)
	{
		printf("%s: gpioOpenFile failed status = %d\n",
			__FUNCTION__, status);
		// goto cleanup;
	}
}

/*!
 *	Helper function to open GPIO value file specified in RDONLY mode.
 *
 *  param[in] 	gpio      	tx2GPIO Gpio pin
 *	param[in] 	dirToOpen	directory ("/value", "/direction", "/edge")
 * 	param[out] 	fd 			file descriptor
 *
 *	returns SUCCESS
 */
Status gpio_open(tx2GPIO gpio, int *fd)
{
	int		length;
	char 	dirBuffer[BUF_SIZE];
	Status 	status = SUCCESS;

	// DEBUG
	// length = snprintf(dirBuffer,
	// 				  sizeof(dirBuffer),
	// 				  GPIO_DIR "/gpio%d/value",
	// 				  gpio);
	length = snprintf(dirBuffer,
				  sizeof(dirBuffer),
				  "/home/minhn/Tests/gpio%d/value",
				  gpio);
	if (length < 0)
	{
		printf("%s: Error formatting directory\n", __FUNCTION__);
		return FILE_HANDLING_ERROR;
	}

	*fd = open(dirBuffer, O_RDONLY | O_NONBLOCK);
	if (*fd < 0)
	{
		printf("%s: Error opening %s\n", __FUNCTION__, dirBuffer);
		close(*fd);
		return FILE_HANDLING_ERROR;
	}

	return status;
}

Status gpio_close(int fd)
{
	Status status = close(fd);
	if (status != SUCCESS)
	{
		printf("%s: gpioClose failed status = %d\n",
			__FUNCTION__, status);
	}

	return status;
}

/*!
 *	Static helper function to open GPIO file specified in RDWR mode.
 *	This function is more generic than gpioOpen.
 *
 *  param[in] 	gpio      	tx2GPIO Gpio pin
 *	param[in] 	dirToOpen	directory ("/value", "/direction", "/edge")
 * 	param[out] 	fd 			file descriptor
 *
 *	returns SUCCESS
 */
static Status gpio_open_file(tx2GPIO gpio, char *dirToOpen, int *fd)
{
	int		length;
	char 	dirBuffer[BUF_SIZE];
	Status 	status = SUCCESS;

	// DEBUG
	// length = snprintf(dirBuffer,
	// 				  sizeof(dirBuffer),
	// 				  GPIO_DIR "/gpio%d",
	// 				  gpio);
	length = snprintf(dirBuffer,
				  sizeof(dirBuffer),
				  "/home/minhn/Tests/gpio%d",
				  gpio);
	if (length < 0)
	{
		printf("%s: Error formatting directory\n", __FUNCTION__);
		return FILE_HANDLING_ERROR;
	}

	strcat(dirBuffer, dirToOpen);

	*fd = open(dirBuffer, O_RDWR | O_NONBLOCK);
	if (*fd < 0)
	{
		printf("%s: Error opening %s\n", __FUNCTION__, dirBuffer);
		close(*fd);
		return FILE_HANDLING_ERROR;
	}

	return status;
}

// TODO
Status gpio_active_low();

int main()
{
	unsigned int val;
	gpio_export(gpio298);
	gpio_set_direction(gpio298, output);
	gpio_set_value(gpio298, low);
	gpio_get_value(gpio298, &val);
	printf("%d\n", val);
	gpio_unexport(gpio298);
	return 0;
}
