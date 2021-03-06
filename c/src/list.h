#ifndef LIST_H
#define LIST_H


typedef struct list * list;
typedef struct node * iterator;


list new_list();
void insert(list l, const char *name, void *data);
void insert_noalloc(list l, char *name, void *data);
int insert_one(list l, const char *name, void *data);
int insert_one_noalloc(list l, char *name, void *data);
int mem(list l, const char *name);
int find(void **data, list l, const char *name);
int findn(void **data, list l, const char *name, unsigned int len);
int memn(list l, const char *name, unsigned int len);
int rm_free_data(list l, const char *name, void (*free_func)(void *));
int rm(list l, const char *name);
int rmn_free_data(list l, const char *name, unsigned int len, void (*free_func)(void *));
int rmn(list l, const char *name, unsigned int len);
void rm_all_if(list l, int (*predicate) (const char *, void *));
void rm_all_if_free_data(list l, int (*predicate) (const char *, void *), void (*free_func) (void *));

void iter(list l, void (*func)(char *, void *));

iterator get_iterator(list l);
void iterate(iterator *i);
char *iterator_getname(iterator i);
void *iterator_getdata(iterator i);

int list_size(list l);

char *pop_name(list l);


#endif /* LIST_H */
