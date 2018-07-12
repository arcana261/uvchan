#include <math.h>
#include <sys/time.h>
#include <testing.h>
#include <uvchan/chan.h>
#include "./config.h"

#define ACTION_TYPE_PUSH 0
#define ACTION_TYPE_CLOSE 1
#define ACTION_TYPE_SLEEP 2
#define ACTION_TYPE_RECORD_TIME 3
#define ACTION_TYPE_ASSERT_TIME_GT 4
#define ACTION_TYPE_ASSERT_TIME_LT 5
#define ACTION_TYPE_POP 6

typedef struct _action_t {
  int type;
  int value;
  uvchan_error_t expected_result;
  int expected_value;
} action_t;

action_t make_action(int type, int value, uvchan_error_t expected_result,
                     int expected_value) {
  action_t result;
  result.type = type;
  result.value = value;
  result.expected_result = expected_result;
  result.expected_value = expected_value;

  return result;
}

#define MAKE_ACTION_CLOSE() make_action(ACTION_TYPE_CLOSE, 0, 0, 0)
#define MAKE_ACTION_PUSH(value) \
  make_action(ACTION_TYPE_PUSH, (value), UVCHAN_ERR_SUCCESS, 0)
#define MAKE_ACTION_FAILING_PUSH(value) \
  make_action(ACTION_TYPE_PUSH, (value), UVCHAN_ERR_CHANNEL_CLOSED, 0)
#define MAKE_ACTION_POP(value) \
  make_action(ACTION_TYPE_POP, 0, UVCHAN_ERR_SUCCESS, (value))
#define MAKE_ACTION_FAILING_POP() \
  make_action(ACTION_TYPE_POP, 0, UVCHAN_ERR_CHANNEL_CLOSED, 0)
#define MAKE_ACTION_RECORD_TIME() make_action(ACTION_TYPE_RECORD_TIME, 0, 0, 0)
#define MAKE_ACTION_ASSERT_TIME_GT(value) \
  make_action(ACTION_TYPE_ASSERT_TIME_GT, (value), 0, 0)
#define MAKE_ACTION_ASSERT_TIME_LT(value) \
  make_action(ACTION_TYPE_ASSERT_TIME_LT, (value), 0, 0)
#define MAKE_ACTION_SLEEP(value) make_action(ACTION_TYPE_SLEEP, (value), 0, 0)

uv_loop_t* make_loop(void);
void free_loop(uv_loop_t* loop);

typedef struct _data_t {
  uvchan_t* ch;
  uv_loop_t* loop;
  action_t* actions;
  size_t count;
  size_t i;
  struct timeval recorded_time;
} data_t;

void _test_using(action_t* actions, size_t count, size_t num_elements);
void _test_coroutine_using(action_t* routine1, size_t routine1_count,
                           action_t* routine2, size_t routine2_count,
                           size_t num_elements);

void test_single_push_should_succeed(void) {
  action_t actions[4];
  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_PUSH(7);
  actions[2] = MAKE_ACTION_ASSERT_TIME_LT(1000);
  actions[3] = MAKE_ACTION_POP(7);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 1);
}

void test_init_handle_should_initialize_data_with_null(void) {
  uv_loop_t* loop;
  uvchan_t* ch;
  uvchan_handle_t handle;

  loop = make_loop();
  ch = uvchan_new(1, sizeof(int));
  handle.data = ch;
  uvchan_handle_init(loop, &handle, ch);
  T_NULL(handle.data);

  uvchan_unref(ch);
  free_loop(loop);
}

void test_push_after_close_should_fail(void) {
  action_t actions[5];
  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_CLOSE();
  actions[2] = MAKE_ACTION_ASSERT_TIME_LT(1000);
  actions[3] = MAKE_ACTION_FAILING_PUSH(7);
  actions[4] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 1);
}

void test_pop_after_close_should_fail(void) {
  action_t actions[5];
  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_CLOSE();
  actions[2] = MAKE_ACTION_ASSERT_TIME_LT(1000);
  actions[3] = MAKE_ACTION_FAILING_POP();
  actions[4] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 1);
}

