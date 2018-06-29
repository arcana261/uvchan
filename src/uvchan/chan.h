#ifndef UVCHAN_CHAN_H__
#define UVCHAN_CHAN_H__

#include <uv.h>
#include <uvchan/error.h>
#include <uvchan/queue.h>

typedef struct _uvchan_t {
  uvchan_queue queue;
  int closed;
  int polling;
  int poll_required;
  int reference_count;
} uvchan_t;

typedef struct _uvchan_handle_t uvchan_handle_t;

typedef void (*uvchan_push_cb)(uvchan_handle_t* handle, uvchan_error_t err);
typedef void (*uvchan_pop_cb)(uvchan_handle_t* handle, void* buffer,
                              uvchan_error_t err);

typedef struct _uvchan_handle_t {
  uv_idle_t idle_handle;

  uvchan_t* ch;
  void* element;
  void* callback;
  void* data;
} uvchan_handle_t;

uvchan_t* uvchan_new(size_t num_elements, size_t element_size);
void uvchan_ref(uvchan_t* chan);
void uvchan_unref(uvchan_t* chan);

void uvchan_handle_init(uv_loop_t* loop, uvchan_handle_t* handle, uvchan_t* ch);

void uvchan_close(uvchan_t* chan);
void uvchan_start_push(uvchan_handle_t* handle, const void* buffer,
                       uvchan_push_cb cb);
void uvchan_start_pop(uvchan_handle_t* handle, void* buffer, uvchan_pop_cb cb);

#endif  // UVCHAN_UVCHAN_H__
