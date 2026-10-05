/*
 * dt_list.c: Lists for Unit 5, Section H.
 *
 * A cell holds a value and a pointer to the list tail.
 * CAR reads the value. CDR reads the tail. CONS creates a new first cell.
 *
 * CONS creates one cell and shares the supplied tail. After these commands:
 *
 *     list nil e
 *     list cons b 2 e
 *     list cons a 1 b
 *
 * List a is (1 2). List b is (2). Both lists reference the cell that holds 2.
 * CONS takes constant time and allocates one cell.
 *
 * dt_list_free releases one cell. Following the tail would release cells that
 * list b still uses.
 *
 * A null pointer represents the empty list.
 */

#include "dt.h"

#include <stdlib.h>

struct dt_list {
    dt_value head;
    dt_list *tail;
};

/*
 * dt_list_nil returns the null pointer that represents the empty list.
 */
dt_list *dt_list_nil(void)
{
       
    /*
     Returns a NULL pointer directly.
     The empty list is simply represented by NULL. This means 
     it requires zero memory allocation
     */
    return NULL;
}

/*
 * dt_list_cons builds a new cell that holds head and references tail.
 * The new cell shares the supplied tail.
 * The function returns NULL after an allocation failure.
 */
dt_list *dt_list_cons(dt_value head, dt_list *tail)
{

    /*
     Allocates exactly one dt_list cell, sets its head, and points to the provided tail.
     This guarantees constant-time insertion at the front and allows multiple 
     distinct lists to safely share the same tail cells in memory without copying them
     */
    dt_list *new_cell = malloc(sizeof(dt_list));
    if (!new_cell) {
        return NULL;
    }
    
    new_cell->head = head;
    new_cell->tail = tail;
    return new_cell;
}

/*
 * dt_list_free releases one cell and preserves its tail.
 * Another list can still reference the tail. The function accepts NULL.
 */
void dt_list_free(dt_list *l)
{

    /*
    Safely frees only the provided cell `l`. It does NOT recursively free `l->tail`.
    Because tails are explicitly shared across multiple lists, following the tail 
    and freeing it would destroy memory that another active list might still be using, 
    leading to a double-free crash in the sanitizers[cite: 22, 30, 37].
     */
    if (l) {
        free(l);
    }
}

/*
 * dt_list_len counts the cells. It visits each cell once.
 */
size_t dt_list_len(const dt_list *l)
{

    /*
    Traverses the linked list nodes until it reaches NULL, counting each step.
    A purely functional singly-linked list structure like this doesn't store a 
    unified length counter, so we must calculate it in linear time by walking the pointers
     */
    size_t count = 0;
    const dt_list *current = l;
    
    while (current != NULL) {
        count++;
        current = current->tail;
    }
    
    return count;
}

/*
 * dt_list_car writes the first cell value to *out.
 * It returns DT_ERR_EMPTY and does not change *out for an empty list.
 * A nil value differs from an absent value.
 */
dt_status dt_list_car(const dt_list *l, dt_value *out)
{

    /*
     Checks if the list is NULL; if so, returns DT_ERR_EMPTY. Otherwise writes the head.
     Returning an error code separates an actually empty list (absence of a cell) 
     from a list whose first valid cell just happens to contain the `nil` value.
     */
    if (!l) {
        return DT_ERR_EMPTY;
    }
    
    *out = l->head;
    return DT_OK;
}

/*
 * dt_list_cdr writes the tail to *out. It returns DT_ERR_EMPTY for an empty
 * list. A one-element list has an empty tail and returns DT_OK.
 */
dt_status dt_list_cdr(const dt_list *l, dt_list **out)
{

    /*
     Returns the tail pointer if the list is not empty.
     Asking for the rest of an empty list is meaningless 
     However, the tail of a one-element list is successfully returned as NULL (an empty list)
     */
    if (!l) {
        return DT_ERR_EMPTY;
    }
    
    *out = l->tail;
    return DT_OK;
}