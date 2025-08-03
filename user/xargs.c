// | pipe operator takes the stdout of left pip and connects it to the stdin of right
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
  char buf[512];

  int cnt = 0;
  char *new_argv[MAXARG];
  memset(new_argv, 0, MAXARG);
  int j = 0;
  // Copy fixed arguments (argv[1..])
  for (int i = 1; i < argc; ++i, ++j) {
    new_argv[j] = argv[i];
  }


  while (read(0, buf + cnt, 1) == 1 && cnt < 512) {
    if (buf[cnt] == '\n') {
      buf[cnt] = 0;
      new_argv[j] = buf;
      if (fork() == 0) {
        exec(argv[1], new_argv);
      }
      wait(0);
      cnt = 0;
    } else {
      cnt++;
      if (cnt >= sizeof(buf) - 1) {
        fprintf(2, "input too long\n");
        exit(1);
      }
    }
  }

  // Handle the last line if not newline-terminated
  if (cnt > 0) {
    buf[cnt] = 0;
    new_argv[j] = buf;
    new_argv[j + 1] = 0;

    if (fork() == 0) {
      exec(new_argv[0], new_argv);
      fprintf(2, "exec failed\n");
      exit(1);
    }
    wait(0);
  }
  exit(0);
}