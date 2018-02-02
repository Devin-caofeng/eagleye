#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/list.h"


List *AddToListTail(List *head, void *data) {
    List *new_node = (List *)malloc(sizeof(List));
    if (new_node  == NULL) return NULL;

    new_node->next = NULL;
    new_node->data = data;

    if (head == NULL) {
        new_node->prev = NULL;
        return new_node;
    }

    List *cur = head;
    while (cur->next != NULL) cur = cur->next;

    cur->next = new_node;
    new_node->prev = cur;

    return new_node;
}

List *AddToListHead(List *head, void *data) {
    List *new_node = (List *)malloc(sizeof(List));
    if (new_node == NULL) return NULL;

    new_node->data = data;

    if (head == NULL) {
        new_node->prev = NULL;
        new_node->next = NULL;
        return new_node;
    }

    new_node->next = head->next;
    new_node->next->prev = new_node;
    head->next = new_node;
    new_node->prev = head;

    return new_node;
}

List *DelFromList(List *head, void *data) {
    if (head == NULL) return NULL;

    List *new_head = head;
    List *cur = head;
    for (; cur != NULL; cur = cur->next) {
        if (cur->data == data) {
            if (head->data == data) new_head = head->next;

            List *temp = cur;
            cur->next->prev = cur->prev;
            cur->prev->next = cur->next;
            free(temp);
        }
    }

    return new_head;
}

List *DestroyList(List *head) {
    List *cur = head;
    while (cur != NULL) {
        List *temp = cur;
        cur = cur->next;
        free(temp);
    }

    return NULL;
}
