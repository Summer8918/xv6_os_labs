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

void filte_prime(int *p) __attribute__((noreturn));

// Convert int to 4 bytes (little-endian)
void int_to_bytes_le(int value, char* bytes) {
    bytes[0] = (value >> 0) & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
}

// Convert 4 bytes back to int (little-endian)
int bytes_to_int_le(const char* bytes) {
    return ((unsigned char)bytes[0]) |
           ((unsigned char)bytes[1] << 8) |
           ((unsigned char)bytes[2] << 16) |
           ((unsigned char)bytes[3] << 24);
}

// Recursively filters multiples of found primes
void filte_prime(int *p) {
  close(p[1]);  // Close write-end in child
  char bytes[4];
  if (read(p[0], bytes, 4) != 4) {
    close(p[0]);
    exit(0);  // nothing to read
  }

  int first = bytes_to_int_le(bytes);
  printf("prime %d\n", first);

  // Limit recursion depth to avoid exhausting file descriptors
  if (first >= 17) {
    while (read(p[0], bytes, 4) == 4) {
      int val = bytes_to_int_le(bytes);
      if (val % first != 0) {
        printf("prime %d\n", val);
      }
    }
    close(p[0]);
    exit(0);
  }

  // Create pipe for next stage
  int p2[2];
  if (pipe(p2) < 0) {
    panic("pipe error");
  }
  int pid = fork();
  if (pid == 0) {
    // Child: filter with next prime
    filte_prime(p2);
  } else if (pid > 0) {
    // Parent: send remaining numbers
    close(p2[0]);
    while (read(p[0], bytes, 4) == 4) {
      int val = bytes_to_int_le(bytes);
      if (val % first != 0) {
        write(p2[1], bytes, 4);
      }
    }
    close(p[0]);
    close(p2[1]);
    wait(0);
  } else {
    panic("fork error");
  }
  exit(0);
}

int
main(int argc, char *argv[])
{
  int p[2];
  if (pipe(p) < 0) {
    panic("pipe error");
  }
  int pid = fork();
  if (pid > 0) {
    close(p[0]);
    char bytes[4];
    for (int i = 2; i <= 280; ++i) {
      int_to_bytes_le(i, bytes);
      write(p[1], bytes, 4);
    }
    close(p[1]);
    wait(0);
    exit(0);
  } else if (pid == 0) {
    filte_prime(p);
  } else {
    panic("fork error");
  }
}
