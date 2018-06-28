#include "./queue.h"

#include <string.h>

#define MEM_LOCATION(buffer, index, element_size) ( ((char*)(buffer)) + ((index) * (element_size)) )
#define INCREMENT(index, capacity_elements) ( ((index) + 1) % (capacity_elements) )

struct _uvchan_queue {
    void* buffer;
    size_t element_size;
    size_t head;
    size_t tail;
    size_t capacity_elements;
};

uvchan_queue* uvchan_queue_new(size_t num_elements, size_t element_size) {
    uvchan_queue *result;

    result = (uvchan_queue*)malloc(sizeof(uvchan_queue));
    result->buffer = malloc((num_elements + 1) * element_size);
    result->element_size = element_size;
    result->head = 1;
    result->tail = 0;
    result->capacity_elements = num_elements + 1;

    return result;
}

void uvchan_queue_destroy(uvchan_queue* queue) {
    free(queue->buffer);
    free(queue);
}

int uvchan_queue_push(uvchan_queue* queue, const void* element) {
    if (queue->head == queue->tail) {
        return 0;
    }

    memcpy(MEM_LOCATION(queue->buffer, queue->head, queue->element_size), element, queue->element_size);
    queue->head = INCREMENT(queue->head, queue->capacity_elements);

    return 1;
}

int uvchan_queue_pop(uvchan_queue* queue, void* element) {
    size_t next;

    next = INCREMENT(queue->tail, queue->capacity_elements);

    if (next == queue->head) {
        return 0;
    }

    memcpy(element, MEM_LOCATION(queue->buffer, next, queue->element_size), queue->element_size);
    queue->tail = next;

    return 1;
}
