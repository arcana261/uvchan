#include <uvchan/select.h>

#include <uv.h>

#include "./config.h"

void uvchan_select_handle_init(uv_loop_t* loop, uvchan_select_handle_t* handle,
                               uvchan_select_cb cb) {
  uv_idle_init(loop, (uv_idle_t*)handle);

  handle->count = 0;
  handle->has_default = 0;
  handle->index = 0;
  handle->captured_default = 0;
  handle->callback = cb;
  handle->ok = 0;
}

int _uvchan_select_handle_indexof(uvchan_select_handle_t* handle, int tag) {
    int i;

    for (i=0; i < handle->count;i++) {
        if (handle->tags[i] == tag) {
            return i;
        }
    }

    return -1;
}

int uvchan_select_handle_add_push(uvchan_select_handle_t* handle,
                                  int tag, uvchan_t* ch, const void* element) {
  if (handle->count >= kUvChanMaxSelect) {
    return 0;
  }

  if (_uvchan_select_handle_indexof(handle, tag) >= 0) {
      return 0;
  }

  handle->tags[handle->count] = tag;
  handle->channels[handle->count] = ch;
  handle->operations[handle->count] = _UVCHAN_OPERATION_PUSH;
  handle->elements[handle->count] = (void*)element;
  handle->count++;

  uvchan_ref(ch);

  return 1;
}

int uvchan_select_handle_add_pop(uvchan_select_handle_t* handle,
                                 int tag, uvchan_t* ch, void* element) {
  if (handle->count >= kUvChanMaxSelect) {
    return 0;
  }

  if (_uvchan_select_handle_indexof(handle, tag) >= 0) {
      return 0;
  }

  handle->tags[handle->count] = tag;
  handle->channels[handle->count] = ch;
  handle->operations[handle->count] = _UVCHAN_OPERATION_POP;
  handle->elements[handle->count] = element;
  handle->count++;

  uvchan_ref(ch);

  return 1;
}

int uvchan_select_handle_add_default(uvchan_select_handle_t* handle, int tag) {
  handle->has_default = 1;
  handle->default_tag = tag;

  return 1;
}

void _uvchan_start_select_fire(uvchan_select_handle_t* handle) {
  int i;

  uv_idle_stop((uv_idle_t*)handle);
  for (i = 0; i < handle->count; i++) {
    uvchan_unref(handle->channels[i]);
  }

  handle->callback(handle);
}

#ifdef LIBUV_0X
static void _uvchan_start_select_idle_cb(uv_idle_t* idle_handle, int status) {
#elif LIBUV_1X
static void _uvchan_start_select_idle_cb(uv_idle_t* handle) {
#else
#error callback not defined for unknown version of libuv
#endif
  uvchan_select_handle_t* handle;
  uvchan_t* ch;
  void* element;
  int i;

#ifdef LIBUV_0X
  ((void)status);
#endif

  handle = (uvchan_select_handle_t*)idle_handle;

  for (i = 0; i < handle->count; i++) {
    ch = handle->channels[i];
    element = handle->elements[i];

    if (ch->closed) {
      handle->index = i;
      handle->captured_default = 0;
      handle->ok = 0;

      _uvchan_start_select_fire(handle);
      return;
    }

    switch (handle->operations[i]) {
      case _UVCHAN_OPERATION_PUSH:
        if ((!ch->poll_required || ch->polling) &&
            uvchan_queue_push(&ch->queue, element)) {
          handle->index = i;
          handle->captured_default = 0;
          handle->ok = 1;

          _uvchan_start_select_fire(handle);
          return;
        }
        break;
      case _UVCHAN_OPERATION_POP:
        if (uvchan_queue_pop(&ch->queue, element)) {
          handle->index = i;
          handle->captured_default = 0;
          handle->ok = 1;

          _uvchan_start_select_fire(handle);
          return;
        }
        break;
    }
  }

  if (handle->has_default) {
      handle->captured_default = 1;
      handle->ok = 1;

      _uvchan_start_select_fire(handle);
      return;
  }
}

int uvchan_select_start(uvchan_select_handle_t* handle) {
    if (handle->count < 1) {
        return 0;
    }

    uv_idle_start((uv_idle_t*)handle, _uvchan_start_select_idle_cb);
    return 1;
}
