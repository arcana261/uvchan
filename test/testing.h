#ifndef TEST_TESTING_H__
#define TEST_TESTING_H__

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./config.h"
#include <sys/time.h>

char MSG[2048];
jmp_buf JUMP_BUF;
int ARGC;
char** ARGV;
typedef void (*TEST_FN)(void);
TEST_FN TESTS[1000];
const char* TEST_NAMES[1000];
int TESTS_COUNT=0;
int EXIT_CODE=0;

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

void _t_init(int argc, char** argv) {
  int i;
  ARGC = argc;
  ARGV = argv;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      printf("%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
      printf("\t-h, --help: show this help message\n");
      printf("\t-l, --list: show list of test cases available\n");
      printf("\t-t, --test: run specific tests\n");
      printf("\nsend bug reports to %s\n\n", PACKAGE_BUGREPORT);
    } else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")) {
      printf("%s v%s\n", PACKAGE_NAME, PACKAGE_VERSION);
      printf("\nsend bug reports to %s\n\n", PACKAGE_BUGREPORT);
    }
  }
}

void _t_add(void (*fn)(void), const char* name) {
  TESTS[TESTS_COUNT] = fn;
  TEST_NAMES[TESTS_COUNT] = name;
  TESTS_COUNT++;
}

void _format_dtime(struct timeval *start, struct timeval *stop, char* buffer, size_t n) {
  int delta;

  delta = (((int)(stop->tv_sec - start->tv_sec)) * 1000) + (((int)(stop->tv_usec - start->tv_usec)) / 1000);
  snprintf(buffer, n, "%d ms", delta);
}

void _t_run_test(void (*fn)(void), const char* name) {
  int i;
  int specific_enabled;
  int specific_found;
  struct timeval stop, start;
  char dtime[100];
    
  specific_enabled = 0;
  specific_found = 0;

  for (i = 1; i < ARGC; i++) {
    if (!strcmp(ARGV[i], "-h") || !strcmp(ARGV[i], "--help")) {
      return;
    } else if (!strcmp(ARGV[i], "-l") || !strcmp(ARGV[i], "--list")) {
      printf("\t%s\n", name);
      return;
    } else if (!strcmp(ARGV[i], "-t") || !strcmp(ARGV[i], "--test")) {
      specific_enabled = 1;
    } else if (specific_enabled && !strcmp(ARGV[i], name)) {
      specific_found = 1;
    }
  }

  if (specific_enabled && !specific_found) {
    return;
  }

  printf("%s... ", name);
  fflush(stdout);

  gettimeofday(&start, NULL);

  if (setjmp(JUMP_BUF) != 0) {
    gettimeofday(&stop, NULL);
    _format_dtime(&start, &stop, dtime, sizeof(dtime));

    printf("ERR (%s)\n%s\n", dtime, MSG);
    EXIT_CODE = -1;
  } else {
    fn();

    gettimeofday(&stop, NULL);
    _format_dtime(&start, &stop, dtime, sizeof(dtime));

    printf("OK (%s)\n", dtime);
    fflush(stdout);
  }
}

int _t_run(int argc, char** argv) {
  struct timeval stop, start;
  char dtime[100];
  int i;

  _t_init(argc, argv);
  gettimeofday(&start, NULL);

  for (i = 0; i < TESTS_COUNT; i++) {
    _t_run_test(TESTS[i], TEST_NAMES[i]);
  }

  gettimeofday(&stop, NULL);
  _format_dtime(&start, &stop, dtime, sizeof(dtime));

  printf("\nTests Took: (%s)\n", dtime);

  return EXIT_CODE;
}

#define _T_COMMA ,
#define _T_PARAN_OPEN (
#define _T_PARAN_CLOSE )
#define _T_ASSERT(check, msg) \
  ((check) ? (1) : (_t_fail(__FILE__, __LINE__, msg)))
#define T_ASSERT(expr) (_T_ASSERT((expr), #expr))
#define T_TRUE(expr) (_T_ASSERT((expr), "expected \"" #expr "\" to be true"))
#define T_FALSE(expr) (_T_ASSERT(!(expr), "expected \"" #expr "\" to be false"))
#define T_OK(expr) (_T_ASSERT(!(expr), "expected \"" #expr "\" to be ok"))
#define T_ZERO(expr) (_T_ASSERT(!(expr), "expected \"" #expr "\" to be zero"))
#define T_CMPINT(expr, op, value)              \
  (_T_ASSERT(                                  \
      ((expr)op(value)),                       \
      "expected \"" #expr "\"[=%d] to be " #op \
      " to %d" _T_COMMA _T_PARAN_OPEN expr _T_PARAN_CLOSE _T_COMMA value))
#define T_EQUAL_PTR(expr, value)                                     \
  (_T_ASSERT(((expr) == (value)),                                    \
             "expected \"" #expr                                     \
             "\"[=%X] to be equal to %X" _T_COMMA _T_PARAN_OPEN expr \
                 _T_PARAN_CLOSE _T_COMMA value))
#define T_NULL(expr)             \
  (_T_ASSERT(!(expr),            \
             "expected \"" #expr \
             "\"[=%X] to be null" _T_COMMA _T_PARAN_OPEN expr _T_PARAN_CLOSE))
#define T_NOT_NULL(expr) \
  (_T_ASSERT((expr), "expected \"" #expr "\" to non-null"))
#define T_EQUAL_STR(expr, value)                                             \
  (_T_ASSERT(!(strcmp((expr), (value))),                                     \
             "expected \"" #expr                                             \
             "\"[=\'%s\"] to be equal to \"%s\"" _T_COMMA _T_PARAN_OPEN expr \
                 _T_PARAN_CLOSE _T_COMMA value))
#define T_NOT_EQUAL_STR(expr, value)                                        \
  (_T_ASSERT((strcmp((expr), (value))),                                     \
             "expected \"" #expr                                            \
             "\"[=\'%s\"] to not be equal to \"%s\"" _T_COMMA _T_PARAN_OPEN \
                 expr _T_PARAN_CLOSE _T_COMMA value))
#define T_FAIL(msg) (_t_fail(__FILE__, __LINE__, (msg)))

#define T_RUN(argc, argv) (_t_run((argc), (argv)))
#define T_ADD(test) (_t_add(test, #test))

#endif  // TEST_TESTING_H__
