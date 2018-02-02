#ifndef LIST_H__
#define LIST_H__

typedef struct list {
    void *data;
    struct list *next;
    struct list *prev;
} List;

List *AddToListTail(List *head, void *data);
List *AddToListHead(List *head, void *data);
List *DelFromList(List *head, void *data);
List *DestroyList(List *head);

#define ForEachList(head, list) \
            for(list = head; list != NULL; list = list->next)

#endif /* LIST_H__ */
