// This is a test to see which type of buffer is best for camera capture

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "jetsonBuffer.h"

// used only for testing
#include <time.h>
#include <stdlib.h>

/***********************************/
/*********Circular Buffer***********/
/***********************************/

/*!
 *  Function to initialize and allocate circular buffer
 *
 *  param[in/out]   cBuf     CIRCULAR_BUF double pointer
 *  param[in]       bufSize  Desired size of buffer
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
        goto cleanup1;
    }

    CIRCULAR_BUF_DATA *data = malloc(sizeof(*data) * bufSize);
    if (data == NULL)
    {
        printf("%s: Error initializing buffer data\n",
            __FUNCTION__);
        status = INSUFFICIENT_RESOURCES;
        goto cleanup2;
    }

    if (pthread_mutex_init(&buffer->bufLock, NULL) != 0)
    {
        printf("%s: Error initializing buffer lock\n",
            __FUNCTION__);
        status = MUTEX_ERROR;
        goto cleanup2;
    }

    buffer->bufferData  = data;
    buffer->head        = -1;
    buffer->tail        = -1;
    buffer->count       = 0;
    buffer->size        = bufSize;
    *cBuf               = buffer;
    return status;

cleanup2:
    free(data);
cleanup1:
    free(buffer);
    return status;
}

/*!
 *  Function to add data to circular buffer
 *
 *  param[in/out]   cBuf  CIRCULAR_BUF double pointer
 *  param[in]       data  Data to add (Subject to change)
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
    CIRCULAR_BUF *pCBuf = *cBuf;

    pthread_mutex_lock(&pCBuf->bufLock);
    if (pCBuf->count == 0)
    {
        pCBuf->head = 0;
        pCBuf->tail = 0;
        pCBuf->count++;
        pCBuf->bufferData[0].data = data;
        goto cleanup;
    }

    pCBuf->tail = (pCBuf->tail + 1) % pCBuf->size;
    pCBuf->bufferData[pCBuf->tail].data = data;

    if (pCBuf->count == pCBuf->size)
    {
        pCBuf->head = (pCBuf->head + 1) % pCBuf->size;
        goto cleanup;
    }

    pCBuf->count++;

cleanup:
    pthread_mutex_unlock(&pCBuf->bufLock);
    return status;
}

/*!
 *  Function to get snapshot of the whole circular buffer
 *
 *  param[in]   cBuf        CIRCULAR_BUF double pointer
 *  param[in]   snapshot    CIRCULAR_BUF_DATA pointer
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
    CIRCULAR_BUF *pCBuf = *cBuf;

    pthread_mutex_lock(&pCBuf->bufLock);
    if (pCBuf->count == 0)
    {
        status = BUFFER_EMPTY;
        goto cleanup;
    }

    TxU32 j = pCBuf->head;
    for (TxU32 i = 0; i < pCBuf->size; i++)
    {
        snapshot[i].data = pCBuf->bufferData[j].data;
        j = (j + 1) % pCBuf->size;
    }

cleanup:
    pthread_mutex_unlock(&pCBuf->bufLock);
    return status;
}

/*!
 *  Function to free allocated circular buffer
 *
 *  param[in]   cBuf  CIRCULAR_BUF double pointer
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
    CIRCULAR_BUF *pCBuf = *cBuf;
    free(pCBuf->bufferData);
    free(pCBuf);
    return status;
}

typedef struct
{
    CIRCULAR_BUF *cBuf;
} CBUF_ARGS;

void
*cBuf_push_values
(
    void *arg
)
{
    Status          status = SUCCESS;
    CBUF_ARGS       *args  = (CBUF_ARGS *) arg;
    CIRCULAR_BUF    *cBuf  = (CIRCULAR_BUF *) args->cBuf;
    TxU32           r;

    srand(time(NULL));

    for (TxU32 i = 0; i < 1000; i++)
    {
        r = rand() % 1000;
        usleep(r);

        status = buffer_push(&cBuf, i);
        if (status != SUCCESS)
        {
            printf("%s: Buffer push failed status: 0x%x\n",
                __FUNCTION__, status);
        }
    }
}

void
*cBuf_read_values
(
    void *arg
)
{
    Status              status  = SUCCESS;
    CBUF_ARGS           *args   = (CBUF_ARGS *) arg;
    CIRCULAR_BUF        *cBuf   = (CIRCULAR_BUF *) args->cBuf;
    CIRCULAR_BUF_DATA   *data   = malloc(sizeof(*data) * C_BUF_SIZE);
    TxU32               r;

    srand(time(NULL));

    for (TxU32 i = 0; i < 1000; i++)
    {
        r = rand() % 1000;
        usleep(r);

        status = buffer_get_snapshot(&cBuf, data);
        if (status != SUCCESS)
        {
            printf("%s: Buffer snapshot failed status: 0x%x\n",
                __FUNCTION__, status);
            goto cleanup;
        }

        for (TxU32 i = 0; i < C_BUF_SIZE - 1; i++)
        {
            // check if buffer is filled with valid data
            if (data[i+1].data == 0)
            {
                break;
            }
            else
            {
                if (data[i+1].data - data[i].data != 1)
                {
                    printf("Circular buffer test failed\n");
                }
            }
        }
    }

    printf("Circular buffer test passed\n");

cleanup:
    free(data);
}

/*!
 *  Test for circular buffer push, snapshot, and mutex functionality
 *
 *  This test creates a circular buffer and has one thread populate it with
 *  data, while another thread gets a snapshot of the data. The output should be
 *  n=C_BUF_SIZE consecutive numbers between the range of 0-1000000
 *
 *  returns SUCCESS
 */
