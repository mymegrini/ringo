#include "list.h"
#include "../protocol/common.h"

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



static int empty(list l)
{
  return l->first == NULL;
}



list new_list() 
{
  struct list *l = malloc(sizeof(struct list));
  l->first = NULL;
  l->last = NULL;
  return l;
}



int mem(list l, const char *name)
{
 void *nothing;
 return find(&nothing, l, name);
}



int find(void **data, list l, const char *name)
{
  if (empty(l))
    return 0;
  node *n = l->first;
  do {
    if (strcmp(n->name, name) == 0) {
      *data = n->data;
      return 1;
    }
  } while (n++ != l->last);
  return 0;
}



int insert_one(list l, const char *name, void *data)
{
  if (empty(l)) {
    l->first = new_node(name, data, NULL);
    l->last = l->last;
    return 1;
  }
  else if (mem(l, name))
    return 0;
  else {
    l->last->next = new_node(name, data, NULL);
    l->last = l->last->next;
    return 1;
  }
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
}



static void free_node(node *n)
{
  free(n->name);
  free(n);
}



static void free_node_data(node *n)
{
  free(n->name);
  free(n->data);
  free(n);
}



int rm_free_data(list l, const char *name)
{
  if (empty(l))
    return 0;
  node *n = l->first;
  if (strcmp(n->name, name) == 0) {
    l->first = n->next;
    free_node_data(n);
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strcmp(n->next->name, name) == 0) {
        node *rem = n->next;
        n->next = n->next->next;
        free_node_data(rem);
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
    l->first = n->next;
    free_node(n);
    return 1;
  }
  else {
    for ( ; n->next != NULL; n = n->next) {
      if (strcmp(n->next->name, name) == 0) {
        node *rem = n->next;
        n->next = n->next->next;
        free_node(rem);
        return 1;
      }
    }
    return 0;
  }
}



void iter(list l, void (*func)(char *, void *)) {
  if (empty(l))
    return;
  for (node *n = l->first; n != NULL; n = n->next)
    func(n->name, n->data);
}
