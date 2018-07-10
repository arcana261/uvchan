#include <uvchan/select.h>

#include <testing.h>
#include "./config.h"

#define TAG_PUSH 10
#define TAG_POP 20
#define TAG_DEFAULT 30

uv_loop_t* make_loop(void);
void free_loop(uv_loop_t* loop);
void empty_channel(uv_loop_t* loop, uvchan_t* ch, const int* expected, int n);

typedef struct _data_t {
  uv_loop_t* loop;
  uvchan_t* ch;
  const int* expected;
  int expected_count;
  int pop_buffer;
  int pop_expected_value;
} data_t;

void _test_single_push_should_succeed_cb(uvchan_select_handle_t* handle,
                                         int tag, uvchan_error_t err) {
  data_t* data;

  T_CMPINT(tag, ==, TAG_PUSH);
  T_OK(err);
  uv_close((uv_handle_t*)handle, NULL);

  data = (data_t*)handle->data;

  empty_channel(data->loop, data->ch, data->expected, data->expected_count);
}

void _dealloc_cb(uv_handle_t* handle) {
  free(handle);
}

void _test_single_pop_should_succeed_cb(uvchan_select_handle_t* handle, int tag, uvchan_error_t err) {
  data_t* data;

  T_CMPINT(tag, ==, TAG_POP);
  T_OK(err);
  uv_close((uv_handle_t*)handle, _dealloc_cb);

  data = (data_t*)handle->data;
  T_CMPINT(data->pop_buffer, ==, data->pop_expected_value);

  empty_channel(data->loop, data->ch, data->expected, data->expected_count);
}

void _test_pop_should_not_have_been_called(uvchan_select_handle_t* handle,
                                           int tag, uvchan_error_t err) {
  T_CMPINT(tag, ==, TAG_POP);
  T_OK(err);
  T_FAIL("pop should not have been called");
}

void _test_default_should_not_have_been_called(uvchan_select_handle_t* handle,
                                               int tag, uvchan_error_t err) {
  T_CMPINT(tag, ==, TAG_DEFAULT);
  T_OK(err);
  T_FAIL("default should not have been called");
}

void _test_default_should_be_called(uvchan_select_handle_t* handle, int tag, uvchan_error_t err) {
  data_t* data;

  T_CMPINT(tag, ==, TAG_DEFAULT);
  T_OK(err);

  uv_close((uv_handle_t*)handle, NULL);

  data = (data_t*)handle->data;
  empty_channel(data->loop, data->ch, data->expected, data->expected_count);
}

