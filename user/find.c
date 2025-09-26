#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// =====================
// Regex helper functions (copied from grep.c)
// =====================
int match(char *re, char *text);
int matchhere(char*, char*);
int matchstar(int, char*, char*);

int match(char *re, char *text) {
  if(re[0] == '^')
    return matchhere(re+1, text);
  do {
    if(matchhere(re, text))
      return 1;
  } while(*text++ != '\0');
  return 0;
}

int matchhere(char *re, char *text) {
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

int matchstar(int c, char *re, char *text) {
  do {
    if(matchhere(re, text))
      return 1;
  } while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

// =====================
// Recursive find function
// =====================
void find(char *path, char *pattern) {
    char buf[512];
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Try to open the current path
    if ((fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        return;
    }

    // Get info about file/directory
    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type) {
    case T_FILE:
        // Extract filename from path
        p = path;
        for (int i = strlen(path) - 1; i >= 0; i--) {
            if (path[i] == '/') {
                p = path + i + 1;
                break;
            }
        }
        // Use regex match instead of strcmp
        if (match(pattern, p))
            printf("%s\n", path);
        break;

    case T_DIR:
        // For directories: prepare buffer
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            strcpy(p, de.name);

            struct stat st2;
            int fd2;
            if ((fd2 = open(buf, 0)) < 0)
                continue;
            if (fstat(fd2, &st2) < 0) {
                close(fd2);
                continue;
            }
            close(fd2);

            if (st2.type == T_DIR) {
                find(buf, pattern);
            } else if (st2.type == T_FILE) {
                if (match(pattern, de.name))
                    printf("%s\n", buf);
            }
        }
        break;
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: find <path> <pattern>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
