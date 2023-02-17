#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc, char *argv[]) {
    if (!argv[1]) {
        fprintf(1, "sleep: lack of argvment\n");
        exit(1);
    }
    int second = atoi(argv[1]);
    sleep(second);
    exit(0);
}