#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

int Open(const char* file, int flag) {
    int fd;
    if ((fd = open(file, flag)) < 0) {
        fprintf(2, "ls: cannot open %s\n", file);
        exit(1);
    }
    return fd;
}

void Fstat(int fd, struct stat *st, char* name) {
  if(fstat(fd, st) < 0){
    fprintf(2, "find: cannot stat %s\n", name);
    close(fd);
    exit(1);
  }
}

char* get_name(char *dir) {
    int len = strlen(dir);
    int i = len - 1;
    for (; i >= 0; i--) {
        if (dir[i] == '/') {
            break;
        }
    }
    return dir + i + 1;
}

void find(char *dir, char *name) {
    int fd = Open(dir, O_RDONLY);
    struct stat st; Fstat(fd, &st, dir);

    /* file or device */
    if (st.type != T_DIR) {
        char *file = get_name(dir);
        if (strcmp(file, name) == 0)
            printf("%s\n", dir);
        close(fd);
        return;
    }
    /* dir */
    struct dirent de;
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
        int dir_len = strlen(dir);
        int file_len = strlen(de.name);
        if (file_len == 0 || 
            strcmp(de.name, ".") == 0 || 
            strcmp(de.name, "..") == 0)
            continue;
        
        char buf[dir_len + 1 + file_len];
        strcpy(buf, dir);
        buf[dir_len] = '/';
        strcpy(buf + dir_len + 1, de.name);
        find(buf, name);
    }
    close(fd);
    return;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("please input path and specified filename\n");
        exit(1);
    }
    char *dir = argv[1];
    char *name = argv[2];
    find(dir, name);
    exit(0);
}