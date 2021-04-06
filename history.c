#include <stddef.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "history.h"
#include "clist.h"


struct clist *list;
struct clist_iterator iter;

void hist_init(unsigned int limit)
{
    // TODO: set up history data structures, with 'limit' being the maximum
    // number of entries maintained.
    list = clist_create(limit, 256);
    iter = clist_create_iter();

}

void hist_destroy(void)
{
    clist_destroy(list);
}

void hist_add(const char *cmd)
{
    // strdup
    char *copy_of_cmd = strdup(cmd);
    clist_add(list, copy_of_cmd);
    free(copy_of_cmd);
}

void hist_print(void)
{
    void *elem;
    while ((elem = clist_iterate_rev(list, &iter)) != NULL) {
        char *ie = (char *) elem;
        unsigned int index;
        if (list->insertions < 100) {
            index = iter.idx;
        } else {
            index = list->insertions - 100 + iter.idx;
        }
        printf("%u %s\n", index, ie);
    }
}

const char *hist_search_prefix(char *prefix)
{
    // TODO: Retrieves the most recent command starting with 'prefix', or NULL
    // if no match found.

    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    // TODO: Retrieves a particular command number. Return NULL if no match
    // found.
    return clist_get(list, command_number);

}

unsigned int hist_last_cnum(void)
{
    // TODO: Retrieve the most recent command number.
    return list->insertions - 1;
    //return clist_insertions(list) - 1;
}
