#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
int ticks = uptime();
printf("uptime: %d ticks",ticks);
exit(0);
}
