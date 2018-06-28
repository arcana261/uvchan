#ifndef _UVCHAN_QUEUE_H__
#define _UVCHAN_QUEUE_H__

#include <stdlib.h>

typedef struct _uvchan_queue uvchan_queue;

uvchan_queue* uvchan_queue_new(size_t num_elements, size_t element_size);
void uvchan_queue_destroy(uvchan_queue* queue);

int uvchan_queue_push(uvchan_queue* queue, const void* element);
int uvchan_queue_pop(uvchan_queue* queue, void* element);

#endif  // _UVCHAN_QUEUE_H__
