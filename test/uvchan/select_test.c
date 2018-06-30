#include <uvchan/select.h>

#include <testing.h>
#include "./config.h"

uv_loop_t* make_loop(void);
void free_loop(uv_loop_t* loop);
void empty_channel(uv_loop_t* loop, uvchan_t* ch);

typedef struct _data_t {
  uv_loop_t* loop;
  uvchan_t* ch;
} data_t;

void _test_single_push_should_succeed_cb(uvchan_select_handle_t* handle,
                                         int tag, uvchan_error_t err) {
  data_t* data;
  T_CMPINT(tag, ==, 10);
  T_OK(err);
  uv_close((uv_handle_t*)handle, NULL);

  data = (data_t*)handle->data;
  empty_channel(data->loop, data->ch);
}

void test_single_push_should_succeed(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;
  int value;

  value = 12;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(loop, &handle, _test_single_push_should_succeed_cb);
  data.loop = loop;
  data.ch = ch;
  handle.data = &data;
  T_OK(uvchan_select_handle_add_push(&handle, 10, ch, &value));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void _empty_channel_pop(uv_loop_t* loop, uvchan_t* ch);

#ifdef LIBUV_0X
static void _empty_channel_pop_close_handle_cb(uv_handle_t* handle) {
#elif LIBUV_1X
static void _empty_channel_pop_close_handle_cb(uv_handle_t* handle) {
#else
#error unknown callback for unknown version of libuv
#endif

  free(handle);
}

void _empty_channel_pop_cb(uvchan_handle_t* handle, void* buffer,
                           uvchan_error_t err) {
  uv_loop_t* loop;
  uvchan_t* ch;

  uv_close((uv_handle_t*)handle, _empty_channel_pop_close_handle_cb);
  free(buffer);

  loop = ((data_t*)handle->data)->loop;
  ch = ((data_t*)handle->data)->ch;
  free(handle->data);

  if (err != UVCHAN_ERR_SUCCESS) {
    return;
  }

  _empty_channel_pop(loop, ch);
}

void _empty_channel_pop(uv_loop_t* loop, uvchan_t* ch) {
  uvchan_handle_t* handle;
  int* buffer;
  data_t* data;

  handle = (uvchan_handle_t*)malloc(sizeof(uvchan_handle_t));
  buffer = (int*)malloc(sizeof(int));
  uvchan_handle_init(loop, handle, ch);
  data = (data_t*)malloc(sizeof(data_t));
  data->loop = loop;
  data->ch = ch;
  handle->data = data;
  uvchan_start_pop(handle, buffer, _empty_channel_pop_cb);
}

void empty_channel(uv_loop_t* loop, uvchan_t* ch) {
  uvchan_close(ch);
  _empty_channel_pop(loop, ch);
}

uv_loop_t* make_loop(void) {
  uv_loop_t* loop;

#ifdef LIBUV_0X
  loop = uv_default_loop();
#elif LIBUV_1X
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
#else
#error unknown operation for unknown version of libuv
#endif

  return loop;
}

void free_loop(uv_loop_t* loop) {
#ifdef LIBUV_0X
#elif LIBUV_1X
  uv_loop_close(loop);
  free(loop);
#else
#error unknown operation for unknown version of libuv
#endif
}

int main(int argc, char* argv[]) {
  T_INIT(argc, argv);

  T_RUN(test_single_push_should_succeed);

  return 0;
}
