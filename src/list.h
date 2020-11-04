#ifndef LIST_H
#define LIST_H

typedef struct list list; 

struct list {
  char *str;
  struct list *next;
};

void list_free(list *head);
void list_rm_last(list *head);
void list_allocate_next(list **head, const int *def_str_size);
void list_init(list **head, const int *def_str_size);
void list_exec(const list *head, const int *len, const int *exec_stat);

#endif /* LIST_H*/
