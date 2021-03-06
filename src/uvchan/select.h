#ifndef UVCHAN_SELECT_H__
#define UVCHAN_SELECT_H__

#include <uv.h>
#include <uvchan/chan.h>

#define kUvChanMaxSelect 32
#define _UVCHAN_OPERATION_PUSH 1
#define _UVCHAN_OPERATION_POP 2

typedef struct _uvchan_select_handle_t {
  uv_idle_t idle_handle;

  uvchan_t* channels[kUvChanMaxSelect];
  int tags[kUvChanMaxSelect];
  int operations[kUvChanMaxSelect];
  void* elements[kUvChanMaxSelect];
  void* callback;
  int count;
  int has_default;
  int default_tag;

  void* data;
} uvchan_select_handle_t;

typedef void (*uvchan_select_cb)(uvchan_select_handle_t* handle, int tag,
                                 uvchan_error_t err);

void uvchan_select_handle_init(uv_loop_t* loop, uvchan_select_handle_t* handle,
                               uvchan_select_cb cb);
int uvchan_select_handle_add_push(uvchan_select_handle_t* handle, int tag,
                                  uvchan_t* ch, const void* buffer);
int uvchan_select_handle_add_pop(uvchan_select_handle_t* handle, int tag,
                                 uvchan_t* ch, void* buffer);
int uvchan_select_handle_add_default(uvchan_select_handle_t* handle, int tag);
int uvchan_select_handle_remove_tag(uvchan_select_handle_t* handle, int tag);

int uvchan_select_handle_start(uvchan_select_handle_t* handle);

#endif  // UVCHAN_SELECT_H__
