// This is a test to see which type of buffer is best for camera capture

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "jetsonBuffer.h"

/***********************************/
/*********Circular Buffer***********/
/***********************************/

/*!
 *  Function to initialize and allocate circular buffer
 *
 *  param[in/out]   cBuf     CIRCULAR_BUF double pointer
 *  param[in]   	bufSize  Desired size of buffer
 *
 *  returns SUCCESS
 */
Status
buffer_init
(
	CIRCULAR_BUF **cBuf,
	TxU32 bufSize
)
{
	Status status = SUCCESS;

	CIRCULAR_BUF *buffer = malloc(sizeof(*buffer));
	if (buffer == NULL)
	{
        printf("%s: Error initializing buffer\n",
            __FUNCTION__);
		status = INSUFFICIENT_RESOURCES;
		goto cleanup;
	}

	CIRCULAR_BUF_DATA *data = malloc(sizeof(*data) * bufSize);
	if (data == NULL)
	{
        printf("%s: Error initializing buffer data\n",
            __FUNCTION__);
		status = INSUFFICIENT_RESOURCES;
		goto cleanup;
	}

	if (pthread_mutex_init(&buffer->bufLock, NULL) != 0)
	{
        printf("%s: Error initializing buffer data\n",
            __FUNCTION__);
        status = MUTEX_ERROR;
		goto cleanup;
	}

	buffer->bufferData = data;
	buffer->head = -1;
	buffer->tail = -1;
	buffer->count = 0;
	buffer->size = bufSize;
	*cBuf = buffer;
	return status;

cleanup:
	free(buffer);
	free(data);
	return status;
}

/*!
 *  Function to add data to circular buffer
 *
 *  param[in/out]   cBuf  CIRCULAR_BUF double pointer
 *  param[in]   	data  Data to add (Subject to change)
 *
 *  returns SUCCESS
 */
Status
buffer_push
(
	CIRCULAR_BUF **cBuf,
	TxU32 data
)
{
	Status status = SUCCESS;

	pthread_mutex_lock(&(*cBuf)->bufLock);
	if ((*cBuf)->count == 0)
	{
		(*cBuf)->head = 0;
		(*cBuf)->tail = 0;
		(*cBuf)->count++;
		(*cBuf)->bufferData[0].data = data;
		goto cleanup;
	}

	(*cBuf)->tail = ((*cBuf)->tail + 1) % (*cBuf)->size;
	(*cBuf)->bufferData[(*cBuf)->tail].data = data;

	if ((*cBuf)->count == (*cBuf)->size)
	{
		(*cBuf)->head = ((*cBuf)->head + 1) % (*cBuf)->size;
		goto cleanup;
	}

	(*cBuf)->count++;

cleanup:
	pthread_mutex_unlock(&(*cBuf)->bufLock);
	return status;
}

/*!
 *  Function to get snapshot of the whole circular buffer
 *
 *  param[in]   cBuf  		CIRCULAR_BUF double pointer
 *  param[in]   snapshot 	CIRCULAR_BUF_DATA pointer
 *
 *  returns SUCCESS
 */
Status
buffer_get_snapshot
(
	CIRCULAR_BUF **cBuf,
	CIRCULAR_BUF_DATA *snapshot
)
{
	Status status = SUCCESS;

	pthread_mutex_lock(&(*cBuf)->bufLock);
	if ((*cBuf)->count == 0)
	{
		status = BUFFER_EMPTY;
		goto cleanup;
	}

	TxU32 j = (*cBuf)->head;
	for (TxU32 i = 0; i < (*cBuf)->size; i++)
	{
		snapshot[i].data = (*cBuf)->bufferData[j].data;
		j = (j + 1) % (*cBuf)->size;
	}

cleanup:
	pthread_mutex_unlock(&(*cBuf)->bufLock);
	return status;
}

/*!
 *  Function to free allocated circular buffer
 *
 *  param[in] 	cBuf  CIRCULAR_BUF double pointer
 *
 *  returns SUCCESS
 */
Status
buffer_free
(
	CIRCULAR_BUF **cBuf
)
{
	Status status = SUCCESS;
	free((*cBuf)->bufferData);
	free(*cBuf);
	return status;
}

typedef struct
{
	CIRCULAR_BUF *cBuf;
} ARGS;

void
*pushValues
(
	void *arg
)
{
	Status status = SUCCESS;
	ARGS *args = (ARGS *) arg;
	CIRCULAR_BUF *cBuf = (CIRCULAR_BUF *) args->cBuf;

	for (TxU32 i = 0; i < 1000000; i++)
	{
		status = buffer_push(&cBuf, i);
		if (status != SUCCESS)
		{
	        printf("%s: Buffer push failed status: 0x%x\n",
	            __FUNCTION__, status);
		}
	}
}

void
*readValues
(
	void *arg
)
{
	Status status = SUCCESS;
	ARGS *args = (ARGS *) arg;
	CIRCULAR_BUF *cBuf = (CIRCULAR_BUF *) args->cBuf;
	CIRCULAR_BUF_DATA *data = malloc(sizeof(*data) * C_BUF_SIZE);

	status = buffer_get_snapshot(&cBuf, data);
	if (status != SUCCESS)
	{
        printf("%s: Buffer snapshot failed status: 0x%x\n",
            __FUNCTION__, status);
		goto cleanup;
	}

	for (TxU32 i = 0; i < C_BUF_SIZE; i++)
	{
		printf("%d\n", (data[i].data));
	}

cleanup:
	free(data);
}

/*!
 *  Test for circular buffer push, snapshot, and mutex functionality
 *
 *  This test creates a circular buffer and has one thread populate it with
 *  data, while another thread gets a snapshot of the data. The output should be
 *  n=C_BUF_SIZE concurrent numbers between the range of 0-1000000
 *
 *  returns SUCCESS
 */
Status
test()
{
	Status status = SUCCESS;
	CIRCULAR_BUF *cBuf;
	pthread_t tid[2];
	ARGS *args;

	status = buffer_init(&cBuf, C_BUF_SIZE);
	if (status != SUCCESS)
	{
        printf("%s: Buffer init failed status: 0x%x\n",
            __FUNCTION__, status);
		goto cleanup;
	}

	args = malloc(sizeof(*args));
	if (args == NULL)
	{
        printf("%s: Error initializing args status\n",
            __FUNCTION__);
		status = INSUFFICIENT_RESOURCES;
		goto cleanup;
	}
	args->cBuf = cBuf;

	pthread_create(&(tid[0]), NULL, &pushValues, args);
	pthread_create(&(tid[1]), NULL, &readValues, args);

	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);

cleanup:
	free(args);
	buffer_free(&cBuf);
	return status;
}

// test
int main()
{
	test();
	return 0;
}