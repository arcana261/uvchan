#include <testing.h>
#include "./config.h"

#include <uvchan/chan.h>

void test_uvchan(void) {
    uvchan_t* ch;

    ch = uvchan_new(1, sizeof(int));
    uvchan_unref(ch);
}

int main(int argc, char* argv[]) {
    T_ADD(test_uvchan);

    return T_RUN(argc, argv);
}
