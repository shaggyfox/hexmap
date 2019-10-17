typedef struct dlist_st dlist;

struct dlist_entry {
  struct dlist_entry *next;
  struct dlist_entry *prev;
  void *data;
};

struct dlist_st {
  struct dlist_entry *begin;
  struct dlist_entry *end;
};

typedef struct dlist_entry dlist_iter;

void dlist_append(dlist *list, void *data);
void dlist_prepend(dlist *list, void *data);
void dlist_clear(dlist *list, void(*free_cb)(void*));
dlist_iter *dlist_begin(dlist *list);
dlist_iter *dlist_next(dlist_iter *iter);
void *dlist_data(dlist_iter *iter);
