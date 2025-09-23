#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"   // for MAXARG

// Recursive function to search files
void find(char *path, char *target, int exec_mode, char **cmdargv, int cmdargc) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Try opening path
    if ((fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:
        // Extract filename from path
        p = path + strlen(path);
        while (p >= path && *p != '/')
            p--;
        p++;

        if (strcmp(p, target) == 0) {
            if (exec_mode == 0) {
                // Normal mode: just print
                printf("%s\n", path);
            } else {
                // Exec mode: run command with file appended
                if (fork() == 0) {
                    char *argv[MAXARG];
                    // Copy command args into argv
                    for (int i = 0; i < cmdargc; i++) {
                        argv[i] = cmdargv[i];
                    }
                    argv[cmdargc] = path;  // append file path
                    argv[cmdargc + 1] = 0;

                    exec(cmdargv[0], argv);
                    printf("exec %s failed\n", cmdargv[0]);
                    exit(1);
                } else {
                    wait(0);
                }
            }
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            find(buf, target, exec_mode, cmdargv, cmdargc);
        }
        break;
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: exec <path> <filename> [-exec <command> [args...]]\n");
        exit(1);
    }

    int exec_mode = 0;
    char **cmdargv = 0;
    int cmdargc = 0;

    // Check for -exec
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-exec") == 0) {
            exec_mode = 1;
            cmdargv = &argv[i + 1];   // command starts after -exec
            cmdargc = argc - (i + 1); // number of command args
            break;
        }
    }

    find(argv[1], argv[2], exec_mode, cmdargv, cmdargc);
    exit(0);
}