void test_single_push_should_succeed(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;
  int value;
  int expected_value;

  value = expected_value = 12;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(loop, &handle, _test_single_push_should_succeed_cb);
  data.loop = loop;
  data.ch = ch;
  data.expected = &expected_value;
  data.expected_count = 1;
  handle.data = &data;
  T_OK(uvchan_select_handle_add_push(&handle, TAG_PUSH, ch, &value));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void _test_push_to_empty_channel_should_not_call_pop(
    uvchan_select_handle_t* handle, int tag, uvchan_error_t err) {
  if (tag == TAG_PUSH) {
    _test_single_push_should_succeed_cb(handle, tag, err);
  } else {
    _test_pop_should_not_have_been_called(handle, tag, err);
  }
}

void test_push_to_empty_channel_should_not_call_pop(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;
  int push_value;
  int pop_value;
  int expected_value;

  push_value = expected_value = 123;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(loop, &handle,
                            _test_push_to_empty_channel_should_not_call_pop);
  data.loop = loop;
  data.ch = ch;
  data.expected = &push_value;
  data.expected_count = 1;
  handle.data = &data;

  T_OK(uvchan_select_handle_add_push(&handle, TAG_PUSH, ch, &push_value));
  T_OK(uvchan_select_handle_add_pop(&handle, TAG_POP, ch, &pop_value));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void _test_push_to_empty_channel_should_not_call_default(
    uvchan_select_handle_t* handle, int tag, uvchan_error_t err) {
  if (tag == TAG_PUSH) {
    _test_single_push_should_succeed_cb(handle, tag, err);
  } else {
    _test_default_should_not_have_been_called(handle, tag, err);
  }
}

void test_push_to_empty_channel_should_not_call_default(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;
  int value;
  int expected_value;

  value = expected_value = 12;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(
      loop, &handle, _test_push_to_empty_channel_should_not_call_default);
  data.loop = loop;
  data.ch = ch;
  data.expected = &expected_value;
  data.expected_count = 1;
  handle.data = &data;

  T_OK(uvchan_select_handle_add_push(&handle, TAG_PUSH, ch, &value));
  T_OK(uvchan_select_handle_add_default(&handle, TAG_DEFAULT));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void test_pop_from_empty_queue_should_call_default(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;
  int value;

  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(
      loop, &handle, _test_default_should_be_called);
  data.loop = loop;
  data.ch = ch;
  data.expected = 0L;
  data.expected_count = 0;
  handle.data = &data;

  T_OK(uvchan_select_handle_add_pop(&handle, TAG_DEFAULT, ch, &value));
  T_OK(uvchan_select_handle_add_default(&handle, TAG_DEFAULT));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void test_single_default_should_be_called(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_select_handle_t handle;
  data_t data;

  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  uvchan_select_handle_init(
      loop, &handle, _test_default_should_be_called);
  data.loop = loop;
  data.ch = ch;
  data.expected = 0L;
  data.expected_count = 0;
  handle.data = &data;

  T_OK(uvchan_select_handle_add_default(&handle, TAG_DEFAULT));
  T_OK(uvchan_select_handle_start(&handle));

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void _test_single_pop(uvchan_handle_t* handle, uvchan_error_t err) {
  data_t* data;
  uvchan_select_handle_t* select_handle;

  T_OK(err);
  uv_close((uv_handle_t*)handle, NULL);

  data = (data_t*)handle->data;
  data->expected = 0L;
  data->expected_count = 0;
  data->pop_expected_value = 21;

  select_handle = (uvchan_select_handle_t*)malloc(sizeof(uvchan_select_handle_t));
  uvchan_select_handle_init(data->loop, select_handle, _test_single_pop_should_succeed_cb);
  T_OK(uvchan_select_handle_add_pop(select_handle, TAG_POP, data->ch, &data->pop_buffer));
  T_OK(uvchan_select_handle_start(select_handle));
}

void test_single_pop(void) {
  uvchan_t* ch;
  uv_loop_t* loop;
  uvchan_handle_t handle;
  data_t data;
  int value;
  
  value = 21;
  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));

  uvchan_handle_init(loop, &handle, ch);
  data.loop = loop;
  data.ch = ch;
  handle.data = &data;
  uvchan_start_push(&handle, &value, _test_single_pop);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(ch);
  free_loop(loop);
}

void _empty_channel_pop(uv_loop_t* loop, uvchan_t* ch, const int* expected,
                        int n);

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
  const int* next_expected;
  int next_n;
  data_t* data;

  T_TRUE(err == UVCHAN_ERR_SUCCESS || err == UVCHAN_ERR_CHANNEL_CLOSED);

  data = (data_t*)handle->data;
  loop = data->loop;
  ch = data->ch;

  if (err == UVCHAN_ERR_SUCCESS) {
    T_CMPINT(data->expected_count, >, 0);
    T_CMPINT(*((int*)buffer), ==, data->expected[0]);
    next_expected = data->expected + 1;
    next_n = data->expected_count - 1;
  } else {
    T_CMPINT(data->expected_count, ==, 0);
  }

  uv_close((uv_handle_t*)handle, _empty_channel_pop_close_handle_cb);
  free(buffer);
  free(handle->data);

  if (err != UVCHAN_ERR_SUCCESS) {
    return;
  }

  _empty_channel_pop(loop, ch, next_expected, next_n);
}

void _empty_channel_pop(uv_loop_t* loop, uvchan_t* ch, const int* expected,
                        int n) {
  uvchan_handle_t* handle;
  int* buffer;
  data_t* data;

  handle = (uvchan_handle_t*)malloc(sizeof(uvchan_handle_t));
  buffer = (int*)malloc(sizeof(int));
  uvchan_handle_init(loop, handle, ch);
  data = (data_t*)malloc(sizeof(data_t));
  data->loop = loop;
  data->ch = ch;
  data->expected = expected;
  data->expected_count = n;
  handle->data = data;
  uvchan_start_pop(handle, buffer, _empty_channel_pop_cb);
}

void empty_channel(uv_loop_t* loop, uvchan_t* ch, const int* expected, int n) {
  uvchan_close(ch);
  _empty_channel_pop(loop, ch, expected, n);
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
  T_RUN(test_push_to_empty_channel_should_not_call_pop);
  T_RUN(test_push_to_empty_channel_should_not_call_default);
  T_RUN(test_pop_from_empty_queue_should_call_default);
  T_RUN(test_single_default_should_be_called);
  T_RUN(test_single_pop);

  return 0;
}
