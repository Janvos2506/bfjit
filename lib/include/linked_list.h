#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct LinkedList
{
    void *data;
    LinkedList *next;
} LinkedList;

#endif // LINKED_LIST_H