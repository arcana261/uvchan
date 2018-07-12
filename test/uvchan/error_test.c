#include <uvchan/error.h>

#include <assert.h>
#include <testing.h>

void test_should_convert_unknown(void) { T_NOT_NULL(uvchan_strerr(-1)); }

void test_success_should_be_zero(void) { assert(UVCHAN_ERR_SUCCESS == 0); }

void test_should_convert_every_error(void) {
  int i;
  const char* unknown;
  const char* err;

  unknown = uvchan_strerr(-1);

  for (i = 0; i < _UVCHAN_ERR_COUNT; i++) {
    err = uvchan_strerr(i);
    T_NOT_NULL(err);
    T_NOT_EQUAL_STR(err, unknown);
  }
}

int main(int argc, char* argv[]) {
  T_ADD(test_should_convert_unknown);
  T_ADD(test_success_should_be_zero);
  T_ADD(test_should_convert_every_error);

  return T_RUN(argc, argv);
}
