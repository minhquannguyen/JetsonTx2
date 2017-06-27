#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "jetsonTX2Gpio.h"

/*!
 *  Function to export GPIO pin specified
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *
 *  returns SUCCESS
 */
Status
gpio_export
(
    TX2GPIO gpio
)
{
    int     fd, length;
    char    gpioBuffer[BUF_SIZE];
    Status  status = SUCCESS;

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
 *  Function to unexport GPIO pin specified
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *
 *  returns SUCCESS
 */
Status
gpio_unexport
(
    TX2GPIO gpio
)
{
    int     fd, length;
    char    gpioBuffer[BUF_SIZE];
    Status  status = SUCCESS;

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
 *  Function to set direction of GPIO pin specified
 *
 *  param[in]   gpio        TX2GPIO Gpio pin
 *  param[in]   direction   Pin Direction (input, output)
 *
 *  returns SUCCESS
 */
Status
gpio_set_direction
(
    TX2GPIO gpio,
    GPIO_PIN_DIRECTION direction
)
{
    int     fd;
    Status  status = SUCCESS;

    status = _gpio_open_file(gpio, GPIO_FILE_MODE_WRITE, "/direction", &fd);
    if (status != SUCCESS)
    {
        printf("%s: gpioOpenFile failed status = %d\n",
            __FUNCTION__, status);
        goto cleanup;
    }

    switch (direction)
    {
        case GPIO_DIRECTION_INPUT:
            if (write(fd, "in", 3) != 3)
            {
                printf("%s: Error writing to gpio%d/direction\n",
                        __FUNCTION__, gpio);
                status = FILE_HANDLING_ERROR;
                goto cleanup;
            }
            break;
        case GPIO_DIRECTION_OUTPUT:
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
 *  Function to set value of GPIO pin specified
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *  param[in]   value   Pin Value (low/0, high/1)
 *
 *  returns SUCCESS
 */
Status
gpio_set_value
(
    TX2GPIO gpio,
    GPIO_PIN_VALUE value
)
{
    int     fd;
    Status  status = SUCCESS;

    status = _gpio_open_file(gpio, GPIO_FILE_MODE_WRITE, "/value", &fd);
    if (status != SUCCESS)
    {
        printf("%s: gpioOpenFile failed status = %d\n",
            __FUNCTION__, status);
        goto cleanup;
    }

    switch (value)
    {
        case GPIO_PIN_VALUE_LOW:
            if (write(fd, "0", 2) != 2)
            {
                printf("%s: Error writing to gpio%d/value\n",
                        __FUNCTION__, gpio);
                status = FILE_HANDLING_ERROR;
                goto cleanup;
            }
            break;
        case GPIO_PIN_VALUE_HIGH:
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
 *  Function to get value of GPIO pin specified
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *  param[out]  value   Pin Value (low/0, high/1)
 *
 *  returns SUCCESS
 */
Status
gpio_get_value
(
    TX2GPIO gpio,
    GPIO_PIN_VALUE *value
)
{
    int     fd;
    char    val;
    Status  status = SUCCESS;
    *value = 0;

    status = _gpio_open_file(gpio, GPIO_FILE_MODE_READ, "/value", &fd);
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

/*!
 *  Function to set edge value of GPIO pin specified
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *  param[in]   edge    Edge Value ("none", "rising", "falling", "both")
 *
 *  returns SUCCESS
 */
Status
gpio_set_edge
(
    TX2GPIO gpio,
    GPIO_PIN_EDGE edge
)
{
    int     fd;
    char    buf[BUF_SIZE];
    int     bufLen;
    Status  status = SUCCESS;

    status = _gpio_open_file(gpio, GPIO_FILE_MODE_WRITE, "/edge", &fd);
    if (status != SUCCESS)
    {
        printf("%s: gpioOpenFile failed status = %d\n",
            __FUNCTION__, status);
        goto cleanup;
    }

    switch (edge)
    {
        case GPIO_PIN_EDGE_NONE:
            strcpy(buf, "none");
            break;
        case GPIO_PIN_EDGE_RISING:
            strcpy(buf, "rising");
            break;
        case GPIO_PIN_EDGE_FALLING:
            strcpy(buf, "falling");
            break;
        case GPIO_PIN_EDGE_BOTH:
            strcpy(buf, "both");
            break;
    }

    bufLen = strlen(buf) + 1;
    if (write(fd, buf, bufLen) != bufLen)
    {
        printf("%s: failed to write gpio%d/edge - none\n",
            __FUNCTION__, gpio);
        status = FILE_HANDLING_ERROR;
        goto cleanup;
    }

cleanup:
    close(fd);
    return status;
}

/*!
 *  Function to set active_low value of GPIO pin specified
 *
 *  param[in]   gpio        TX2GPIO Gpio pin
 *  param[in]   activeLow   active_low Value ("none", "rising",
 *                                            "falling", "both")
 *
 *  returns SUCCESS
 */
Status
gpio_active_low
(
    TX2GPIO gpio,
    GPIO_ACTIVE_LOW activeLow
)
{
    int     fd;
    Status  status = SUCCESS;

    status = _gpio_open_file(gpio, GPIO_FILE_MODE_WRITE, "/active_low", &fd);
    if (status != SUCCESS)
    {
        printf("%s: gpioOpenFile failed status = %d\n",
            __FUNCTION__, status);
        goto cleanup;
    }

    switch (activeLow)
    {
        case GPIO_ACTIVE_LOW_FALSE:
            if (write(fd, "0", 2) != 2)
            {
                printf("%s: Error writing to gpio%d/value\n",
                        __FUNCTION__, gpio);
                status = FILE_HANDLING_ERROR;
                goto cleanup;
            }
            break;
        case GPIO_ACTIVE_LOW_TRUE:
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
 *  Helper function to open GPIO value file specified in RDONLY mode.
 *
 *  param[in]   gpio    TX2GPIO Gpio pin
 *  param[out]  fd      file descriptor
 *
 *  returns SUCCESS
 */
Status
gpio_open
(
    TX2GPIO gpio,
    int *fd
)
{
    int     length;
    char    dirBuffer[BUF_SIZE];
    Status  status = SUCCESS;

    // DEBUG
     length = snprintf(dirBuffer,
                    sizeof(dirBuffer),
                    GPIO_DIR "/gpio%d/value",
                    gpio);
    //length = snprintf(dirBuffer,
                  //sizeof(dirBuffer),
                  //"/home/minhn/Tests/gpio%d/value",
                  //gpio);
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

/*!
 *  Helper function to close GPIO file pointed to by fd.
 *
 *  param[in]   fd  GPIO file descriptor
 *
 *  returns SUCCESS
 */
Status
gpio_close
(
    int fd
)
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
 *  Static helper function to open GPIO file specified in RDWR mode.
 *  This function is more generic than gpioOpen.
 *
 *  param[in]   gpio        TX2GPIO Gpio pin
 *  param[in]   dirToOpen   directory ("/value", "/direction",
 *                                     "/edge", "active_low")
 *  param[in]   mode        Gpio file mode (read/write)
 *  param[out]  fd          file descriptor
 *
 *  returns SUCCESS
 */
static Status
_gpio_open_file
(
    TX2GPIO gpio,
    GPIO_FILE_MODE mode,
    char *dirToOpen,
    int *fd
)
{
    int     length;
    char    dirBuffer[BUF_SIZE];
    Status  status = SUCCESS;

    // DEBUG
     length = snprintf(dirBuffer,
                    sizeof(dirBuffer),
                    GPIO_DIR "/gpio%d",
                    gpio);
    //length = snprintf(dirBuffer,
                  //sizeof(dirBuffer),
                  //"/home/minhn/Tests/gpio%d",
                  //gpio);
    if (length < 0)
    {
        printf("%s: Error formatting directory\n", __FUNCTION__);
        return FILE_HANDLING_ERROR;
    }

    strcat(dirBuffer, dirToOpen);

    switch (mode)
    {
        case GPIO_FILE_MODE_READ:
            *fd = open(dirBuffer, O_RDONLY | O_NONBLOCK);
            break;
        case GPIO_FILE_MODE_WRITE:
            *fd = open(dirBuffer, O_WRONLY | O_NONBLOCK | O_TRUNC);
            break;
    }

    if (*fd < 0)
    {
        printf("%s: Error opening %s\n", __FUNCTION__, dirBuffer);
        close(*fd);
        return FILE_HANDLING_ERROR;
    }

    return status;
}

/*!
 *  Simple test to turn LED on/off at gpio pin 298
 */
Status
ledTest1()
{
    GPIO_PIN_VALUE val;
    Status status = SUCCESS;

    status = gpio_export(gpio298);
    if (status != SUCCESS)
    {
		printf("%s: gpio%d may be exported already\n",
				__FUNCTION__, gpio298);
		goto cleanup;
	}

    status = gpio_set_direction(gpio298, GPIO_DIRECTION_OUTPUT);
    if (status != SUCCESS)
    {
		printf("%s: gpio%d set direction error\n",
				__FUNCTION__, gpio298);
	}

    for (int i = 0; i < 100; i++)
    {
		usleep(100000);
		if (i % 2 == 0)
		{
			status = gpio_set_value(gpio298, GPIO_PIN_VALUE_HIGH);
		}
		else
		{
			status = gpio_set_value(gpio298, GPIO_PIN_VALUE_LOW);
		}
		if (status != SUCCESS)
		{
			printf("%s: gpio%d set value error\n",
				__FUNCTION__, gpio298);
		}

		status = gpio_get_value(gpio298, &val);
		if (status != SUCCESS)
		{
			printf("%s: gpio%d get value error\n",
				__FUNCTION__, gpio298);
		}
		printf("write gpio%d value: %d\n", gpio298, val);
	}

cleanup:
    gpio_unexport(gpio298);
    return status;
}

// Testing
int main()
{
	Status status = SUCCESS;
	status = ledTest1();
	if (status != SUCCESS)
	{
		return -1;
	}
    return 0;
}
