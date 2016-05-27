#include "list.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

typedef struct node node;

struct node {
  char *name;
  void *data;
  node *next;
};


struct list {
  node *first;
  node *last;
  int size;
};




static node *new_node(const char *name, void *data, node *next)
{
  node *n = malloc(sizeof(node));
  n->name = malloc(strlen(name)+1);
  strcpy(n->name, name);
  n->data = data;
  n->next = next;
  return n;
}



static node *new_node_noalloc(char *name, void *data, node *next)
{
  node *n = malloc(sizeof(node));
  n->name = name;
  n->data = data;
  n->next = next;
  return n;
}



static int empty(list l)
{
  /* return l->size == 0; */
  return l->first == NULL;
}



list new_list() 
{
  struct list *l = malloc(sizeof(struct list));
  l->first = NULL;
  l->last = NULL;
  l->size = 0;
  return l;
}



int mem(list l, const char *name)
{
 void *nothing;
 return find(&nothing, l, name);
}



int memn(list l, const char *name, unsigned int len)
{
 void *nothing;
 return findn(&nothing, l, name, len);
}



int find(void **data, list l, const char *name)
{
  if (empty(l))
    return 0;
  for (node *n = l->first; n != NULL; n = n->next)
    if (strcmp(n->name, name) == 0) {
      *data = n->data;
      return 1;
    }
  return 0;
}



int findn(void **data, list l, const char *name, unsigned int len)
{
  if (empty(l))
    return 0;
  for (node *n = l->first; n != NULL; n = n->next)
    if (strncmp(n->name, name, len) == 0) {
      *data = n->data;
      return 1;
    }
  return 0;
}



int insert_one(list l, const char *name, void *data)
{
  if (empty(l)) {
    l->first = new_node(name, data, NULL);
    l->last = l->first;
  }
  else if (mem(l, name))
    return 0;
  else {
    l->last->next = new_node(name, data, NULL);
    l->last = l->last->next;
  }
  ++l->size;
  return 1;
}



int insert_one_noalloc(list l, char *name, void *data)
{
  if (empty(l)) {
    l->first = new_node_noalloc(name, data, NULL);
    l->last = l->first;
  }
  else if (mem(l, name))
    return 0;
  else {
    l->last->next = new_node_noalloc(name, data, NULL);
    l->last = l->last->next;
  }
  ++l->size;
  return 1;
}



void insert(list l, const char *name, void *data)
{
  if (empty(l)) {
    l->first = new_node(name, data, NULL);
    l->last = l->first;
  }
  else {
    l->last->next = new_node(name, data, NULL);
    l->last = l->last->next;
  }
  ++l->size;
}



void insert_noalloc(list l, char *name, void *data)
{
  if (empty(l)) {
    l->first = new_node_noalloc(name, data, NULL);
    l->last = l->first;
  }
  else {
    l->last->next = new_node_noalloc(name, data, NULL);
    l->last = l->last->next;
  }
  ++l->size;
}



static void free_node(node *n)
{
  free(n->name);
  free(n);
}



/* static void free_node_data(node *n) */
/* { */
/*   free(n->name); */
/*   free(n->data); */
/*   free(n); */
/* } */

static void free_node_data(node *n, void (*free_func)(void *))
{
  free(n->name);
  free_func(n->data);
  free(n);
}


int rm_free_data(list l, const char *name, void (*free_func) (void *))
{
  if (empty(l))
    return 0;
  node *n = l->first;
  if (strcmp(n->name, name) == 0) {
    /* if (l->first == l->last) */
    /*   l->first = NULL; */
    l->first = n->next;
    free_node_data(n, free_func);
    --l->size;
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strcmp(n->next->name, name) == 0) {
        node *rem = n->next;
        n->next = n->next->next;
        if (rem == l->last)
          l->last = n;
        free_node_data(rem, free_func);
        --l->size;
        return 1;
      }
    }
    return 0;
  }
}

int rmn_free_data(list l, const char *name, unsigned int len, void (*free_func)(void*))
{
  if (empty(l))
    return 0;
  node *n = l->first;
  if (strncmp(n->name, name, len) == 0) {
    /* if (l->first == l->last) */
    /*   l->first = NULL; */
    l->first = n->next;
    free_node_data(n, free_func);
    --l->size;
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strncmp(n->next->name, name, len) == 0) {
        node *rem = n->next;
        n->next = n->next->next;
        if (rem == l->last)
          l->last = n;
        free_node_data(rem, free_func);
        --l->size;
        return 1;
      }
    }
    return 0;
  }
}



