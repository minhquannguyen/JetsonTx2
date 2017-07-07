#ifndef _JETSON_BUFFER_H
#define _JETSON_BUFFER_H

#include "../Common/inc/jetsonStatus.h"
#include "../Common/inc/jetsonConstants.h"

#define C_BUF_SIZE 8
#define MAX_QUEUE_SIZE 8

/*
 * Circular Buffer Struct and function definitions
 */

typedef struct
{
	TxU32 data;
} CIRCULAR_BUF_DATA;

typedef struct
{
	CIRCULAR_BUF_DATA 	*bufferData;
	TxU32 				head;
	TxU32 				tail;
	TxU32 				count;
	TxU32 				size;
	pthread_mutex_t		bufLock;
} CIRCULAR_BUF;

Status buffer_init(CIRCULAR_BUF **cBuf, TxU32 bufSize);
Status buffer_push(CIRCULAR_BUF **cBuf, TxU32 data);
Status buffer_get_snapshot(CIRCULAR_BUF **cBuf, CIRCULAR_BUF_DATA *snapshot);
Status buffer_free(CIRCULAR_BUF **cBuf);

/*
 * Multithreaded Queue Struct and function definitions
 */

typedef struct node
{
	TxU32 data;
	struct node *next;
} QUEUE_NODE_T;

typedef struct queue
{
	QUEUE_NODE_T 	*head;
	QUEUE_NODE_T 	*tail;
	TxU32			count;
	TxU32 			size;
	pthread_mutex_t	qLock;
	pthread_cond_t 	qDataReady;
	pthread_cond_t 	qHasSpace;
} QUEUE_T;

Status queue_init(QUEUE_T **queue, TxU32 maxSize);
Status queue_enqueue(QUEUE_T **queue, TxU32 data);
Status queue_dequeue(QUEUE_T **queue, QUEUE_NODE_T **node);

#endif