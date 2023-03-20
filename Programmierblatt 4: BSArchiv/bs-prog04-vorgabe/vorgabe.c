#include "vorgabe.h"
#include <assert.h>

void init(fileInfoList *list) {
    list->head = NULL;
    list->tail = &list->head;
}

void enqueue(fileInfoList *list, fileInfo *item) {
    assert(list);
    assert(item);
    item->next = NULL;
    *list->tail = item;
    list->tail = &item->next;
}

fileInfo *dequeue(fileInfoList *list) {
    assert(list);
    fileInfo *item = list->head;
    if (item) {
        list->head = item->next;
        if (!list->head) {
            list->tail = &list->head;
        } else {
            item->next = NULL;
        }
    }
    return item;
}
