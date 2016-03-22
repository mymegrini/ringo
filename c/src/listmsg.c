#include "common.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef struct node node;

struct node {
    node *next;
    char *idm;
};

typedef struct list {
    node *first;
    node *last;
    int size;
} list;


static list l = { NULL, NULL };


static node *newnode(char *idm, node *next) {
    node *n = (node *)malloc(sizeof(node));
    n->idm = (char *)malloc(strlen(idm) + 1);
    strcpy(n->idm, idm);
    n->next = next;
    return n;
}

static int isempty() {
    return l.first == NULL;
}

static void freenode(node *n) {
    free(n->idm);
    free(n);
}



void freelist() {
    node *n = l.first;
    while (n != NULL) {
        node *next = n->next;
        freenode(n);
        n = next;
    }
    //free(l);
}


/**
 * lookup search for an element and remove/add it if found/not found.
 *
 * @param the list
 * @param the item
 * @return 1 if found, 0 else
 */
int lookup(char *idm) {
    // empty list
    if (isempty(l)) {
        l.first = newnode(idm, NULL);
        l.last  = l.first;
        ++l.size;
        return 0;
    }
    // found in first position
    if (strcmp(l.first->idm, idm) == 0) {
        node *n = l.first;
        l.first = l.first->next;
        freenode(n);
        --l.size;
        return 1;
    }
    // find elsewhere
    for (node *i = l.first; i->next != NULL; i = i->next) {
        if (strcmp(i->next->idm, idm) == 0) {
            // actualise last element if needed
            if (i->next == l.last)
                l.last = i;
            node *n = i->next;
            i->next = i->next->next;
            freenode(n);
            --l.size;
            return 1;
        }
    }
    // not found
    l.last->next = newnode(idm, NULL);
    l.last = l.last->next;
    ++l.size;
    return 0;
}


void printmsg() {
    printf(UNDERLINED "LIST:" RESET "\n");
    if (isempty(l))
        printf("empty\n");
    else {
        int i = 0; 
        for (node *n = l.first; n != NULL; n = n->next, i++)
            printf(UNDERLINED "%4d:\t%s\n", n->idm);
    }
}


int nmessage() {
    return l.size;
}
