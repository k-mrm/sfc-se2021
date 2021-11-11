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

#define NTAB 2049

int dohash(int n) {
  return n % NTAB;
}

int rehash(int h) {
  return (h + 1) % NTAB;
}

void htadd(TUPLE **tab, TUPLE *data) {
  int h = dohash(data->key);
  while(tab[h] != NULL) {
    h = rehash(h);
  }
  tab[h] = data;
}

TUPLE *htsearch(TUPLE **tab, int key) {
  int h = dohash(key);
  while(tab[h] != 0) {
    if(tab[h]->key == key)
      return tab[h];
    h = rehash(h);
  }
  return NULL;
}

TUPLE **make_htable() {
  TUPLE **tab = (TUPLE **)calloc(1, sizeof(TUPLE *) * NTAB);
  return tab;
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

  while(true) {
    nr = read(rfd, bufR, NB_BUFR * sizeof(TUPLE));
    if (nr == -1) ERR; else if (nr == 0) break;
  }

  TUPLE **ht = make_htable();
  for(int i = 0; i < 1024; i++) {
    htadd(ht, &bufR[i]);
  }

  gettimeofday(&begin, NULL);
  if ((lseek(sfd, 0, SEEK_SET)) == -1) ERR;
  while (true) {
    ns = read(sfd, bufS, NB_BUFS * sizeof(TUPLE));
    if (ns == -1) ERR; else if (ns == 0) break;

    // join
    for (int i = 0; i < ns/(int)sizeof(TUPLE); i++) {
      TUPLE *t = htsearch(ht, bufS[i].key);
      if(!t)
        continue;
      result.rkey = t->key;
      result.rval = t->val;
      result.skey = bufS[i].key;
      result.sval = bufS[i].val;
      resultVal += result.rval;
    }
  }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  printf("Result: %d\n", resultVal);

  return 0;
}
