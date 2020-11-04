#include <stdlib.h>

#include "list.h"
#include "shell.h"

void list_free(list *head)
{
  list *tmp;
  while ((tmp = head) != NULL){
    head = head->next;
    free(tmp->str);
    free(tmp);
  }
}

void list_rm_last(list *head)
{
  list *tmp = head;
  /* Do not remove last element from list*/
  if(head->next == NULL) return;
  while(tmp->next->next != NULL)
    tmp = tmp->next;
  free(tmp->next->str);
  free(tmp->next);
  tmp->next = NULL;
}

/* Allocate memory for next sring*/
void list_allocate_next(list **head, const int *def_str_size)
{
  (*head)->next = malloc(sizeof(**head));
  *head = (*head)->next;
  (*head)->str = calloc(*def_str_size + 1, sizeof(*(*head)->str));
  (*head)->next = NULL;
}

void list_init(list **head, const int *def_str_size)
{
  *head = malloc(sizeof(**head));
  (*head)->str = calloc(*def_str_size + 1, sizeof(*(*head)->str));
  (*head)->next = NULL;
}

void list_exec(const list *head, const int *len, const int *exec_stat)
{ 
  char **exec_argv;
  int cd_check_res;
  /* Add position for NULL pointer*/
  exec_argv = malloc((*len+1) * sizeof(*exec_argv)); 
  form_exec_argv(exec_argv, head, len);
  cd_check_res = check_cd(exec_argv);
  if(cd_check_res == 0)
    exec_cd(exec_argv[1]);
  if(cd_check_res == NOTCD)
    exec_command(exec_argv, exec_stat);
  free(exec_argv);
}
