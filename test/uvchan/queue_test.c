#include <testing.h>
#include <uvchan/queue.h>

void test_pop_should_not_read_from_empty(void) {
  uvchan_queue q;
  int result;

  uvchan_queue_init(&q, 0, sizeof(int));
  T_FALSE(uvchan_queue_pop(&q, &result));
  uvchan_queue_destroy(&q);
}

void test_push_should_not_push_to_empty(void) {
  uvchan_queue q;
  int value;

  value = 5;

  uvchan_queue_init(&q, 0, sizeof(int));
  T_FALSE(uvchan_queue_push(&q, &value));
  uvchan_queue_destroy(&q);
}

void test_push_pop_single_element(void) {
  uvchan_queue q;
  int value;
  int result;

  value = 5;

  uvchan_queue_init(&q, 10, sizeof(int));
  T_TRUE(uvchan_queue_push(&q, &value));
  T_TRUE(uvchan_queue_pop(&q, &result));
  T_CMPINT(5, ==, result);
  T_FALSE(uvchan_queue_pop(&q, &result));
  uvchan_queue_destroy(&q);
}

void test_push_pop_full(void) {
  uvchan_queue q;
  int i;
  int result;

  uvchan_queue_init(&q, 10, sizeof(int));
  for (i = 0; i < 10; i++) {
    T_TRUE(uvchan_queue_push(&q, &i));
  }
  T_FALSE(uvchan_queue_push(&q, &i));
  for (i = 0; i < 10; i++) {
    T_TRUE(uvchan_queue_pop(&q, &result));
    T_CMPINT(result, ==, i);
  }
  T_FALSE(uvchan_queue_pop(&q, &result));
  uvchan_queue_destroy(&q);
}

int main(int argc, char* argv[]) {
  T_INIT(argc, argv);

  T_RUN(test_pop_should_not_read_from_empty);
  T_RUN(test_push_should_not_push_to_empty);
  T_RUN(test_push_pop_single_element);
  T_RUN(test_push_pop_full);

  return 0;
}
