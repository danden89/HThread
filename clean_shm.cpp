#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "inc/hglobal.h"

using namespace hthread;
int main(int argc, char* argv[]) {
    if (argc == 1) {
    *ptask_number = 1;
    } else {
        *ptask_number = atoi(argv[1]) + 1;
    }

    printf("task number: %d\n", *ptask_number);
}
