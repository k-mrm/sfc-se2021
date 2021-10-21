#include "bptree.h"
#include <vector>
#include <sys/time.h>
#include <cstring>

struct timeval
cur_time(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return t;
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
  if (key < leaf->key[0]) {
    for (i = leaf->nkey; i > 0; i--) {
      leaf->chi[i] = leaf->chi[i-1] ;
      leaf->key[i] = leaf->key[i-1] ;
    } 
    leaf->key[0] = key;
    leaf->chi[0] = (NODE *)data;
  }
  else {
    for (i = 0; i < leaf->nkey; i++) {
      if (key < leaf->key[i]) break;
    }
    for (int j = leaf->nkey; j > i; j--) {    
      leaf->chi[j] = leaf->chi[j-1] ;
      leaf->key[j] = leaf->key[j-1] ;
    } 

    /* CodeQuiz */
    leaf->key[i] = key;
    leaf->chi[i] = (NODE *)data;
  }
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

TEMP *alloc_temp() {
  TEMP *t;
  if(!(t = (TEMP *)calloc(1, sizeof(*t)))) ERR;

  t->isLeaf = true;
  t->nkey = 0;

  return t;
}

void cp_leaf2tmp(NODE *leaf, TEMP *tmp) {
  for(int i = 0; i < N; i++)
    tmp->chi[i] = leaf->chi[i];
  for(int i = 0; i < N-1; i++)
    tmp->key[i] = leaf->key[i];

  tmp->nkey = leaf->nkey;
}

TEMP *insert_in_temp(TEMP *temp, int key, DATA *data) {
  int i;
  if (key < temp->key[0]) {
    for (i = temp->nkey; i > 0; i--) {
      temp->chi[i] = temp->chi[i-1];
      temp->key[i] = temp->key[i-1];
    } 
    temp->key[0] = key;
    temp->chi[0] = (NODE *)data;
  }
  else {
    for (i = 0; i < temp->nkey; i++) {
      if (key < temp->key[i]) break;
    }
    for (int j = temp->nkey; j > i; j--) {    
      temp->chi[j] = temp->chi[j-1];
      temp->key[j] = temp->key[j-1];
    } 

    /* CodeQuiz */
    temp->key[i] = key;
    temp->chi[i] = (NODE *)data;
  }
  temp->nkey++;

  return temp;
}

void insert_in_parent(NODE *n, int key, NODE *n2) {
  if(n == Root) {
    NODE *newroot = alloc_node(NULL);
    newroot->chi[0] = n;
    newroot->key[0] = key;
    newroot->chi[1] = n2;
    newroot->nkey = 1;

    Root = newroot;

    return;
  }

  return;
}

void clean_leaf(NODE *leaf) {
  for(int i = 0; i < N-1; i++) {
    leaf->chi[i] = 0;
    leaf->key[i] = 0;
  }
}

void cp_tmp2leaf1(NODE *leaf, TEMP *tmp) {
  int i;
  int n = (int)ceil(N/2);
  for(i = 0; i < n; i++) {
    leaf->chi[i] = tmp->chi[i];
    leaf->key[i] = tmp->key[i];
  }

  leaf->nkey = i;
}

void cp_tmp2leaf2(NODE *leaf, TEMP *tmp) {
  int i, j;
  int n = (int)ceil(N/2);
  for(i = n, j = 0; i < N; i++, j++) {
    leaf->chi[j] = tmp->chi[i];
    leaf->key[j] = tmp->key[i];
  }

  leaf->nkey = j;
}

void leaf_split(NODE *leaf, int key, DATA *data) {
  printf("split %d\n", key);
  TEMP *tmp = alloc_temp();

  cp_leaf2tmp(leaf, tmp);
  insert_in_temp(tmp, key, data);

  NODE *l2 = alloc_leaf(leaf->parent);
  l2->chi[N-1] = leaf->chi[N-1];
  leaf->chi[N-1] = l2;

  clean_leaf(leaf);

  cp_tmp2leaf1(leaf, tmp);
  cp_tmp2leaf2(l2, tmp);

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
    // future work
    leaf_split(leaf, key, data);
  }
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

int
main(int argc, char *argv[])
{
  struct timeval begin, end;

  init_root();

  printf("-----Insert-----\n");
  begin = cur_time();
  while (true) {
    insert(interactive(), NULL);
    print_tree(Root);
  }
  end = cur_time();

  return 0;
}
