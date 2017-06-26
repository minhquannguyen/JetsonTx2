#ifndef _JETSON_STATUS_H
#define _JETSON_STATUS_H

typedef int Status;

typedef enum 
{
    SUCCESS,
    INSUFFICIENT_RESOURCES,
    INVALID_DATA,
    FILE_HANDLING_ERROR,
    GENERIC_ERROR
} STATUS_VALUES;

#endif
