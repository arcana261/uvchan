#include <uvchan/chan.h>
#include <uvchan/error.h>
#include "./config.h"

static void _uvchan_default_push_cb(uvchan_handle_t* handle,
                                    uvchan_error_t err);
static void _uvchan_default_pop_cb(uvchan_handle_t* handle, void* buffer,
                                   uvchan_error_t err);

uvchan_t* uvchan_new(size_t num_elements, size_t element_size) {
  uvchan_t* chan;

  chan = (uvchan_t*)malloc(sizeof(uvchan_t));

  if (num_elements < 1) {
    num_elements = 1;
    chan->poll_required = 1;
  } else {
    chan->poll_required = 0;
  }

  uvchan_queue_init(&chan->queue, num_elements, element_size);
  chan->closed = 0;
  chan->polling = 0;
  chan->reference_count = 1;

  return chan;
}

void uvchan_unref(uvchan_t* chan) {
  if ((--chan->reference_count) < 1) {
    uvchan_queue_destroy(&chan->queue);
    free(chan);
  }
}

void uvchan_ref(uvchan_t* chan) { ++chan->reference_count; }

void uvchan_close(uvchan_t* chan) { chan->closed = 1; }

void uvchan_handle_init(uv_loop_t* loop, uvchan_handle_t* handle,
                        uvchan_t* ch) {
  uv_idle_init(loop, (uv_idle_t*)handle);
  handle->callback = 0L;
  handle->ch = ch;
  handle->data = 0L;
}

#ifdef LIBUV_0X
static void _uvchan_start_push_idle_cb(uv_idle_t* handle, int status) {
#elif LIBUV_1X
static void _uvchan_start_push_idle_cb(uv_idle_t* handle) {
#else
#error callback not defined for unknown version of libuv
#endif
  uvchan_handle_t* ch_handle;

#ifdef LIBUV_0X
  ((void)status);
#endif

  ch_handle = (uvchan_handle_t*)handle;

  if (ch_handle->ch->closed) {
    uv_idle_stop(handle);
    ((uvchan_push_cb)(ch_handle->callback))(ch_handle,
                                            UVCHAN_ERR_CHANNEL_CLOSED);

    uvchan_unref(ch_handle->ch);
  } else if ((!ch_handle->ch->poll_required || ch_handle->ch->polling) &&
             (uvchan_queue_push(&ch_handle->ch->queue, ch_handle->element) ==
              UVCHAN_ERR_SUCCESS)) {
    uv_idle_stop(handle);
    ((uvchan_push_cb)(ch_handle->callback))(ch_handle, UVCHAN_ERR_SUCCESS);

    uvchan_unref(ch_handle->ch);
  }
}

void uvchan_start_push(uvchan_handle_t* handle, const void* element,
                       uvchan_push_cb cb) {
  if (cb == 0L) {
    cb = &_uvchan_default_push_cb;
  }

  handle->element = (void*)element;
  handle->callback = (void*)cb;
  uvchan_ref(handle->ch);

  uv_idle_start((uv_idle_t*)handle, _uvchan_start_push_idle_cb);
}

#ifdef LIBUV_0X
static void _uvchan_start_pop_idle_cb(uv_idle_t* handle, int status) {
#elif LIBUV_1X
static void _uvchan_start_pop_idle_cb(uv_idle_t* handle) {
#else
#error callback not defined for unknown version of libuv
#endif
  uvchan_handle_t* ch_handle;

#ifdef LIBUV_0X
  ((void)status);
#endif

  ch_handle = (uvchan_handle_t*)handle;

  if (uvchan_queue_pop(&ch_handle->ch->queue, ch_handle->element) ==
      UVCHAN_ERR_SUCCESS) {
    uv_idle_stop(handle);
    ch_handle->ch->polling--;

    ((uvchan_pop_cb)(ch_handle->callback))(ch_handle, ch_handle->element,
                                           UVCHAN_ERR_SUCCESS);
    uvchan_unref(ch_handle->ch);
  } else if (ch_handle->ch->closed) {
    uv_idle_stop(handle);
    ch_handle->ch->polling--;

    ((uvchan_pop_cb)(ch_handle->callback))(ch_handle, ch_handle->element,
                                           UVCHAN_ERR_CHANNEL_CLOSED);
    uvchan_unref(ch_handle->ch);
  }
}

void uvchan_start_pop(uvchan_handle_t* handle, void* element,
                      uvchan_pop_cb cb) {
  if (cb == 0L) {
    cb = &_uvchan_default_pop_cb;
  }

  handle->element = element;
  handle->callback = (void*)cb;
  handle->ch->polling++;
  uvchan_ref(handle->ch);

  uv_idle_start((uv_idle_t*)handle, _uvchan_start_pop_idle_cb);
}

void _uvchan_default_push_cb(uvchan_handle_t* handle, uvchan_error_t err) {
  ((void)err);

  uv_close((uv_handle_t*)handle, 0L);
}

void _uvchan_default_pop_cb(uvchan_handle_t* handle, void* buffer,
                            uvchan_error_t err) {
  ((void)buffer);
  ((void)err);

  uv_close((uv_handle_t*)handle, 0L);
}
