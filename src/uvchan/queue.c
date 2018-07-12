#include <uvchan/queue.h>

#include <assert.h>
#include <string.h>

#define MEM_LOCATION(buffer, index, element_size) \
  (((char*)(buffer)) + ((index) * (element_size)))
#define INCREMENT(index, capacity_elements) \
  (((index) + 1) % (capacity_elements))

void uvchan_queue_init(uvchan_queue* queue, size_t num_elements,
                       size_t element_size) {
  queue->buffer = malloc((num_elements + 1) * element_size);
  queue->element_size = element_size;
  queue->tail = 0;
  queue->capacity_elements = num_elements + 1;
  queue->head = INCREMENT(0, queue->capacity_elements);
}

void uvchan_queue_destroy(uvchan_queue* queue) {
  assert(queue->head == INCREMENT(queue->tail, queue->capacity_elements));
  free(queue->buffer);
  queue->buffer = 0L;
}

uvchan_error_t uvchan_queue_push(uvchan_queue* queue, const void* element) {
  if (queue->head == queue->tail) {
    return UVCHAN_ERR_QUEUE_FULL;
  }

  memcpy(MEM_LOCATION(queue->buffer, queue->head, queue->element_size), element,
         queue->element_size);
  queue->head = INCREMENT(queue->head, queue->capacity_elements);

  return UVCHAN_ERR_SUCCESS;
}

uvchan_error_t uvchan_queue_pop(uvchan_queue* queue, void* element) {
  size_t next;

  next = INCREMENT(queue->tail, queue->capacity_elements);

  if (next == queue->head) {
    return UVCHAN_ERR_QUEUE_EMPTY;
  }

  memcpy(element, MEM_LOCATION(queue->buffer, next, queue->element_size),
         queue->element_size);
  queue->tail = next;

  return UVCHAN_ERR_SUCCESS;
}
