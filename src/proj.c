#include <stdio.h>
#include <stdlib.h>
#include "proj.h"

void check_allocated_mem(void *ptr)
{
  if(!ptr){
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }
}

void list_free(LIST *head)
{
  LIST *tmp;
  while(head != NULL){
    tmp = head;
    head = head->next;
    free(tmp->str);
    free(tmp);
  }
}

void list_write_char(LIST *head, char *tmp, int *pos, const int *def_str_size)
{
  /*If the string length is <n> times <def_str_size>*/
  /*reallocate <n+1>*<def_str_size> as much memory for it*/
  /*Handle only boundary case*/
  if((*pos-1) / *def_str_size != *pos / *def_str_size){
    head->str = realloc(head->str, *def_str_size * (*pos / *def_str_size+1));
    check_allocated_mem(head->str);
  }
  *(head->str+*pos) = *tmp;
  (*pos)++;
}

void list_rm_last(LIST *head)
{
  if(head->next == NULL) { free(head); return; }
  LIST *tmp = head;
  while(tmp->next->next != NULL){
    tmp = tmp->next;
  }
  free(tmp->next->str); free(tmp->next);
  tmp->next = NULL;
}

/*Allocate memory for next sring*/
void list_allocate_next(LIST **head, const int *def_str_size)
{
  (*head)->next = malloc(sizeof(**head));
  check_allocated_mem((*head)->next);
  *head = (*head)->next;
  (*head)->str = calloc(*def_str_size, sizeof(*(*head)->str));
  check_allocated_mem((*head)->str);
  (*head)->next = NULL;
}

void list_init(LIST **head, const int *def_str_size)
{
  *head = malloc(sizeof(**head));
  check_allocated_mem(*head);
  (*head)->str = calloc(*def_str_size, sizeof(*(*head)->str));
  check_allocated_mem((*head)->str);
  (*head)->next = NULL;
}

void list_print(LIST *head, int n)
{
  int i;
  for(i = 0; head != NULL && i < n; i++){
    printf("[%s]\n", head->str);
    head = head->next;
  }
}

int check_errors(int *brace_flag, int *eow_flag, int *str_count)
{
  /*Handle odd number of braces case*/
  if(*brace_flag)
    return ODDBR;
  /*Current line ends with spaces*/
  if(*eow_flag){
    return *str_count - 1;
  } 
  return *str_count;
}

int read_str(LIST *head, const int *def_str_size)
{
  char tmp;
  int str_len = 0, str_count = 1;
  int brace_flag = 0;             /* Open br => 1, Cl br => 0*/
  int eow_flag = 0;               /* End of word flag*/
  printf(">> ");
  while((tmp = getchar()) != '\n'){
    switch(tmp){
      case EOF:
        return EOF;
      case '"':
        brace_flag = !brace_flag;
        str_count += (str_count==1) ? 0 : brace_flag; /*single word in br */
        continue;
      case ' ':
        /*Handle multiple spaces case*/
        if(eow_flag) 
          continue;
        /*Handle "abc def" case*/
        if(brace_flag){
          list_write_char(head, &tmp, &str_len, def_str_size);
          continue;
        }
        head->str[str_len] = '\0';
        eow_flag = 1; str_len = 0; str_count += !brace_flag;   
        list_allocate_next(&head, def_str_size);
        continue;
    }
    eow_flag = 0;
    list_write_char(head, &tmp, &str_len, def_str_size);
  } 
  head->str[str_len] = '\0';
  return check_errors(&brace_flag, &eow_flag, &str_count); 
}

int stream(const int def_str_size)
{
  int res, str_count = 0;
  LIST *head;
  list_init(&head, &def_str_size);
  res = read_str(head, &def_str_size);

  while(res != EOF){
    if(res == ODDBR){
      fprintf(stderr, "Odd number of quotation marks\n");
    } else {
      str_count += res;
      list_print(head, res);
      list_free(head);
      list_init(&head, &def_str_size);
    }
    res = read_str(head, &def_str_size);
  }
  list_free(head);
  return str_count;  
}
