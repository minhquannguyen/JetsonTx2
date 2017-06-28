#ifndef TX2_GPIO_H
#define TX2_GPIO_H

#include "../Common/inc/jetsonStatus.h"
#include "../Common/inc/jetsonConstants.h"

#define GPIO_DIR "/sys/class/gpio"

typedef enum {
    gpio298 = 298,  // Pin 31 GPIO9_MOTION_INT
    gpio388 = 388,  // pin 37 GPIO8_ALS_PROX_INT
    gpio389 = 389,  // Pin 33 GPIO11_AP_WAKE_BT
    gpio398 = 398,  // Pin 29 GPIO19_AUD_RST
    gpio481 = 481   // Pin 18 GPIO16_MDM_WAKE_AP
} TX2_GPIO;

typedef enum {
    GPIO_FILE_MODE_READ = 0,
    GPIO_FILE_MODE_WRITE
} GPIO_FILE_MODE;

typedef enum {
    GPIO_DIRECTION_INPUT = 0,
    GPIO_DIRECTION_OUTPUT
} GPIO_PIN_DIRECTION;

typedef enum {
    GPIO_PIN_VALUE_LOW = 0,
    GPIO_PIN_VALUE_HIGH
} GPIO_PIN_VALUE;

typedef enum {
    GPIO_PIN_EDGE_NONE = 0,
    GPIO_PIN_EDGE_RISING,
    GPIO_PIN_EDGE_FALLING,
    GPIO_PIN_EDGE_BOTH
} GPIO_PIN_EDGE;

typedef enum {
    GPIO_ACTIVE_LOW_FALSE = 0,
    GPIO_ACTIVE_LOW_TRUE
} GPIO_ACTIVE_LOW;

static Status _gpio_open_file(TX2_GPIO gpio, GPIO_FILE_MODE mode, char *dirToOpen, int *fd);
Status gpio_export(TX2_GPIO gpio);
Status gpio_unexport(TX2_GPIO gpio);
Status gpio_set_direction(TX2_GPIO gpio, GPIO_PIN_DIRECTION direction);
Status gpio_set_value(TX2_GPIO gpio, GPIO_PIN_VALUE value);
Status gpio_get_value(TX2_GPIO gpio, GPIO_PIN_VALUE *value);
Status gpio_set_edge(TX2_GPIO gpio, GPIO_PIN_EDGE edge);
Status gpio_active_low(TX2_GPIO gpio, GPIO_ACTIVE_LOW activeLow);
Status gpio_open(TX2_GPIO gpio, int *fd);
Status gpio_close(int *fd);

#endif
