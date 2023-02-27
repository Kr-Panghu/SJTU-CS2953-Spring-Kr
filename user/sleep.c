// Implement the UNIX program sleep for xv6;
// your sleep should pause for a user-specified number
// of ticks. A tick is a notion of time defined by the xv6 kernel
// namely the time between two interrupts

// int sleep(int n): Pause for n clock ticks

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *str;

  if(argc <= 1){
    printf("Err: Fail to fetch argument!\n");
    exit(1);
  }
  if(argc >= 3){
    printf("Err: Too much argument!\n");
    exit(1);
  }
  str = argv[1];
  i = atoi(str);
  sleep(i);
  exit(0);
}
