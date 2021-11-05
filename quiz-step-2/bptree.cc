#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <cstring>
#include <random>
#include <algorithm>
#include <time.h>

struct timeval
cur_time(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return t;
}

void print_node(NODE *n) {
  if(!n)
    return;

  for(int i = 0; i < n->nkey; i++) {
    printf("%d ", n->key[i]);
  }
  printf("\n");
}

void
print_tree_core(NODE *n)
{
  printf("["); 
  for (int i = 0; i < n->nkey; i++) {
    if (!n->isLeaf) print_tree_core(n->chi[i]); 
    printf("%d", n->key[i]); 
    if (i != n->nkey-1 && n->isLeaf) putchar(' ');
  }
  if (!n->isLeaf) print_tree_core(n->chi[n->nkey]);
  printf("]");
}

void
print_tree(NODE *node)
{
  print_tree_core(node);
  printf("\n"); fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
  int kid;

  if (node->isLeaf) return node;
  for (kid = 0; kid < node->nkey; kid++) {
    if (key < node->key[kid]) break;
  }

  return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
  int i;
  for (i = 0; i < leaf->nkey; i++) {
    if (key < leaf->key[i]) break;
  }
  for (int j = leaf->nkey; j > i; j--) {    
    leaf->chi[j] = leaf->chi[j-1] ;
    leaf->key[j] = leaf->key[j-1] ;
  } 

  leaf->key[i] = key;
  leaf->chi[i] = (NODE *)data;

  leaf->nkey++;

  return leaf;
}

NODE *
alloc_leaf(NODE *parent)
{
  NODE *node;
  if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
  node->isLeaf = true;
  node->parent = parent;
  node->nkey = 0;

  return node;
}

NODE *alloc_node(NODE *parent)
{
  NODE *node;
  if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
  node->isLeaf = false;
  node->parent = parent;
  node->nkey = 0;

  return node;
}

void cp_node2tmp(NODE *n, TEMP *tmp) {
  for(int i = 0; i < N; i++)
    tmp->chi[i] = n->chi[i];
  for(int i = 0; i < N-1; i++)
    tmp->key[i] = n->key[i];

  tmp->nkey = n->nkey;
}

void clean_node(NODE *n) {
  for(int i = 0; i < N-1; i++) {
    n->chi[i] = 0;
    n->key[i] = 0;
  }
  n->nkey = 0;
}

TEMP *insert_in_temp(TEMP *temp, int key, DATA *data) {
  int i;
  for (i = 0; i < temp->nkey; i++) {
    if (key < temp->key[i]) break;
  }
  for (int j = temp->nkey; j > i; j--) {    
    temp->chi[j] = temp->chi[j-1];
    temp->key[j] = temp->key[j-1];
  } 

  temp->key[i] = key;
  temp->chi[i] = (NODE *)data;
  temp->nkey++;

  return temp;
}

void insert_in_node(NODE *node, int key, NODE *c) {
  int i;
  for(i = 0; i < node->nkey; i++) {
    if(key < node->key[i])
      break;
  }

  for(int j = node->nkey; j > i; j--)
    node->key[j] = node->key[j-1];
  for(int j = node->nkey+1; j > i; j--)
    node->chi[j] = node->chi[j-1];

  node->key[i] = key;
  node->chi[i+1] = c;
  node->nkey++;
}

void insert_in_node_temp(TEMP *tmp, int key, NODE *c) {
  int i;
  for(i = 0; i < tmp->nkey; i++) {
    if(key < tmp->key[i])
      break;
  }

  for(int j = tmp->nkey; j > i; j--)
    tmp->key[j] = tmp->key[j-1];
  for(int j = tmp->nkey+1; j > i; j--)
    tmp->chi[j] = tmp->chi[j-1];

  tmp->key[i] = key;
  tmp->chi[i+1] = c;
  tmp->nkey++;
}

void cp_tmp2node1(TEMP *tmp, NODE *node) {
  int n = (int)ceil((double)(N+1)/2);
  int i;
  for(i = 0; i < n; i++)
    node->chi[i] = tmp->chi[i];
  for(i = 0; i < n-1; i++)
    node->key[i] = tmp->key[i];

  node->nkey = i;
}

void cp_tmp2node2(TEMP *tmp, NODE *node) {
  int n = (int)ceil((double)(N+1)/2);
  int i, j;
  for(i = n, j = 0; i < N+1; i++, j++)
    node->chi[j] = tmp->chi[i];
  for(i = n, j = 0; i < N; i++, j++)
    node->key[j] = tmp->key[i];

  node->nkey = j;
}

