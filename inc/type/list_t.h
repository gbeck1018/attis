#pragma once

typedef struct list_entry_t
{
    struct list_entry_t *next;
    struct list_entry_t *prev;
} list_entry_t;

typedef struct list_t
{
    list_entry_t *head;
    list_entry_t *tail;
} list_t;

#define container_of(ptr, type, name) \
    ((type *)(((char *)(ptr)) - ((size_t) & ((type *)0)->name)))

/**
 * @brief A convenient and safe for_each loop for the lists, starting from
 * a given element
 * @param[in, out] elem The starting element
 * @param[in] temp A temp element pointer to pre fetch the next value
 * @param[in] type The type of the struct that contains the list
 * @param[in] name The name of the list_t member in type
 * @note Set elem before using. temp is not safe to use.
 */
#define for_each_element_from(elem, temp, type, name)                         \
    for (temp                                                                 \
         = (elem == NULL ? NULL : container_of(elem->list.next, type, name)); \
         elem != NULL;                                                        \
         elem = temp,                                                         \
         temp                                                                 \
         = (elem == NULL ? NULL : container_of(elem->list.next, type, name)))

void add_element_to_end(list_entry_t *element, list_t *list);
void remove_element(list_entry_t *element, list_t *list);
