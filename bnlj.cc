#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "debug.h"

#define SZ_PAGE 4096
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))

typedef struct _TUPLE {
  int key;
  int val;
} TUPLE;

typedef struct _RESULT {
  int rkey;
  int rval;
  int skey;
  int sval;
} RESULT;

void
printDiff(struct timeval begin, struct timeval end)
{
  long diff;

  diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  printf("Diff: %ld us (%ld ms)\n", diff, diff/1000);
}

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
}

void readfromstorage() {
  int fd;
  TUPLE t;
  fd = open("R", O_RDONLY, 0644);
  if(fd < 0)
    ERR;

  while(read(fd, &t, sizeof(t)) == sizeof(t)) {
    printf("%d %d\n", t.key, t.val);
  }

  close(fd);
}


int 
main(void)
{
  int rfd;
  int sfd;
  int nr;
  int ns;
  TUPLE bufR[NB_BUFR];
  TUPLE bufS[NB_BUFS];
  RESULT result;
  int resultVal = 0;
  struct timeval begin, end;

  rfd = open("R", O_RDONLY); if (rfd == -1) ERR;
  sfd = open("S", O_RDONLY); if (sfd == -1) ERR;

  gettimeofday(&begin, NULL);
  while (true) {
    nr = read(rfd, bufR, NB_BUFR * sizeof(TUPLE));
    if (nr == -1) ERR; else if (nr == 0) break;

    if ((lseek(sfd, 0, SEEK_SET)) == -1) ERR;
    while (true) {
      ns = read(sfd, bufS, NB_BUFS * sizeof(TUPLE));
      if (ns == -1) ERR; else if (ns == 0) break;

      // join
      for (int i = 0; i < nr/(int)sizeof(TUPLE); i++) {
        for (int j = 0; j < ns/(int)sizeof(TUPLE); j++) {
          if (bufR[i].val == bufS[j].val) {
            result.rkey = bufR[i].key;
            result.rval = bufR[i].val;
            result.skey = bufS[j].key;
            result.sval = bufS[j].val;
            resultVal += result.rval;
          }
        }
      }
    }
  }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  printf("Result: %d\n", resultVal);

  return 0;
}