int rm(list l, const char *name)
{
  if (empty(l))
    return 0;
  node *n = l->first;
  if (strcmp(n->name, name) == 0) {
    /* if (l->first == l->last) */
    /*   l->last = NULL; */
    l->first = n->next;
    free_node(n);
    --l->size;
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strcmp(n->next->name, name) == 0) {
        node *rem = n->next;
        if (rem == l->last)
          l->last = n;
        n->next = n->next->next;
        free_node(rem);
        --l->size;
        return 1;
      }
    }
    return 0;
  }
}



int rmn(list l, const char *name, unsigned int len)
{
  if (empty(l))
    return 0;
  node *n = l->first;
  if (strncmp(n->name, name, len) == 0) {
    free_node(n);
    --l->size;
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strncmp(n->next->name, name, len) == 0) {
        node *rem = n->next;
        if (rem == l->last)
          l->last = n;
        n->next = n->next->next;
        free_node(rem);
        --l->size;
        return 1;
      }
    }
    return 0;
  }
}



void iter(list l, void (*func)(char *, void *))
{
  if (empty(l))
    return;
  for (node *n = l->first; n != NULL; n = n->next)
    func(n->name, n->data);
}



iterator get_iterator(list l)
{
  if (empty(l))
    return NULL;
  else
    return l->first;
}

char *iterator_getname(iterator i)
{
  return i->name;
}

void *iterator_getdata(iterator i)
{
  return i->data;
}

void iterate(iterator *i)
{
  *i = (*i)->next;
}



void rm_all_if(list l, int (*predicate) (const char *, void *))
{
  if (empty(l))
    return;

  while (l->first && predicate(l->first->name, l->first->data)) {
    node *rm = l->first;
    l->first = l->first->next;
    free_node(rm);
    --l->size;
  }
  if (! l->first)
    l->last = NULL;
  else {
    node *n = l->first;
    while (n->next != l->last) {
      if (predicate(n->next->name, n->next->data)) {
        node *rm = n->next;
        n->next = n->next->next;
        free_node(rm);
        --l->size;
      }
      else {
        n = n->next;
      }
    }
    if (predicate(n->next->name, n->next->data)) {
        node *rm = n->next;
        n->next = n->next->next;
        free_node(rm);
        --l->size;
        l->last = n;
    }
  }
}



void rm_all_if_free_data(list l, int (*predicate) (const char *, void *), void (*free_func) (void *))
{
  if (empty(l))
    return;

  while (l->first && predicate(l->first->name, l->first->data)) {
    node *rm = l->first;
    l->first = l->first->next;
    free_node_data(rm, free_func);
    --l->size;
  }
  if (! l->first)
    l->last = NULL;
  else {
    node *n = l->first;
    while (n->next != l->last) {
      if (predicate(n->next->name, n->next->data)) {
        node *rm = n->next;
        n->next = n->next->next;
        free_node_data(rm, free_func);
        --l->size;
      }
      else {
        n = n->next;
      }
    }
    if (predicate(n->next->name, n->next->data)) {
        node *rm = n->next;
        n->next = n->next->next;
        free_node_data(rm, free_func);
        --l->size;
        l->last = n;
    }
  }

  /* node *n = l->first; */
  /* while (n->next) { */
  /*   if (predicate(n->next->name, n->next->data)) { */
  /*     node *rm = n->next; */
  /*     n->next = n->next->next; */
  /*     free_node_data(rm, free_func); */
  /*     --l->size; */
  /*   } */
  /*   else { */
  /*     n = n->next; */
  /*   } */
  /* } */
  
  /* n = l->first; */
  /* if (predicate(n->name, n->data)) { */
  /*   l->first = l->first->next; */
  /*   free_node_data(n, free_func); */
  /*   --l->size; */
  /* } */
}



int list_size(list l)
{
  return l->size;
}



char *pop_name(list l)
{
  if (empty(l))
    return NULL;
  node *n = l->first;
  l->first = l->first->next;
  char *str = strdup(n->name);
  free_node(n);
  return str;
}
