#include "error_handling.h"
#include "type/list_t.h"

void add_element_to_end(list_entry_t *element, list_t *list)
{
    ASSERT(list, "Bad node attempted to add to list\n");

    element->next = NULL;
    element->prev = list->tail;

    if (list->head == NULL)
    {
        list->head = element;
    }

    if (list->tail != NULL)
    {
        list->tail->next = element;
    }
    list->tail = element;
}

void remove_element(list_entry_t *element, list_t *list)
{
    ASSERT(list, "Bad node attempted to remove from list\n");
    if (element->next != NULL)
    {
        element->next->prev = element->prev;
    }
    else
    {
        list->tail = element->prev;
    }
    if (element->prev != NULL)
    {
        element->prev->next = element->next;
    }
    else
    {
        list->head = element->next;
    }
}
