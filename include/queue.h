#ifndef QUEUE_H__
#define QUEUE_H__

typedef struct queue {
    int read;
    int write;
    int block_num;
    char **block;
} Queue;

#define InitQueue(head, T, node_count) \
    do { \
        (head) = (Queue *)malloc(sizeof(Queue)); \
        (head)->block_num = node_count + 1; \
        (head)->read = (head)->write = 0; \
        (head)->block = (char **)malloc(sizeof(T *) * (node_count + 1)); \
    } while (0)

#define CanRead(node) \
    ((node)->read != (node)->write)

#define CanWrite(node) \
    ((node)->read != (((node)->write + 1) % (node)->block_num))

#define GetRead(head, T) \
    ((T *)(((head)->block)[(head)->read]))

#define GetWrite(head, T) \
    (((head)->block)[(head)->write])

#define ReadBackward(head, T, V) \
    (CanRead(head) ? ((V) = GetRead(head, T), 1) : 0)

#define ReadForward(head) \
    ((head)->read = (((head)->read + 1) % (head)->block_num))

#define WriteBackward(head, T, V) \
    (CanWrite(head) ? ((V) = GetWrite(head, T)))

#define WriteForward(head) \
    ((head)->write = (((head)->write + 1) % (head)->block_num))

#define ReadOne(head, T, V) \
    (CanRead(head) ? ((V) = GetRead(head, T), ReadForward(head), 1) : 0)

#define WriteOne(head, T, V) \
    (CanWrite(head) ? (GetWrite(head, T) = (T *)(V), WriteForward(head), 1) : 0)

/* #define T \
     ((T *)(((head)->block)[(head)->write])) = (T *)(V) */


#endif /* QUEUE_H__ */
