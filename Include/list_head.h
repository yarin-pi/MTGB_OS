#ifndef LIST_H
#define LIST_H
struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

static inline void list_add(struct list_head *new, struct list_head *head)
{
    struct list_head *temp = head;
    while (temp->next != NULL)
        temp = temp->next;

    temp->next = new;
    new->prev = temp;
    new->next = NULL;
}

static inline void list_del(struct list_head *item)
{
    struct list_head *next = item->next;
    struct list_head *prev = item->prev;
    if (next != NULL)
        next->prev = prev;
    if (prev != NULL)
        prev->next = next;
    item->next = NULL;
    item->prev = NULL;
}

#endif LIST_H
