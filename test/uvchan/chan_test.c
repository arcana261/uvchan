#include <math.h>
#include <testing.h>
#include <time.h>
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

typedef struct _data_t {
  uvchan_t* ch;
  uv_loop_t* loop;
  action_t* actions;
  size_t count;
  size_t i;
  time_t recorded_time;
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
  action_t push_routine[3];
  action_t pop_routine[4];

  push_routine[0] = MAKE_ACTION_RECORD_TIME();
  push_routine[1] = MAKE_ACTION_PUSH(12);
  push_routine[2] = MAKE_ACTION_ASSERT_TIME_GT(1000);

  pop_routine[0] = MAKE_ACTION_SLEEP(1000);
  pop_routine[1] = MAKE_ACTION_RECORD_TIME();
  pop_routine[2] = MAKE_ACTION_POP(12);
  pop_routine[3] = MAKE_ACTION_ASSERT_TIME_LT(1000);

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

void _perform_action(data_t* data) {
  uvchan_handle_t* handle;
  uv_timer_t* timer;
  action_t* action;
  time_t now;

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
      data->recorded_time = time(NULL);
      data->i++;
      _perform_action(data);
      break;
    case ACTION_TYPE_ASSERT_TIME_GT:
      now = time(NULL);
      T_CMPINT((now - data->recorded_time) * 1000, >=, action->value);
      data->i++;
      _perform_action(data);
      break;
    case ACTION_TYPE_ASSERT_TIME_LT:
      now = time(NULL);
      T_CMPINT((now - data->recorded_time) * 1000, <, action->value);
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

#ifdef LIBUV_0X
  loop = uv_default_loop();
#elif LIBUV_1X
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
#else
#error unknown operation for unknown version of libuv
#endif

  data.loop = loop;

  _perform_action(&data);

  T_FALSE(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(data.ch);

#ifdef LIBUV_0X
#elif LIBUV_1X
  uv_loop_close(loop);
  free(loop);
#else
#error unknown operation for unknown version of libuv
#endif
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

#ifdef LIBUV_0X
  loop = uv_default_loop();
#elif LIBUV_1X
  loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);
#else
#error unknown operation for unknown version of libuv
#endif

  routine1_data.loop = loop;

  routine2_data = routine1_data;
  routine2_data.actions = routine2;
  routine2_data.count = routine2_count;

  _perform_action(&routine1_data);
  _perform_action(&routine2_data);

  T_FALSE(uv_run(loop, UV_RUN_DEFAULT));

  uvchan_unref(routine1_data.ch);

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
  T_RUN(test_push_after_close_should_fail);
  T_RUN(test_pop_after_close_should_fail);
  T_RUN(test_push_polling);
  T_RUN(test_single_push_pop);
  T_RUN(test_full_push_pop);
  T_RUN(test_closing_should_not_interrupt_pulling);

  return 0;
}