void test_push_polling(void) {
  action_t push_routine[6];
  action_t pop_routine[8];

  push_routine[0] = MAKE_ACTION_RECORD_TIME();
  push_routine[1] = MAKE_ACTION_PUSH(12);
  push_routine[2] = MAKE_ACTION_ASSERT_TIME_GT(100);
  push_routine[3] = MAKE_ACTION_RECORD_TIME();
  push_routine[4] = MAKE_ACTION_PUSH(20);
  push_routine[5] = MAKE_ACTION_ASSERT_TIME_GT(100);

  pop_routine[0] = MAKE_ACTION_SLEEP(150);
  pop_routine[1] = MAKE_ACTION_RECORD_TIME();
  pop_routine[2] = MAKE_ACTION_POP(12);
  pop_routine[3] = MAKE_ACTION_SLEEP(150);
  pop_routine[4] = MAKE_ACTION_RECORD_TIME();
  pop_routine[5] = MAKE_ACTION_ASSERT_TIME_LT(1000);
  pop_routine[6] = MAKE_ACTION_POP(20);
  pop_routine[7] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_coroutine_using(push_routine, sizeof(push_routine) / sizeof(action_t),
                        pop_routine, sizeof(pop_routine) / sizeof(action_t), 0);
}

void test_single_push_pop(void) {
  action_t actions[4];
  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_PUSH(19);
  actions[2] = MAKE_ACTION_POP(19);
  actions[3] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 1);
}

void test_full_queue_waiting(void) {
  action_t push_routine[5];
  action_t pop_routine[5];

  push_routine[0] = MAKE_ACTION_RECORD_TIME();
  push_routine[1] = MAKE_ACTION_PUSH(10);
  push_routine[2] = MAKE_ACTION_ASSERT_TIME_LT(1000);
  push_routine[3] = MAKE_ACTION_PUSH(20);
  push_routine[4] = MAKE_ACTION_ASSERT_TIME_GT(1000);

  pop_routine[0] = MAKE_ACTION_SLEEP(1000);
  pop_routine[0] = MAKE_ACTION_RECORD_TIME();
  pop_routine[1] = MAKE_ACTION_POP(10);
  pop_routine[2] = MAKE_ACTION_POP(20);
  pop_routine[0] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_coroutine_using(push_routine, sizeof(push_routine) / sizeof(action_t),
                        pop_routine, sizeof(pop_routine) / sizeof(action_t), 1);
}

void test_full_push_pop(void) {
  action_t actions[8];

  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_PUSH(1);
  actions[2] = MAKE_ACTION_PUSH(2);
  actions[3] = MAKE_ACTION_PUSH(3);
  actions[4] = MAKE_ACTION_POP(1);
  actions[5] = MAKE_ACTION_POP(2);
  actions[6] = MAKE_ACTION_POP(3);
  actions[7] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 3);
}

void test_closing_should_not_interrupt_pulling(void) {
  action_t actions[8];

  actions[0] = MAKE_ACTION_RECORD_TIME();
  actions[1] = MAKE_ACTION_PUSH(1);
  actions[2] = MAKE_ACTION_PUSH(2);
  actions[3] = MAKE_ACTION_CLOSE();
  actions[4] = MAKE_ACTION_POP(1);
  actions[5] = MAKE_ACTION_POP(2);
  actions[6] = MAKE_ACTION_FAILING_POP();
  actions[7] = MAKE_ACTION_ASSERT_TIME_LT(1000);

  _test_using(actions, sizeof(actions) / sizeof(action_t), 2);
}

static void _test_push_should_keep_channel_reference_push_cb(
    uvchan_handle_t* handle, uvchan_error_t err) {
  int buffer;

  T_CMPINT(handle->ch->reference_count, ==, 1);
  uvchan_queue_pop(&handle->ch->queue, &buffer);
  uv_close((uv_handle_t*)handle, NULL);
}

void test_push_should_keep_channel_reference(void) {
  uv_loop_t* loop;
  uvchan_t* chan;
  uvchan_handle_t push_handle;
  int value;

  loop = make_loop();
  chan = uvchan_new(1, sizeof(int));
  value = 12;
  uvchan_handle_init(loop, &push_handle, chan);
  uvchan_start_push(&push_handle, &value,
                    _test_push_should_keep_channel_reference_push_cb);

  uvchan_unref(chan);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  free_loop(loop);
}

static void _test_pop_should_keep_channel_reference_pop_cb(
    uvchan_handle_t* handle, void* buffer, uvchan_error_t err) {
  uv_close((uv_handle_t*)handle, NULL);

  T_CMPINT(handle->ch->reference_count, ==, 1);
}

void test_pop_should_keep_channel_reference(void) {
  uv_loop_t* loop;
  uvchan_t* chan;
  uvchan_handle_t pop_handle;
  int value;
  int buffer;

  value = 12;
  loop = make_loop();
  chan = uvchan_new(1, sizeof(int));
  uvchan_queue_push(&chan->queue, &value);
  uvchan_handle_init(loop, &pop_handle, chan);

  uvchan_start_pop(&pop_handle, &buffer,
                   _test_pop_should_keep_channel_reference_pop_cb);
  uvchan_unref(chan);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  free_loop(loop);
}

