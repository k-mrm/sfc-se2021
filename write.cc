#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "debug.h"

typedef struct _TUPLE {
  int key;
  int val;
} TUPLE;

void
writeToStorage(const int max)
{
  int fd;
  TUPLE t;
  int key = 0;

  fd = open("R", O_WRONLY|O_TRUNC|O_CREAT, 0644);
  if (fd == -1) ERR;
  for (int i = 0; i < max; i++) {
    t.key = key++;
    t.val = rand() % 100;
    write(fd, &t, sizeof(t));
  }
  close(fd);

  key = 0;
  fd = open("S", O_WRONLY|O_TRUNC|O_CREAT, 0644);
  if (fd == -1) ERR;
  for (int i = 0; i < max; i++) {
    t.key = key++;
    t.val = rand() % 100;
    write(fd, &t, sizeof(t));
  }
  close(fd);
}

void readtostorage() {
  int fd;
  TUPLE t;
  fd = open("R", O_RDONLY, 0644);
  if(fd < 0)
    ERR;

  while(read(fd, &t, sizeof(t)) == sizeof(t)) {
    printf("%d %d\n", t.key, t.val);
  }
  close(fd);

  fd = open("S", O_RDONLY, 0644);
  if(fd < 0)
    ERR;

  while(read(fd, &t, sizeof(t)) == sizeof(t)) {
    printf("%d %d\n", t.key, t.val);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int max;

  if (argc != 2) max = 10;
  else max = atoi(argv[1]);

  writeToStorage(max);
  readtostorage();

  return 0;
}
