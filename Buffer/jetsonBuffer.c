// This is a test to see which type of buffer is best for camera capture

#include <stdio.h>
#include <stdlib.h>
#include "jetsonBuffer.h"

/***********************************/
/*********Circular Buffer***********/
/***********************************/

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

Status
buffer_size
(
	CIRCULAR_BUF cBuf,
	TxU32 *size
)
{
	*size = cBuf.size;
	return SUCCESS;
}

Status buffer_push
(
	CIRCULAR_BUF **cBuf,
	TxU32 data
)
{
	Status status = SUCCESS;

	if ((*cBuf)->count == 0)
	{
		(*cBuf)->head = 0;
		(*cBuf)->tail = 0;
		(*cBuf)->count++;
		(*cBuf)->bufferData[0].data = data;
		return status;
	}

	(*cBuf)->tail = ((*cBuf)->tail + 1) % (*cBuf)->size;
	(*cBuf)->bufferData[(*cBuf)->tail].data = data;

	if ((*cBuf)->count == (*cBuf)->size)
	{
		(*cBuf)->head = ((*cBuf)->head + 1) % (*cBuf)->size;
		return status;
	}

	(*cBuf)->count++;
	return status;
}

Status
buffer_get_snapshot
(
	CIRCULAR_BUF **cBuf,
	CIRCULAR_BUF_DATA *data
)
{
	if ((*cBuf)->count == 0)
	{
		return BUFFER_EMPTY;
	}

	TxU32 j = (*cBuf)->head;
	for (TxU32 i = 0; i < (*cBuf)->size; i++)
	{
		data[i].data = (*cBuf)->bufferData[j].data;
		j = (j + 1) % (*cBuf)->size;
	}

}

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

// test
int main()
{
	CIRCULAR_BUF *cBuf;
	int size;
	buffer_init(&cBuf, C_BUF_SIZE);
	buffer_push(&cBuf, 0);
	buffer_push(&cBuf, 1);
	buffer_push(&cBuf, 2);
	buffer_push(&cBuf, 3);
	buffer_push(&cBuf, 4);
	buffer_push(&cBuf, 5);
	buffer_push(&cBuf, 6);
	buffer_push(&cBuf, 7);
	buffer_push(&cBuf, 8);
	buffer_push(&cBuf, 9);
	buffer_push(&cBuf, 10);
	buffer_push(&cBuf, 11);
	buffer_push(&cBuf, 12);
	buffer_push(&cBuf, 13);
	buffer_push(&cBuf, 14);

	for (int i =0; i < cBuf->size; i++)
	{
		printf("%d\n", cBuf->bufferData[i].data);
	}

	printf("head %d\n", cBuf->bufferData[cBuf->head].data);
	printf("tail %d\n", cBuf->bufferData[cBuf->tail].data);

	CIRCULAR_BUF_DATA *data = malloc(sizeof(*data) * C_BUF_SIZE);
	buffer_get_snapshot(&cBuf, data);

	for(int i = 0; i < C_BUF_SIZE; i++)
	{
		printf("%d\n", data[i].data);
	}

	buffer_free(&cBuf);
	return 0;
}