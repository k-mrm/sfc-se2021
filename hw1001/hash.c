#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void put_array(int *arr, int n) {
  for(int i = 0; i < n; i++) {
    printf("%d ", arr[i]);
  }
}

void binsearch(int *arr, int n, int key) {
  printf("binsearch: ");
  int left = 0;
  int right = n;
  int mid;
  int step = 0;

  while(left <= right) {
    step++;
    mid = (left + right) / 2;
    if(arr[mid] == key)
      goto found;
    else if(arr[mid] < key)
      left = mid + 1;
    else
      right = mid - 1;
  }

  assert(0);

found:
  printf("%d, step %d\n", arr[mid], step);
}

int hash(int n, int ntab) {
  return n % ntab;
}

int rehash(int h, int ntab) {
  return (h + 1) % ntab;
}

int *make_htable(int *arr, int narr, int ntab) {
  int *tab = malloc(sizeof(int) * ntab);
  for(int i = 0; i < narr; i++) {
    int h = hash(arr[i], ntab);
    while(tab[h] != 0)
      h = rehash(h, ntab);
    tab[h] = arr[i];
  }
  return tab;
}

void hsearch(int *arr, int n, int key) {
  printf("hsearch: ");
  int ntab = 2039;
  int *tab = make_htable(arr, n, ntab);
  int h = hash(key, ntab);
  int step = 1;
  while(tab[h] != key) {
    h = rehash(h, ntab);
    step++;
  }

  printf("%d, step %d\n", tab[h], step);
}

int main(void) {
  int n = 1000;
  int *arr = malloc(sizeof(int) * n);
  for(int i = 1; i <= n; i++)
    arr[i] = i;
  put_array(arr, n);

  int key;
  printf("Key? ");
  scanf("%d", &key);
  assert(key > 0);

  binsearch(arr, n, key);
  hsearch(arr, n, key);
}
