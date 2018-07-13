#ifndef UVCHAN_QUEUE_H__
#define UVCHAN_QUEUE_H__

#include <stdlib.h>
#include <uvchan/error.h>

/**
 * @brief Models a FIFO queue
 *
 * uvchan_queue models a FIFO queue data structure.
 * It is indended to be used internally by channel
 * objects, however it is part of public API and
 * can be used safely as such.
 *
 * Queue stores objects of the same type, and uses
 * memcpy function to deep copy objects into it's internal
 * memory.
 *
 * uvchan_queue is not threadsafe except when it is used
 * by a single consumer and a single producer both in
 * seperate threads, doing lock-less operations on queue.
 * This fact can be observed by \b test_ipc_safety. This makes
 * uvchan_queue ideal for connecting working threads doing
 * IO or other stuff and libuv idle handlers, because
 * there would be a single producer and a single consumer,
 * and the queue itself is lockless. This is achieved by
 * incorporating \b volatile on internal queue structure.
 *
 * Also note that queue does not private a \b length operation
 * intentionally. As providing this method would violate
 * constrained lock-less thread safety of queue.
 *
 * @code{.c}
 * uvchan_queue q;
 * int value;
 *
 * uvchan_queue_init(&q, 1, sizeof(int));
 *
 * value = 5;
 * uvchan_queue_push(&q, &value);
 *
 * uvchan_queue_pop(&q, &value);
 *
 * uvchan_queue_destroy(&q);
 * @endcode
 *
 * @see uvchan_queue_init
 * @see uvchan_queue_push
 * @see uvchan_queue_pop
 * @see uvchan_queue_destroy
 */
typedef struct _uvchan_queue {
  void* _buffer;            /**< @private */
  size_t element_size;      /**< size of each item in queue in bytes */
  volatile size_t _head;    /**< @private */
  volatile size_t _tail;    /**< @private */
  size_t capacity_elements; /**< capacity of queue */
} uvchan_queue;

/**
 * @brief initialize a new queue
 *
 * This function initializes a new queue. provided @p queue must
 * exist beforehand. Initialized data structure has to be later
 * destroyed by calling #uvchan_queue_destroy. Note that since
 * this method does not allocate actual memory to hold _uvchan_queue
 * structure itself, it is up to the caller to manage memory lifecycle
 * of _uvchan_queue instance.
 *
 * @param queue location of _uvchan_queue instance to initialize.
 * @param num_elements shows desired maximum capacity of queue to
 * contain specified number of items.
 * @param element_size shows size of each item in bytes. This is
 * typically a \b sizeof operation.
 *
 * @see uvchan_queue_destroy
 */
void uvchan_queue_init(uvchan_queue* queue, size_t num_elements,
                       size_t element_size);

/**
 * @brief destroy resources allocated to queue
 *
 * This function destroys a queue created by previously calling
 * #uvchan_queue_init. This method only releases structures allocated
 * internally by queue so the caller has the responsibility of
 * deallocating _uvchan_queue instance itself.
 *
 * @param queue queue structure to release
 *
 * @warning since _uvchan_queue handles items by deep-copying them
 * into it's internal structure, it has no knowledge of how to
 * properly disallocate the data such items refer to (e.g. char* strings).
 * As such, #uvchan_queue_destroy performs an assertion that the
 * queue should be empty before calling #uvchan_queue_destroy.
 *
 * @see uvchan_queue_init
 */
void uvchan_queue_destroy(uvchan_queue* queue);

/**
 * @brief push a new item to back of queue
 *
 * This function pushes a new item to back of queue. This process
 * is done by deep-copying @p buffer parameter using memcpy function.
 *
 * @return zero if operation succeeds. non-zero if error occurs.
 *
 * @see UVCHAN_ERR_SUCCESS
 * @see UVCHAN_ERR_QUEUE_FULL
 * @see uvchan_strerr
 */
uvchan_error_t uvchan_queue_push(uvchan_queue* queue, const void* buffer);

/**
 * @brief pop an item from front of queue
 *
 * This function retrieves an item from front of queue. This process
 * is done by deep-copying into @p buffer parameter using memcpy function.
 * so the caller has to have allocated at least #_uvchan_queue#element_size
 * bytes.
 *
 * @return zero if operation succeeds. non-zero if error occurs.
 *
 * @see UVCHAN_ERR_SUCCESS
 * @see UVCHAN_ERR_QUEUE_EMPTY
 * @see uvchan_strerr
 */
uvchan_error_t uvchan_queue_pop(uvchan_queue* queue, void* buffer);

#endif  // UVCHAN_QUEUE_H__
