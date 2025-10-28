//for assignemnt2 
#include "kernel/types.h"
#include "user/user.h"

int main() {
  int pages = 10;
  char *p = sbrk(pages * 4096);
  // touch pages one at a time:
  for(int i = 0; i < pages; i++){
    p[i * 4096] = 'A' + (i % 26);
    printf("touched page %d char=%c\n", i, p[i*4096]);
  }
  exit(0);
}
