#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"


void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

int
main(int argc, char *argv[])
{
  int p1[2];  // parent process write, child process read
  int p2[2];  // child process write, parent process read
  if(pipe(p1) < 0) {
    panic("pipe");
  }
  if(pipe(p2) < 0) {
    panic("pipe");
  }
  int pid = fork();
  if(pid == 0) { // child process
    close(p1[1]);
    char buf[10];
    read(p1[0], buf, 1);
    close(p1[0]);
    int cpid = getpid();

    printf("%d: received ping\n", cpid);
    close(p2[0]);
    write(p2[1], buf, 1);
    close(p2[1]);
    exit(0);
  } else if(pid < 0){
    printf("grind: fork failed\n");
    exit(1);
  } else {  // parent process
    close(p1[0]);
    char buf[10] = "A";
    write(p1[1], buf, 1);
    close(p1[1]);

    int ppid = getpid();
    close(p2[1]);
    read(p2[0], buf, 1);
    printf("%d: received pong\n", ppid);
    close(p2[0]);
  }

  exit(0);
}
