#ifndef LIST_H
#define LIST_H


typedef struct list * list;


list new_list();
void insert(list l, const char *name, void *data);
int mem(list l, const char *name);
int find(void **data, list l, const char *name);
int rm_free_data(list l, const char *name);
int rm(list l, const char *name);

void iter(list l, void (*func)(char *, void *));

#endif /* LIST_H */
