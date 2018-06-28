#include "./queue.h"

#include <stdio.h>

int test_should_not_pop_from_empty_queue() {
    int elem;
    uvchan_queue* q;

    q = uvchan_queue_new(4, sizeof(int));

    if (uvchan_queue_pop(q, &elem)) {
        return -1;
    }

    uvchan_queue_destroy(q);
    return 0;
}

int main(int argc, char* argv[]) {
    int result;



    printf("test_should_not_pop_from_empty_queue... ");
    if (!test_should_not_pop_from_empty_queue()) {
        printf("ERR\n");
    } else {
        printf("OK\n");
    }
}
