#include <uvchan/chan.h>

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

static void _uvchan_start_push_idle_cb(uv_idle_t* handle) {
  uvchan_handle_t* ch_handle;

  ch_handle = (uvchan_handle_t*)handle;

  if (ch_handle->ch->closed) {
    uv_idle_stop(handle);

    ((uvchan_push_cb)(ch_handle->callback))(ch_handle, 0);
  } else if ((!ch_handle->ch->poll_required || ch_handle->ch->polling) &&
             uvchan_queue_push(&ch_handle->ch->queue, ch_handle->element)) {
    uv_idle_stop(handle);

    ((uvchan_push_cb)(ch_handle->callback))(ch_handle, 1);
  }
}

void uvchan_start_push(uvchan_handle_t* handle, const void* element,
                       uvchan_push_cb cb) {
  handle->element = (void*)element;
  handle->callback = (void*)cb;
  uv_idle_start((uv_idle_t*)handle, _uvchan_start_push_idle_cb);
}

static void _uvchan_start_pop_idle_cb(uv_idle_t* handle) {
  uvchan_handle_t* ch_handle;

  ch_handle = (uvchan_handle_t*)handle;
  ch_handle->ch->polling++;

  if (ch_handle->ch->closed) {
    uv_idle_stop(handle);
    ch_handle->ch->polling--;

    ((uvchan_pop_cb)(ch_handle->callback))(ch_handle, ch_handle->element, 0);
  } else if (uvchan_queue_pop(&ch_handle->ch->queue, ch_handle->element)) {
    uv_idle_stop(handle);
    ch_handle->ch->polling--;

    ((uvchan_pop_cb)(ch_handle->callback))(ch_handle, ch_handle->element, 1);
  }
}

void uvchan_start_pop(uvchan_handle_t* handle, void* element,
                      uvchan_pop_cb cb) {
  handle->element = element;
  handle->callback = (void*)cb;
  uv_idle_start((uv_idle_t*)handle, _uvchan_start_pop_idle_cb);
}
