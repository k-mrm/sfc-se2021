#include <stdio.h>
#include <stdlib.h>

struct list {
  struct list *next;
  int data;
};

struct list *new_node(int data) {
  struct list *l = malloc(sizeof(*l));
  l->next = NULL;
  l->data = data;

  return l;
}

struct list *list_add(struct list *l, int data) {
  struct list *n = new_node(data);
  l->next = n;

  return n;
}

void put_list(struct list *l) {
  for(; l; l = l->next) {
    printf("%d ", l->data);
  }
}

int main(void) {
  struct list head;
  struct list *hp = &head;
  for(int i = 1; i <= 100; i++) {
    hp = list_add(hp, i);
  }
  put_list(head.next);
}
