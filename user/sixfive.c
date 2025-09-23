#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: sixfive file\n");
    exit(1);
  }

  int fd = open(argv[1], 0);
  if(fd < 0){
    printf("sixfive: cannot open %s\n", argv[1]);
    exit(1);
  }

  char buf[512];
  int n;
  while((n = read(fd, buf, sizeof(buf)-1)) > 0){
    buf[n] = '\0';
    char *p = buf;
    while(*p){
      int num = 0;
      while(*p >= '0' && *p <= '9'){
        num = num*10 + (*p - '0');
        p++;
      }
      if(num != 0 && (num % 5 == 0 || num % 6 == 0)){
        printf("%d\n", num);
      }
      if(*p) p++;
    }
  }

  close(fd);
  exit(0);
}
