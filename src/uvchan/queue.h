#ifndef UVCHAN_QUEUE_H__
#define UVCHAN_QUEUE_H__

#include <stdlib.h>

typedef struct _uvchan_queue {
  void* buffer;
  size_t element_size;
  size_t head;
  size_t tail;
  size_t capacity_elements;
} uvchan_queue;

void uvchan_queue_init(uvchan_queue* queue, size_t num_elements,
                       size_t element_size);
void uvchan_queue_destroy(uvchan_queue* queue);

int uvchan_queue_push(uvchan_queue* queue, const void* element);
int uvchan_queue_pop(uvchan_queue* queue, void* element);

#endif  // UVCHAN_QUEUE_H__
