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

