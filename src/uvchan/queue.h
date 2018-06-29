#ifndef UVCHAN_QUEUE_H__
#define UVCHAN_QUEUE_H__

#include <stdlib.h>
#include <uvchan/error.h>

typedef struct _uvchan_queue {
  void* buffer;
  size_t element_size;
  volatile size_t head;
  volatile size_t tail;
  size_t capacity_elements;
} uvchan_queue;

void uvchan_queue_init(uvchan_queue* queue, size_t num_elements,
                       size_t element_size);
void uvchan_queue_destroy(uvchan_queue* queue);

uvchan_error_t uvchan_queue_push(uvchan_queue* queue, const void* buffer);
uvchan_error_t uvchan_queue_pop(uvchan_queue* queue, void* buffer);

#endif  // UVCHAN_QUEUE_H__
