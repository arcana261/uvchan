#include "./queue.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

char MSG[2048];
jmp_buf JUMP_BUF;
int ARGC;
char** ARGV;

int _t_fail(const char* file, int line, const char* msg, ...) {
  va_list args;
  char TEMP[sizeof(MSG)];

  va_start(args, msg);
  vsnprintf(TEMP, sizeof(TEMP), msg, args);
  va_end(args);

  snprintf(MSG, sizeof(MSG), "[%s:%d] ASSERTION FAILED: %s", file, line, TEMP);
  longjmp(JUMP_BUF, 1);
  return 0;
}

void T_INIT(int argc, char** argv) {
  ARGC = argc;
  ARGV = argv;
}

void _t_run(void (*fn)(void), const char* name) {
  printf("%s... ", name);
  if (setjmp(JUMP_BUF) != 0) {
    printf("ERR\n%s\n", MSG);
  } else {
    fn();
    printf("OK\n");
  }
}

#define _T_COMMA ,
#define _T_ASSERT(check, msg) \
  ((check) ? (1) : (_t_fail(__FILE__, __LINE__, msg)))
#define T_ASSERT(expr) (_T_ASSERT((expr), #expr))
#define T_TRUE(expr) (_T_ASSERT((expr), "expected \"" #expr "\" to be true"))
#define T_FALSE(expr) (_T_ASSERT(!(expr), "expected \"" #expr "\" to be false"))
#define T_CMPINT(expr, op, value)                                        \
  (_T_ASSERT(((expr)op(value)), "expected \"" #expr "\"[=%d] to be " #op \
                                " to %d" _T_COMMA(expr) _T_COMMA value))

#define T_RUN(test) _t_run(test, #test)

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
  T_RUN(test_pop_should_not_read_from_empty);
  T_RUN(test_push_should_not_push_to_empty);
  T_RUN(test_push_pop_single_element);
  T_RUN(test_push_pop_full);
}
