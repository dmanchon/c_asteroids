#ifndef __LISTLH_
#define __LISTLH_

typedef struct list_node list_node;
struct list_node {
    void* data;
    list_node *next;
};

typedef struct list list;
struct list {
    list_node *head;
    int size;
};


list* list_init();
void list_push(list* l, void* data);
void* list_pop(list* l);
void list_insert_after(list* l, void* data, int index);
list_node* list_get(list* l, int index);
void list_delete(list *l, int index);
list_node *list_delete_node(list *l, list_node *node);
void list_free(list* l);

#endif