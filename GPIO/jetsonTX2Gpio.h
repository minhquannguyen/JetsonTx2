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

static Status gpioOpenFile(tx2GPIO gpio, char *dirToOpen, int *fd);
Status gpioExport(tx2GPIO gpio);
Status gpioUnexport(tx2GPIO gpio);
Status gpioSetDirection(tx2GPIO gpio, pinDirection direction);
Status gpioSetValue(tx2GPIO gpio, pinValue value);
Status gpioGetValue(tx2GPIO gpio, pinValue *value);
Status gpioSetEdge(tx2GPIO gpio, char *edge);
Status gpioOpen(tx2GPIO gpio, int *fd);
Status gpioClose(int fd);
Status gpioActiveLow();

#endif