Status
circular_buffer_test()
{
    Status status = SUCCESS;
    CIRCULAR_BUF *cBuf;
    pthread_t tid[2];
    CBUF_ARGS *args;

    status = buffer_init(&cBuf, C_BUF_SIZE);
    if (status != SUCCESS)
    {
        printf("%s: Buffer init failed status: 0x%x\n",
            __FUNCTION__, status);
        goto cleanup1;
    }

    args = malloc(sizeof(*args));
    if (args == NULL)
    {
        printf("%s: Error initializing args status\n",
            __FUNCTION__);
        status = INSUFFICIENT_RESOURCES;
        goto cleanup2;
    }
    args->cBuf = cBuf;

    pthread_create(&(tid[0]), NULL, &cBuf_push_values, args);
    pthread_create(&(tid[1]), NULL, &cBuf_read_values, args);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

cleanup2:
    free(args);
cleanup1:
    buffer_free(&cBuf);
    return status;
}

/***********************************/
/*******Multithreaded Queue*********/
/***********************************/

/*!
 *  Function to initialize and allocate threaded queue
 *
 *  param[in/out]   queue   QUEUE_T double pointer
 *  param[in]       qSize   Desired size of queue
 *
 *  returns SUCCESS
 */
Status
queue_init
(
    QUEUE_T **queue,
    TxU32 qSize
)
{
    Status status = SUCCESS;

    if (qSize > MAX_QUEUE_SIZE)
    {
        printf("%s: Error requested queue size too large\n",
            __FUNCTION__);
        return INSUFFICIENT_RESOURCES;
    }

    QUEUE_T *q = malloc(sizeof(*q));
    if (q == NULL)
    {
        printf("%s: Error initializing queue\n",
            __FUNCTION__);
        status = INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    if (pthread_mutex_init(&q->qLock, NULL) != 0)
    {
        printf("%s: Error initializing queue lock\n",
            __FUNCTION__);
        status = MUTEX_ERROR;
        goto cleanup;
    }

    if (pthread_cond_init(&q->qDataReady, NULL) != 0)
    {
        printf("%s: Error initializing queue Condition Variable\n",
            __FUNCTION__);
        status = CONDVAR_ERROR;
        goto cleanup;
    }

    if (pthread_cond_init(&q->qHasSpace, NULL) != 0)
    {
        printf("%s: Error initializing queue Condition Variable\n",
            __FUNCTION__);
        status = CONDVAR_ERROR;
        goto cleanup;
    }

    q->nodeCount    = 0;
    q->qSize        = qSize;
    q->head         = NULL;
    q->tail         = NULL;
    *queue          = q;
    return status;

cleanup:
    free(q);
    return status;
}

/*!
 *  Function to enqueue data into threaded queue
 *
 *  param[in/out]   queue   QUEUE_T double pointer
 *  param[in]       data    Data to enqueue
 *
 *  returns SUCCESS
 */
Status
queue_enqueue
(
    QUEUE_T **queue,
    TxU32 data
)
{
    Status  status  = SUCCESS;
    QUEUE_T *q      = *queue;

    pthread_mutex_lock(&q->qLock);
    while (q->nodeCount >= q->qSize)
    {
        pthread_cond_wait(&q->qHasSpace, &q->qLock);
    }

    QUEUE_NODE_T *qNode = malloc(sizeof(*qNode));
    if (qNode == NULL)
    {
        printf("%s: Error initializing queue node\n",
            __FUNCTION__);
        status = INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    qNode->data = data;
    qNode->next = NULL;

    if (q->head)
    {
        q->tail->next = qNode;
    }
    else
    {
        q->head = qNode;
    }

    q->tail = qNode;
    q->nodeCount++;
    pthread_cond_signal(&q->qDataReady);
    pthread_mutex_unlock(&q->qLock);
    return status;

cleanup:
    free(qNode);
    return status;
}

/*!
 *  Function to dequeue data from threaded queue
 *
 *  param[in]       queue   QUEUE_T double pointer
 *  param[in/out]   node    QUEUE_NODE_T node with dequeued data
 *
 *  returns SUCCESS
 */
Status
queue_dequeue
(
    QUEUE_T **queue,
    QUEUE_NODE_T **node
)
{
    Status  status  = SUCCESS;
    QUEUE_T *q      = *queue;

    pthread_mutex_lock(&q->qLock);

    while (q->nodeCount <= 0)
    {
        pthread_cond_wait(&q->qDataReady, &q->qLock);
    }

    if (q->head)
    {
        QUEUE_NODE_T *tmpNode = q->head;
        q->head = tmpNode->next;
        q->nodeCount--;
        *node = tmpNode;
    }
    else
    {
        printf("%s: Error queue empty\n",
            __FUNCTION__);
        status = BUFFER_EMPTY;
    }

    pthread_cond_signal(&q->qHasSpace);
    pthread_mutex_unlock(&q->qLock);
    return status;
}

/*!
 *  Function to free allocated threaded queue
 *
 *  param[in]   queue  QUEUE_T double pointer
 *
 *  returns SUCCESS
 */
Status
queue_free
(
    QUEUE_T **queue
)
{
    Status  status  = SUCCESS;
    QUEUE_T *q      = *queue;

    while (q->head)
    {
        QUEUE_NODE_T *nextNode;
        if (q->head->next)
        {
            nextNode = q->head->next;
        }
        free(q->head);
        q->head = nextNode;
    }

    free(q);
    return status;
}

typedef struct
{
    QUEUE_T *q;
} Q_ARGS;

void
*q_push_values
(
    void *arg
)
{
    Status  status  = SUCCESS;
    Q_ARGS  *args   = (Q_ARGS *) arg;
    QUEUE_T *q      = (QUEUE_T *) args->q;
    TxU32   r;

    srand(time(NULL));

    for (TxU32 i = 0; i < 1000; i++)
    {
        r = rand() % 1000;
        usleep(r);
        status = queue_enqueue(&q, i);
        if (status != SUCCESS)
        {
            printf("%s: Buffer push failed status: 0x%x\n",
                __FUNCTION__, status);
        }
    }
}

void
*q_read_values
(
    void *arg
)
{
    Status          status  = SUCCESS;
    Q_ARGS          *args   = (Q_ARGS *) arg;
    QUEUE_T         *q      = (QUEUE_T *) args->q;
    QUEUE_NODE_T    *q_node;
    TxU32           r;

    srand(time(NULL));

    TxU32 value = 0;
    for (TxU32 i = 0; i < 1000; i++)
    {
        r = rand() % 1000;
        usleep(r);

        status = queue_dequeue(&q, &q_node);
        if (status != SUCCESS)
        {
            printf("%s: Queue dequeue failed status: 0x%x\n",
                __FUNCTION__, status);
            goto cleanup;
        }

        if (value != q_node->data)
        {
            printf("Threaded queue test failed\n");
            goto cleanup;
        }
        free(q_node);
        value++;
    }

    printf("Threaded queue test passed\n");
    return NULL;

cleanup:
    free(q_node);
}

/*!
 *  Test for threaded queue enqueue, dequeue, and conditional variable
 *  functionality
 *
 *  This test creates a threaded queue that has one thread enqueue it with data,
 *  while another thread dequeues the data. The test enqueues and dequeues
 *  values up to 1000 at random times, but the dequeued value should be
 *  consecutive to the previously dequeued value.
 *
 *  returns SUCCESS
 */
Status
threaded_queue_test()
{
    Status status = SUCCESS;
    QUEUE_T *q;
    pthread_t tid[2];
    Q_ARGS *args;

    status = queue_init(&q, MAX_QUEUE_SIZE);
    if (status != SUCCESS)
    {
        printf("%s: Buffer init failed status: 0x%x\n",
            __FUNCTION__, status);
        goto cleanup1;
    }

    args = malloc(sizeof(*args));
    if (args == NULL)
    {
        printf("%s: Error initializing args status\n",
            __FUNCTION__);
        status = INSUFFICIENT_RESOURCES;
        goto cleanup2;
    }
    args->q = q;

    pthread_create(&(tid[0]), NULL, &q_push_values, args);
    pthread_create(&(tid[1]), NULL, &q_read_values, args);

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

cleanup2:
    free(args);
cleanup1:
    queue_free(&q);
    return status;
}

// Test
int main()
{
    circular_buffer_test();
    threaded_queue_test();
    return 0;
}