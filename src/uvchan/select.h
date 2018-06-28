#ifndef UVCHAN_SELECT_H__
#define UVCHAN_SELECT_H__

#include <uv.h>
#include <uvchan/chan.h>

#define _UVCHAN_MAX_SELECT 32
#define _UVCHAN_OPERATION_PUSH 1
#define _UVCHAN_OPERATION_POP 2

typedef struct _uvchan_select_handle_t uvchan_select_handle_t;
typedef void (*uvchan_select_cb)(uvchan_select_handle_t* handle);

typedef struct _uvchan_select_handle_t {
    uv_idle_t idle_handle;

    uvchan_t* chan;

    int tags[_UVCHAN_MAX_SELECT];
    int operations[_UVCHAN_MAX_SELECT];
    void* elements[_UVCHAN_MAX_SELECT];
    void* callbacks[_UVCHAN_MAX_SELECT];
    int count;
    int default_index;
    int index;

    void* data;
} uvchan_select_handle_t;

void uvchan_select_handle_init(uv_loop_t* loop, uvchan_select_handle_t* handle);
int uvchan_select_handle_add_push(uvchan_select_handle_t* handle, int tag, const void* element);
int uvchan_select_handle_add_pop(uvchan_select_handle_t* handle, int tag, void* element);
int uvchan_select_handle_add_default(uvchan_select_handle_t* handle, int tag);
void uvchan_select_start(uvchan_select_handle_t* handle);

int uvchan_select_handle_get_result_tag(uvchan_select_handle_t* handle);
void* uvchan_select_handle_get_result_element(uvchan_select_handle_t* handle);
void* uvchan_select_handle_get_element_for_tag(uvchan_select_handle_t* handle, int tag);

#endif  // UVCHAN_SELECT_H__
