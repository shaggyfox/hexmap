#include <stdlib.h>
#include "list.h"

void dlist_append(dlist *list, void *data)
{
  struct dlist_entry *new_entry = malloc(sizeof(*new_entry));
  new_entry->data = data;
  new_entry->next = NULL;
  new_entry->prev = NULL;
  if (list->end) {
    new_entry->prev = list->end;
    list->end->next = new_entry;
    list->end = new_entry;
  } else {
    list->begin = list->end = new_entry;
  }
}

void dlist_prepend(dlist *list, void *data)
{
  struct dlist_entry *new_entry = malloc(sizeof(*new_entry));
  new_entry->data = data;
  new_entry->prev = NULL;
  new_entry->next = NULL;
  if (list->begin) {
    new_entry->next = list->begin;
    list->begin = new_entry;
  } else {
    list->begin = list->end = new_entry;
  }
}

void dlist_clear(dlist *list, void(*free_cb)(void*))
{
  struct dlist_entry *tmp;
  while((tmp=list->begin)) {
    if (free_cb) {
      free_cb(tmp->data);
    }
    list->begin = tmp->next;
    free(tmp);
  }
}

dlist_iter *dlist_begin(dlist *list)
{
  return list->begin;
}

dlist_iter *dlist_next(dlist_iter *iter)
{
  return iter->next;
}

void *dlist_data(dlist_iter *iter)
{
  return iter->data;
}
