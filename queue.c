#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#undef __LIST_HAVE_TYPEOF
/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head) {
        return;
    }
    element_t *entry = NULL, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        list_del(&entry->list);
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    int len = strlen(s) + 1;
    new->value = malloc(len);
    if (!(new->value)) {
        free(new);
        return false;
    }
    memcpy(new->value, s, len);
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    size_t len = strlen(s) + 1;
    new->value = malloc(len);
    if (!(new->value)) {
        free(new);
        return false;
    }
    memcpy(new->value, s, len);
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_node = list_first_entry(head, element_t, list);
    list_del(&remove_node->list);
    if (sp) {
        strncpy(sp, remove_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_node = list_last_entry(head, element_t, list);
    list_del(&remove_node->list);
    if (sp) {
        strncpy(sp, remove_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    struct list_head *node;
    int size = 0;
    list_for_each (node, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *slow = head->next, *fast = slow->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    if (fast != head)
        slow = slow->next;
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    element_t *entry = NULL, *safe;
    bool dup = false;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (entry->list.next != head &&
            strcmp(entry->value,
                   list_entry(entry->list.next, element_t, list)->value) == 0) {
            dup = true;
            list_del(&entry->list);
            q_release_element(entry);
        } else if (dup) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    struct list_head *node = head->next;
    while (node != head && node->next != head) {
        struct list_head *node2 = node->next;
        list_del(node);
        list_add(node, node2);
        node = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *pos, *safe;
    list_for_each_safe (pos, safe, head)
        list_move(pos, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    struct list_head *pos, *safe, *cut = head;
    int count = 0;
    list_for_each_safe (pos, safe, head) {
        count++;
        if (count % k == 0) {
            LIST_HEAD(tmp);
            count = 0;
            list_cut_position(&tmp, cut, pos);
            q_reverse(&tmp);
            list_splice(&tmp, cut);
            cut = safe->prev;
        }
    }
}

void _list_merge(struct list_head *left, struct list_head *right, bool descend)
{
    struct list_head head;
    INIT_LIST_HEAD(&head);
    while (!list_empty(left) && !list_empty(right)) {
        int cmp = strcmp(list_first_entry(left, element_t, list)->value,
                         list_first_entry(right, element_t, list)->value);
        if ((descend && cmp > 0) || (!descend && cmp <= 0)) {
            list_move_tail(left->next, &head);
        } else {
            list_move_tail(right->next, &head);
        }
    }
    if (!list_empty(right))
        list_splice_tail(right, &head);
    list_splice(&head, left);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *mid = head;
    int size = q_size(head) / 2;
    for (int i = 0; i < size; i++)
        mid = mid->next;
    struct list_head left, right;
    INIT_LIST_HEAD(&left);
    INIT_LIST_HEAD(&right);
    list_cut_position(&left, head, mid);
    list_cut_position(&right, head, head->prev);
    q_sort(&left, descend);
    q_sort(&right, descend);
    _list_merge(&left, &right, descend);
    list_splice(&left, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *right = list_entry(head->prev, element_t, list);
    element_t *left = list_entry(head->prev->prev, element_t, list);
    while (&left->list != head) {
        if (strcmp(right->value, left->value) < 0) {
            list_del(&left->list);
            element_t *tmp = left;
            left = list_entry(left->list.prev, element_t, list);
            free(tmp->value);
            free(tmp);

        } else {
            left = list_entry(left->list.prev, element_t, list);
            right = list_entry(right->list.prev, element_t, list);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;
    element_t *right = list_entry(head->prev, element_t, list);
    element_t *left = list_entry(head->prev->prev, element_t, list);
    while (&left->list != head) {
        if (strcmp(right->value, left->value) > 0) {
            list_del(&left->list);
            free(left->value);
            free(left);
            left = list_entry(right->list.prev, element_t, list);
        } else {
            left = list_entry(left->list.prev, element_t, list);
            right = list_entry(right->list.prev, element_t, list);
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    else if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);
    queue_contex_t *tmp = NULL,
                   *first = list_first_entry(head, queue_contex_t, chain);
    list_for_each_entry (tmp, head, chain) {
        if (tmp && tmp->q && tmp != first) {
            list_splice_tail_init(tmp->q, first->q);
            first->size += tmp->size;
            tmp->size = 0;
        }
    }
    q_sort(first->q, descend);
    return first->size;
}
