// Write a program that uses UNIX system calls to "ping-pong"
// a byte between two processes over a pair of pipes,
// one for each direction. The parent should send a byte to the child;
// the child should print ": received ping", where is its process ID,
// write the byte on the pipe to the parent, and exit; the parent
// should read the byte from the child, print ": received pong",
// and exit. Your solution should be in the file user/pingpong.c.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// int pipe(int p[]): Create a pipe, put read/write fd in p[0], p[1]
// int fork(): Create a process, return child's PID
// int getpid(): Return the current process's PID
// int read(int fd, char *buf, int n): 
//      Read n bytes into buf; returns number read, or 0 if EOF

// pipe: 0 read, 1 write

int
main(int argc, char *argv[])
{
  if(argc > 1){
    printf("Err: Too much argument!\n");
    exit(1);
  }

  int fat_p[2], kid_p[2];
  char c, msg = 'x';

  if(pipe(fat_p) < 0) {
    printf("Err: Fail to initialize pipe!\n");
    exit(1);
  }
  if(pipe(kid_p) < 0) {
    printf("Err: Fail to initialize pipe!\n");
    exit(1);
  }

  if(fork() == 0) { // parent -> child
    if(read(fat_p[0], &c, 1) != 1) {
        printf("Err: pipe read failed!\n");
        exit(1);
    }
    printf("%d: received ping\n", getpid());
    if(write(kid_p[1], &msg, 1) != 1) {
        printf("Err: pipe write failed!\n");
        exit(1);
    }
  }
  else { // child -> parent
    if(write(fat_p[1], &msg, 1) != 1) {
        printf("Err: pipe write failed!\n");
        exit(1);
    }
    if(read(kid_p[0], &c, 1) != 1) {
        printf("Err: pipe read failed!\n");
        exit(1);
    }
    printf("%d: received pong\n", getpid());
  }

  exit(0);
}