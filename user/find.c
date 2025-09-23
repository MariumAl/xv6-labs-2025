#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Recursive function to find all files named 'target' starting from 'path'
void find(char *path, char *target) {
    char buf[512];          // Buffer to store full path names
    char *p;                // Pointer to traverse path strings
    int fd;                 // File descriptor for current path
    struct dirent de;       // Directory entry structure
    struct stat st;         // Stat structure to get file info

    // Try to open the current path
    if ((fd = open(path, 0)) < 0) {
        printf("find: cannot open %s\n", path);
        return;
    }
    // Get information about the file/directory
    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // Check the type of the path (file or directory)
    switch(st.type) {

        case T_FILE:
            // For files: extract the filename from the path
            p = path;
            for (int i = strlen(path) - 1; i >= 0; i--) {
                if (path[i] == '/') {
                    p = path + i + 1; // Point to start of filename
                    break;
                }
            }
            // If filename matches target, print the full path
            if (strcmp(p, target) == 0)
                printf("%s\n", path);
            break;
        case T_DIR:
            // For directories: prepare a buffer for sub-paths
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/'; // Add '/' to separate directory and file names

            // Read all entries in the directory
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue; // Skip unused entries

                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue; // Skip "." and ".." to avoid infinite recursion

                // Construct full path for the directory entry
                strcpy(p, de.name);

                // Get information about the entry
                struct stat st2;
                int fd2;
                if ((fd2 = open(buf, 0)) < 0)
                    continue;
                if (fstat(fd2, &st2) < 0) {
                    close(fd2);
                    continue;
                }
                close(fd2);
                // Recurse if it’s a directory
                if (st2.type == T_DIR)
                    find(buf, target);
                // Print path if it’s a file and matches the target
                else if (st2.type == T_FILE && strcmp(de.name, target) == 0)
                    printf("%s\n", buf);
            }
            break;
    }

    close(fd); // Close the file descriptor for this path
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: find <path> <filename>\n");
        exit(1);
    }
    // Start the recursive search from the provided path
    find(argv[1], argv[2]);

    exit(0);
}
