// user/find.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char buf[512];
char *argv[32];   // for exec

void
find(char *path, char *pattern, int exec_mode, int argc, char **cmd_argv)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    // check if last component matches
    {
      char *fname = path;
      for (char *s = path; *s; s++)
        if (*s == '/')
          fname = s + 1;

      if(strcmp(fname, pattern) == 0){
        if(exec_mode){
          // build argv for exec
          char *eargv[32];
          for(int i = 0; i < argc; i++)
            eargv[i] = cmd_argv[i];
          eargv[argc] = path;   // append file path
          eargv[argc+1] = 0;

          if(fork() == 0){
            exec(eargv[0], eargv);
            fprintf(2, "find: exec %s failed\n", eargv[0]);
            exit(1);
          }
          wait(0);
        } else {
          printf("%s\n", path);
        }
      }
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, pattern, exec_mode, argc, cmd_argv);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: find <path> <pattern> [-exec <command> ...]\n");
    exit(1);
  }

  int exec_mode = 0;
  int cmd_argc = 0;
  char **cmd_argv = 0;

  // detect -exec
  for(int i = 3; i < argc; i++){
    if(strcmp(argv[i-1], "-exec") == 0){
      exec_mode = 1;
      cmd_argc = argc - i;
      cmd_argv = &argv[i];
      break;
    }
  }

  find(argv[1], argv[2], exec_mode, cmd_argc, cmd_argv);
  exit(0);
}