static void _test_push_should_support_null_callback_cb(uvchan_handle_t* handle,
                                                       void* buffer,
                                                       uvchan_error_t err) {
  T_CMPINT(err, ==, UVCHAN_ERR_SUCCESS);
  uv_close((uv_handle_t*)handle, NULL);
}

void test_push_should_support_null_callback(void) {
  uv_loop_t* loop;
  uvchan_t* chan;
  uvchan_handle_t push_handle;
  uvchan_handle_t pop_handle;
  int value;
  int buffer;

  value = 12;
  loop = make_loop();
  chan = uvchan_new(1, sizeof(int));
  uvchan_handle_init(loop, &push_handle, chan);
  uvchan_handle_init(loop, &pop_handle, chan);
  uvchan_start_push(&push_handle, &value, NULL);
  uvchan_start_pop(&pop_handle, &buffer,
                   _test_push_should_support_null_callback_cb);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(chan);
  free_loop(loop);
}

static void _test_push_should_support_null_callback_polling_cb(
    uvchan_handle_t* handle, void* buffer, uvchan_error_t err) {
  T_CMPINT(err, ==, UVCHAN_ERR_SUCCESS);
  uv_close((uv_handle_t*)handle, NULL);
}

void test_push_should_support_null_callback_polling(void) {
  uv_loop_t* loop;
  uvchan_t* chan;
  uvchan_handle_t push_handle;
  uvchan_handle_t pop_handle;
  int value;
  int buffer;

  value = 12;
  loop = make_loop();
  chan = uvchan_new(0, sizeof(int));
  uvchan_handle_init(loop, &push_handle, chan);
  uvchan_handle_init(loop, &pop_handle, chan);
  uvchan_start_push(&push_handle, &value, NULL);
  uvchan_start_pop(&pop_handle, &buffer,
                   _test_push_should_support_null_callback_polling_cb);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(chan);
  free_loop(loop);
}

static void _test_pop_should_support_null_callback_cb(uvchan_handle_t* handle,
                                                      uvchan_error_t err) {
  T_CMPINT(err, ==, UVCHAN_ERR_SUCCESS);
  uv_close((uv_handle_t*)handle, NULL);
}

