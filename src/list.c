#include "list.h"
#include <stdlib.h>
#include <assert.h>


list* list_init() {
    list *l = malloc(sizeof(list));
    assert(l != NULL);

    l->head = NULL;
    l->size = 0;

    return l;
}

void list_push(list* l, void* data) {
    list_node *n = l->head;
    list_node *new = malloc(sizeof(list_node));
    assert(new != NULL);

    new->data = data;
    new->next = NULL;

    // empty list
    if (!l->head) {
        l->head = new;
    } else {
        // find the last node
        while (n->next) {
            n = n->next;
        }
        n->next = new;
    }
    l->size++;
}


void* list_pop(list* l) {
    // empty list
    if (!l->head) {
        return NULL;
    }

    // find last
    list_node *prev = l->head;
    list_node *n = prev->next;

    // last element
    if (!n) {
        l->head = NULL;
        l->size = 0;
        void *data = prev->data;
        free(prev);
        return data;
    }

    // +1 elements
    while (n->next) {
        prev = n;
        n = prev->next;
    }

    void *data = prev->next->data;
    
    free(n);
    prev->next = NULL;
    l->size--;
    return data;
}

void list_insert_after(list* l, void* data, int index) {
    assert(index < l->size);

    // find the node
    list_node *n = l->head;
    for (int i = 0; i < index; i++) {
        n = n->next;
    }

    list_node *new = malloc(sizeof(list_node));
    assert(new != NULL);

    new->data = data;
    new->next = n->next;

    l->size++;
    n->next = new;
}

void* list_get(list* l, int index) {
    if (!l->head || l->size <= index) {
        return NULL;
    }

    list_node *n = l->head;
    for (int i = 0; i < index; i++) {
        n = n->next;
    }

    return n->data;
}

void list_delete(list *l, int index) {
    if (!l->head) {
        return;
    }

    if (l->size < index) {
        return;
    }

    list_node *prev;
    list_node *n = l->head;

    for (int i = 0; i < index; i++) {
        prev = n;
        n = n->next;
    }

    prev->next = n->next;
    free(n);
    l->size--;
}

void list_free(list* l) {
    list_node *n = l->head;
    while (n) {
        list_node *d = n;
        n = n->next;
        free(d);
    }
    free(l);
}

// remove the node and return the next
list_node *list_delete_node(list *l, list_node *node) {
    list_node *prev = NULL;
    list_node *n = l->head;
    
    while (n) {
        if (n == node) {
            if (!prev) {
                l->head = n->next;
                free(n);
                l->size--;
                // return the next
                return l->head;
            } else {
                prev->next = n->next;
                free(n);
                l->size--;
                // return the next
                return prev->next;
            }            
        }
        prev = n;
        n = n->next;
    }
    return NULL;
}
