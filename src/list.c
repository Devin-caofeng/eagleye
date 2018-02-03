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
    List *list;
    List *tmp;

    if (head == NULL) {
        return NULL;
    }
    tmp = head;
    while(tmp->next) tmp = tmp->next;
    ForEachList(tmp, list) {
        if (list->data == data) {
            //若当前节点就是头结点，则将当前节点的下一个节点返回作为头结点
            if (list == tmp) {
                tmp = list->next;
            }
            //若当前节点的上一个节点存在则将链表补全，即下一个的指针指向当前节点的下一个处
            if (list->prev) {
                list->prev->next = list->next;
            }
            //若当前节点的下一个节点存在则将链表补全，即上一个的指针指向当前节点的上一个处
            if (list->next) {
                list->next->prev = list->prev;
            }
            //释放当前节点,注意：并未释放当前节点中data的空间,需要手动释放data的空间
//            free(list->data);
            free(list);
            break;
        }
    }

            /* if (cur == head) {           */
            /*     new_head = head->next;   */
            /*     cur->next->prev = NULL;  */
            /*     free(cur->data);         */
            /*     free(cur);               */
            /* }                            */
            /* List *temp = cur;            */
            /* cur->next->prev = cur->prev; */
            /* cur->prev->next = cur->next; */
            /* free(temp->data);            */
            /* free(temp);                  */
    return tmp;
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