void test_pop_should_support_null_callback(void) {
  uv_loop_t* loop;
  uvchan_t* chan;
  uvchan_handle_t push_handle;
  uvchan_handle_t pop_handle;
  int value;
  int buffer;

  value = 12;
  loop = make_loop();
  chan = uvchan_new(1, sizeof(int));
  uvchan_handle_init(loop, &push_handle, chan);
  uvchan_handle_init(loop, &pop_handle, chan);
  uvchan_start_push(&push_handle, &value,
                    _test_pop_should_support_null_callback_cb);
  uvchan_start_pop(&pop_handle, &buffer, NULL);

  T_OK(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(chan);
  free_loop(loop);
}

static void _push_callback(uvchan_handle_t* handle, uvchan_error_t ok);
static void _pop_callback(uvchan_handle_t* handle, void* element,
                          uvchan_error_t ok);
#ifdef LIBUV_0X
static void _timer_callback(uv_timer_t* handle, int status);
#elif LIBUV_1X
static void _timer_callback(uv_timer_t* handle);
#else
#error unknown callback for unknown version of libuv
#endif
static void _dealloc_callback(uv_handle_t* handle);

int get_elapsed_time(struct timeval* prev, struct timeval* now) {
  return ((int)(now->tv_sec - prev->tv_sec)) * 1000 +
         (((int)(now->tv_usec - prev->tv_usec)) / 1000);
}

void _perform_action(data_t* data) {
  uvchan_handle_t* handle;
  uv_timer_t* timer;
  action_t* action;
  struct timeval now;

  if (data->i >= data->count) {
    return;
  }

  action = data->actions + data->i;

  switch (action->type) {
    case ACTION_TYPE_PUSH:
      handle = (uvchan_handle_t*)malloc(sizeof(uvchan_handle_t));
      uvchan_handle_init(data->loop, handle, data->ch);
      handle->data = data;
      uvchan_start_push(handle, &action->value, _push_callback);
      break;
    case ACTION_TYPE_POP:
      handle = (uvchan_handle_t*)malloc(sizeof(uvchan_handle_t));
      uvchan_handle_init(data->loop, handle, data->ch);
      handle->data = data;
      uvchan_start_pop(handle, &action->value, _pop_callback);
      break;
    case ACTION_TYPE_CLOSE:
      uvchan_close(data->ch);
      data->i++;
      _perform_action(data);
      break;
    case ACTION_TYPE_SLEEP:
      timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
      timer->data = data;
      uv_timer_init(data->loop, timer);
      uv_timer_start(timer, _timer_callback, action->value, action->value);
      break;
    case ACTION_TYPE_RECORD_TIME:
      gettimeofday(&data->recorded_time, NULL);
      data->i++;
      _perform_action(data);
      break;
    case ACTION_TYPE_ASSERT_TIME_GT:
      gettimeofday(&now, NULL);
      T_CMPINT(get_elapsed_time(&data->recorded_time, &now), >=, action->value);
      data->i++;
      _perform_action(data);
      break;
    case ACTION_TYPE_ASSERT_TIME_LT:
      gettimeofday(&now, NULL);
      T_CMPINT(get_elapsed_time(&data->recorded_time, &now), <, action->value);
      data->i++;
      _perform_action(data);
      break;
  }
}

static void _push_callback(uvchan_handle_t* handle, uvchan_error_t ok) {
  data_t* data;
  action_t* action;

  data = (data_t*)handle->data;
  action = data->actions + data->i;

  T_CMPINT(ok, ==, action->expected_result);
  uv_close((uv_handle_t*)handle, _dealloc_callback);

  data->i++;
  _perform_action(data);
}

static void _pop_callback(uvchan_handle_t* handle, void* element,
                          uvchan_error_t ok) {
  data_t* data;
  action_t* action;

  data = (data_t*)handle->data;
  action = data->actions + data->i;

  T_CMPINT(ok, ==, action->expected_result);
  T_EQUAL_PTR(element, &action->value);
  T_CMPINT(action->value, ==, action->expected_value);
  uv_close((uv_handle_t*)handle, _dealloc_callback);

  data->i++;
  _perform_action(data);
}

#ifdef LIBUV_0X
static void _timer_callback(uv_timer_t* handle, int status) {
#elif LIBUV_1X
static void _timer_callback(uv_timer_t* handle) {
#else
#error unknown callback for unknown version of libuv
#endif
  data_t* data;

#ifdef LIBUV_0X
  ((void)status);
#endif

  data = (data_t*)handle->data;

  uv_timer_stop(handle);
  uv_close((uv_handle_t*)handle, _dealloc_callback);

  data->i++;
  _perform_action(data);
}

static void _dealloc_callback(uv_handle_t* handle) { free(handle); }

void _test_using(action_t* actions, size_t count, size_t num_elements) {
  data_t data;
  uv_loop_t* loop;

  data.ch = uvchan_new(num_elements, sizeof(int));
  data.actions = actions;
  data.count = count;
  data.i = 0;

  loop = make_loop();

  data.loop = loop;

  T_CMPINT(data.ch->reference_count, ==, 1);
  _perform_action(&data);

  T_FALSE(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(data.ch);
  free_loop(loop);
}

void _test_coroutine_using(action_t* routine1, size_t routine1_count,
                           action_t* routine2, size_t routine2_count,
                           size_t num_elements) {
  data_t routine1_data;
  data_t routine2_data;
  uv_loop_t* loop;

  routine1_data.ch = uvchan_new(num_elements, sizeof(int));
  routine1_data.actions = routine1;
  routine1_data.count = routine1_count;
  routine1_data.i = 0;

  loop = make_loop();

  routine1_data.loop = loop;

  routine2_data = routine1_data;
  routine2_data.actions = routine2;
  routine2_data.count = routine2_count;

  _perform_action(&routine1_data);
  _perform_action(&routine2_data);

  T_FALSE(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(routine1_data.ch);

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
  T_ADD(test_single_push_should_succeed);
  T_ADD(test_push_after_close_should_fail);
  T_ADD(test_pop_after_close_should_fail);
  T_ADD(test_push_polling);
  T_ADD(test_single_push_pop);
  T_ADD(test_full_push_pop);
  T_ADD(test_closing_should_not_interrupt_pulling);
  T_ADD(test_push_should_keep_channel_reference);
  T_ADD(test_pop_should_keep_channel_reference);
  T_ADD(test_init_handle_should_initialize_data_with_null);
  T_ADD(test_push_should_support_null_callback);
  T_ADD(test_push_should_support_null_callback_polling);
  T_ADD(test_pop_should_support_null_callback);

  return T_RUN(argc, argv);
}
