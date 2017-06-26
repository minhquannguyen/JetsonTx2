#ifndef TX2_GPIO_H
#define TX2_GPIO_H

#include "../Common/inc/jetsonStatus.h"

#define GPIO_DIR "/sys/class/gpio"
#define BUF_SIZE 64

typedef unsigned int tx2GPIO;
typedef unsigned int pinDirection;
typedef unsigned int pinValue;

enum tx2GPIO {
	gpio298 = 298,	// Pin 31 GPIO9_MOTION_INT
	gpio388 = 388,	// pin 37 GPIO8_ALS_PROX_INT
	gpio389 = 389,	// Pin 33 GPIO11_AP_WAKE_BT
	gpio398 = 398,	// Pin 29 GPIO19_AUD_RST
	gpio481 = 481	//Pin 18 GPIO16_MDM_WAKE_AP
};

enum pinDirection {
	input = 0,
	output
};

enum pinValue {
	low = 0,
	high
};

static Status gpio_open_file(tx2GPIO gpio, char *dirToOpen, int *fd);
Status gpio_export(tx2GPIO gpio);
Status gpio_unexport(tx2GPIO gpio);
Status gpio_set_direction(tx2GPIO gpio, pinDirection direction);
Status gpio_set_value(tx2GPIO gpio, pinValue value);
Status gpio_get_value(tx2GPIO gpio, pinValue *value);
Status gpio_set_edge(tx2GPIO gpio, char *edge);
Status gpio_open(tx2GPIO gpio, int *fd);
Status gpio_close(int fd);
Status gpio_active_low();

#endif
