#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define SIZE 16

#define STRING 0
#define NORMAL 1

int Fork() {
    int pid;
    if ((pid = fork()) < 0) {
        fprintf(1, "fork error\n");
        exit(1);
    };
    return pid;
}

void Exec(char *cmd, char* argv[]) {
    if (exec(cmd, argv) < 0) {
        printf("exec error.\n");
        exit(1);
    }
}

/* extend capacity of ptr to double */
char* extend(char *ptr, int *size) {
    *size *= 2;
    char *new_ptr = (char *) malloc(*size);
    strcpy(new_ptr, ptr);
    free(ptr);
    return new_ptr;
}

int read_line(char *argv[]) {
    static int mode = NORMAL;
    int i = 0, j = 0;
    int n = 0;
    char *param = (char *) malloc(SIZE);
    int size = SIZE;
    char buf;
    while (read(0, &buf, 1) > 0) {
        switch (buf) {
            case '\n':
                argv[i++] = param;
                return n;
            case ' ':
                argv[i++] = param;
                j = 0;
                param = (char *) malloc(SIZE);
                size = SIZE;
                break;
            case '"':
                mode = mode == NORMAL ? STRING : NORMAL;
                break;
            case '\\':
                if (mode == STRING) {
                    read(0, &buf, 1);
                    if (buf == 'n') {
                        argv[i] = param;
                        return n;
                    }
                    param[j++] = buf; n++;
                    if (j == size) {
                        extend(param, &size);
                    }
                    break;
                }
                param[j++] = '\\'; n++;
                if (j == size) {
                    extend(param, &size);
                }
                break;
            default:
                param[j++] = buf; n++;
                if (j == size) {
                    extend(param, &size);
                }
        }
    }

    if (mode == STRING) {
        printf("xargs: parse line error\n");
        exit(1);
    }
    argv[i] = 0x0;
    return n;
}

int main(int argc, char *argv[]) {
    while (1) {
        char *args[MAXARG]; 
        if (read_line(args) == 0) 
            break;
        char *tmp[MAXARG];
        int i = 0;
        while (argv[i + 1]) {
            tmp[i] = argv[i + 1]; i++;
        }
        int j = 0;
        while (args[j]) {
            tmp[i++] = args[j++];
        }
        if (Fork() == 0) {
            Exec(argv[1], tmp);
        }
        int status;
        wait(&status);
    }
    exit(0);
}