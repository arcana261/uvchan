#include <uvchan/error.h>
#include <uvchan/select.h>

#include <uv.h>

#include "./config.h"

void uvchan_select_handle_init(uv_loop_t* loop, uvchan_select_handle_t* handle,
                               uvchan_select_cb cb) {
  uv_idle_init(loop, (uv_idle_t*)handle);

  handle->count = 0;
  handle->has_default = 0;
  handle->callback = cb;
}

int _uvchan_select_handle_indexof(uvchan_select_handle_t* handle, int tag) {
  int i;

  for (i = 0; i < handle->count; i++) {
    if (handle->tags[i] == tag) {
      return i;
    }
  }

  return -1;
}

int uvchan_select_handle_add_push(uvchan_select_handle_t* handle, int tag,
                                  uvchan_t* ch, const void* element) {
  if (handle->count >= kUvChanMaxSelect) {
    return UVCHAN_ERR_SELECT_FULL;
  }

  if (_uvchan_select_handle_indexof(handle, tag) >= 0) {
    return UVCHAN_ERR_SELECT_DUPLICATE_TAG;
  }

  handle->tags[handle->count] = tag;
  handle->channels[handle->count] = ch;
  handle->operations[handle->count] = _UVCHAN_OPERATION_PUSH;
  handle->elements[handle->count] = (void*)element;
  handle->count++;

  uvchan_ref(ch);

  return UVCHAN_ERR_SUCCESS;
}

int uvchan_select_handle_add_pop(uvchan_select_handle_t* handle, int tag,
                                 uvchan_t* ch, void* element) {
  if (handle->count >= kUvChanMaxSelect) {
    return UVCHAN_ERR_SELECT_FULL;
  }

  if (_uvchan_select_handle_indexof(handle, tag) >= 0) {
    return UVCHAN_ERR_SELECT_DUPLICATE_TAG;
  }

  handle->tags[handle->count] = tag;
  handle->channels[handle->count] = ch;
  handle->operations[handle->count] = _UVCHAN_OPERATION_POP;
  handle->elements[handle->count] = element;
  handle->count++;

  uvchan_ref(ch);

  return UVCHAN_ERR_SUCCESS;
}

int uvchan_select_handle_add_default(uvchan_select_handle_t* handle, int tag) {
  handle->has_default = 1;
  handle->default_tag = tag;

  return UVCHAN_ERR_SUCCESS;
}

int uvchan_select_handle_remove_tag(uvchan_select_handle_t* handle, int tag) {
  int i;
  int j;

  i = _uvchan_select_handle_indexof(handle, tag);
  if (i < 0) {
    return UVCHAN_ERR_SELECT_TAG_NOTFOUND;
  }

  uvchan_unref(handle->channels[i]);

  j = i + 1;
  while (j < handle->count) {
    handle->channels[i] = handle->channels[j];
    handle->tags[i] = handle->tags[j];
    handle->operations[i] = handle->tags[j];
    handle->elements[i] = handle->elements[j];

    i = j;
    j = i + 1;
  }

  handle->count--;

  return UVCHAN_ERR_SUCCESS;
}

void _uvchan_start_select_fire(uvchan_select_handle_t* handle, int tag,
                               uvchan_error_t err) {
  int i;

  uv_idle_stop((uv_idle_t*)handle);
  for (i = 0; i < handle->count; i++) {
    uvchan_unref(handle->channels[i]);
  }

  ((uvchan_select_cb)handle->callback)(handle, tag, err);
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
      _uvchan_start_select_fire(handle, handle->tags[i],
                                UVCHAN_ERR_CHANNEL_CLOSED);
      return;
    }

    switch (handle->operations[i]) {
      case _UVCHAN_OPERATION_PUSH:
        if ((!ch->poll_required || ch->polling) &&
            (uvchan_queue_push(&ch->queue, element) == UVCHAN_ERR_SUCCESS)) {
          _uvchan_start_select_fire(handle, handle->tags[i],
                                    UVCHAN_ERR_SUCCESS);
          return;
        }
        break;
      case _UVCHAN_OPERATION_POP:
        if (uvchan_queue_pop(&ch->queue, element) == UVCHAN_ERR_SUCCESS) {
          _uvchan_start_select_fire(handle, handle->tags[i],
                                    UVCHAN_ERR_SUCCESS);
          return;
        }
        break;
    }
  }

  if (handle->has_default) {
    _uvchan_start_select_fire(handle, handle->default_tag, UVCHAN_ERR_SUCCESS);
    return;
  }
}

int uvchan_select_handle_start(uvchan_select_handle_t* handle) {
  if (!handle->has_default && handle->count < 1) {
    return UVCHAN_ERR_SELECT_EMPTY;
  }

  uv_idle_start((uv_idle_t*)handle, _uvchan_start_select_idle_cb);
  return UVCHAN_ERR_SUCCESS;
}
