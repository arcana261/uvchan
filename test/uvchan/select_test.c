#include <uvchan/select.h>

#include <testing.h>
#include "./config.h"

uv_loop_t* make_loop(void);
void free_loop(uv_loop_t* loop);

void test_single_push_should_succeed_cb(uvchan_select_handle_t* handle, int tag,
                                        uvchan_error_t err) {
  T_CMPINT(tag, ==, 10);
  T_OK(err);
  uv_close((uv_handle_t*)handle, NULL);
}

void test_single_push_should_succeed(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  int value;

  value = 12;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(loop, &handle, test_single_push_should_succeed_cb);
  T_OK(uvchan_select_handle_add_push(&handle, 10, ch, &value));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
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
