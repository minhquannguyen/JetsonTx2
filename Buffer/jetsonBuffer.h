#ifndef _JETSON_BUFFER_H
#define _JETSON_BUFFER_H

#include "../Common/inc/jetsonStatus.h"
#include "../Common/inc/jetsonConstants.h"

#define C_BUF_SIZE 8

typedef struct {
	TxU32 data;
} CIRCULAR_BUF_DATA;

typedef struct {
	CIRCULAR_BUF_DATA 	*bufferData;
	TxU32 				head;
	TxU32 				tail;
	TxU32 				count;
	TxU32 				size;
} CIRCULAR_BUF;

#endif