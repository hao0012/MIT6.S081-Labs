#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int Fork() {
    int pid;
    if ((pid = fork()) < 0) {
        // fprintf(1, "fork error\n");
        exit(1);
    };
    return pid;
}

int main(int argc, char *argv[]) {
    int p2c[2], c2p[2];
    char buf;

    pipe(p2c); pipe(c2p);
    if (Fork() == 0) { /* in child */
        close(p2c[1]);
        close(c2p[0]);
        read(p2c[0], &buf, 1);
        printf("%d: received ping\n", getpid());
        write(c2p[1], &buf, 1);
        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        close(p2c[0]);
        close(c2p[1]);
        write(p2c[1], "p", 1);
        read(c2p[0], &buf, 1);
        printf("%d: received pong\n", getpid());
        close(p2c[1]);
        close(c2p[0]);
        exit(0);
    }

}