void insert_in_parent(NODE *n, int key, NODE *n2) {
  if(n == Root) {
    NODE *newroot = alloc_node(NULL);
    newroot->chi[0] = n;
    newroot->key[0] = key;
    newroot->chi[1] = n2;
    newroot->nkey = 1;

    n2->parent = n->parent = newroot;

    Root = newroot;

    return;
  }

  NODE *p = n->parent;

  if(p->nkey < N-1) {
    insert_in_node(p, key, n2);
    n2->parent = p;
  } else {
    TEMP tmp;

    cp_node2tmp(p, &tmp);

    insert_in_node_temp(&tmp, key, n2);

    clean_node(p);

    NODE *p2 = alloc_node(NULL);

    cp_tmp2node1(&tmp, p);
    cp_tmp2node2(&tmp, p2);

    for(int i = 0; i < p->nkey+1; i++)
      p->chi[i]->parent = p;
    for(int i = 0; i < p2->nkey+1; i++)
      p2->chi[i]->parent = p2;

    int k = tmp.key[(int)ceil((double)(N+1)/2)-1];
    
    insert_in_parent(p, k, p2);
  }
}

void cp_tmp2leaf1(TEMP *tmp, NODE *leaf) {
  int i;
  int n = (int)ceil((double)N/2);
  for(i = 0; i < n; i++) {
    leaf->chi[i] = tmp->chi[i];
    leaf->key[i] = tmp->key[i];
  }

  leaf->nkey = i;
}

void cp_tmp2leaf2(TEMP *tmp, NODE *leaf) {
  int i, j;
  int n = (int)ceil((double)N/2);
  for(i = n, j = 0; i < N; i++, j++) {
    leaf->chi[j] = tmp->chi[i];
    leaf->key[j] = tmp->key[i];
  }

  leaf->nkey = j;
}

void leaf_split(NODE *leaf, int key, DATA *data) {
  TEMP tmp;

  cp_node2tmp(leaf, &tmp);
  insert_in_temp(&tmp, key, data);

  NODE *l2 = alloc_leaf(leaf->parent);
  l2->chi[N-1] = leaf->chi[N-1];
  leaf->chi[N-1] = l2;

  clean_node(leaf);

  cp_tmp2leaf1(&tmp, leaf);
  cp_tmp2leaf2(&tmp, l2);

  int k = l2->key[0];
  
  insert_in_parent(leaf, k, l2);
}

void 
insert(int key, DATA *data)
{
  NODE *leaf;

  if (Root == NULL) {
    leaf = alloc_leaf(NULL);
    Root = leaf;
  }
  else {
    leaf = find_leaf(Root, key);
  }

  if (leaf->nkey < (N-1)) {
    insert_in_leaf(leaf, key, data);
  }
  else { // split
    leaf_split(leaf, key, data);
  }
}

int search(int key) {
  NODE *leaf = find_leaf(Root, key);

  for(int i = 0; i < leaf->nkey; i++) {
    if(leaf->key[i] == key)
      return 1;
  }

  return 0;
}

void
init_root(void)
{
  Root = NULL;
}

int 
interactive()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

#define NTAB  20000003

int dohash(int n) {
  return n % NTAB;
}

int rehash(int h) {
  return (h + 1) % NTAB;
}

void htadd(int *tab, int key) {
  int h = dohash(key);
  while(tab[h] != 0) {
    h = rehash(h);
  }
  tab[h] = key;
}

int htsearch(int *tab, int key) {
  int h = dohash(key);
  while(tab[h] != 0) {
    if(tab[h] == key)
      return 1;
    h = rehash(h);
  }
  return 0;
}

int *make_htable() {
  int *tab = (int *)calloc(1, sizeof(int) * NTAB);
  return tab;
}

#define NELEM 10000000

int
main(int argc, char *argv[])
{
  if(argc != 2) {
    puts("argv error");
    return 1;
  }

  struct timeval begin, end;
  std::random_device rnd;

  init_root();

  printf("-----Insert-----\n");
  /*
  while (true) {
    insert(interactive(), NULL);
    print_tree(Root);
  }
  */

  switch(argv[1][0]-'0') {
    case 1:
      begin = cur_time();
      for(int i = 1; i <= NELEM; i++)
        insert(i, NULL);
      for(int i = 1; i <= NELEM; i++) {
        int res = search(i);
        if(res == 0)
          printf("%d not found\n", i);
      }
      puts("done!");
      break;
    case 2:
      begin = cur_time();
      for(int i = NELEM; i >= 1; i--)
        insert(i, NULL);
      for(int i = 1; i <= NELEM; i++) {
        int res = search(i);
        if(res == 0)
          printf("%d not found\n", i);
      }
      puts("done!");
      break;
    case 3: {
      int *ht = make_htable();
      srand((unsigned int)time(NULL));
      for(int i = 0; i < NELEM; i++) {
        int r;
        do {
          r = rand() % 100000007;
        } while(htsearch(ht, r));
        htadd(ht, r);
      }

      for(int i = 0; i < NTAB; i++) {
        if(ht[i] == 0)
          continue;
        insert(i, NULL);
      }

      for(int i = 0; i < NTAB; i++) {
        if(ht[i] == 0)
          continue;
        int res = search(i);
        if(res == 0)
          printf("%d not found\n", i);
      }
      puts("done!");
      break;
    }
    default:
      break;
  }
  end = cur_time();

  return 0;
}
