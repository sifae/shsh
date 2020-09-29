#include <stdio.h>
#include <stdlib.h>
#include "proj.h"

void free_list(LIST *head)
{
  LIST *tmp;
  while(head != NULL){
    tmp = head;
    head = head->next;
    free(tmp->str);
    free(tmp);
  }
}

void check_allocated_mem(void *ptr)
{
  if(!ptr){
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }
}

void list_write_char(LIST *buf, char *tmp, int *pos, const int *def_str_size)
{
  /*If the string length is <n> times <def_str_size>*/
  /*reallocate <n+1>*<def_str_size> as much memory for it*/
  /*Handle only boundary case*/
  if((*pos-1) / *def_str_size != *pos / *def_str_size){
    buf->str = realloc(buf->str, *def_str_size * (*pos / *def_str_size+1));
    check_allocated_mem(buf->str);
  }
  *(buf->str+*pos) = *tmp;
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
void list_allocate_next(LIST **buf_it, const int *def_str_size)
{
  (*buf_it)->next = malloc(sizeof(**buf_it));
  check_allocated_mem((*buf_it)->next);
  *buf_it = (*buf_it)->next;
  (*buf_it)->str = calloc(*def_str_size, sizeof(*(*buf_it)->str));
  check_allocated_mem((*buf_it)->str);
}

void reset_vars(
    int *str_len, int *str_count, 
    int *brace_flag, int *eow_flag, 
    const int *def_str_size,
    LIST **buf, LIST **buf_it 
    )
{
  *str_len = 0; *str_count = 1; *brace_flag = 0; *eow_flag = 0;
  *buf = malloc(sizeof(**buf));
  check_allocated_mem(*buf);
  (*buf)->str = calloc(*def_str_size, sizeof(*(*buf)->str));
  check_allocated_mem((*buf)->str);
  *buf_it = *buf;
}

/*Description in the header file*/  
void print_str(LIST *buf, int n)
{
  int i;
  for(i = 0;buf != NULL && i < n; i++){
    printf("[%s]\n", buf->str);
    buf = buf->next;
  }
}

int read_str(const int def_str_size)
{
  char tmp;           /* Temp var to store current char*/
  int brace_flag;     /* Open br => 1, Cl br => 0*/
  int eow_flag;       /* End of word flag(just to handle multiple spaces)*/
  int cur_str_count;  /* Current Nubmer of entered strings*/
  int cur_str_len;    /* Current string lenght*/
  int gl_str_count=0; /* Global string counter*/
  LIST *head;         /* Buffer with strings*/
  LIST *buf_it;       /* Buffer 'iterator'*/

  reset_vars(&cur_str_len, &cur_str_count, &brace_flag, 
      &eow_flag, &def_str_size, &head, &buf_it);

  printf(">> ");
  while((tmp = getchar()) != EOF){
    switch(tmp) {
      case '\n':
        buf_it->next = NULL;
        /*Current line ends with spaces*/
        if(eow_flag){
          cur_str_count--;
          list_rm_last(head);
        } else {
          /*Add 'end of string' char*/
          *(buf_it->str+cur_str_len) = '\0';
        }
        /*Handle odd number of braces case*/
        if(brace_flag){
          fprintf(stderr, "Odd number of quotation marks\n");
        } else {
          /*Print entered strings*/
          print_str(head, cur_str_count);
        }
        printf(">> ");        
        /*Reset everything*/
        free_list(head);
        gl_str_count += cur_str_count;
        reset_vars(&cur_str_len, &cur_str_count, &brace_flag,
            &eow_flag, &def_str_size, &head, &buf_it);
        continue;
      case '"':
        cur_str_count += brace_flag;
        brace_flag = !brace_flag;
        continue;
      case ' ':
        /*Handle multiple spaces case*/
        if(eow_flag) 
          continue;
        /*Handle "abc def" case*/
        if(brace_flag){
          list_write_char(buf_it, &tmp, &cur_str_len, &def_str_size);
          continue;
        }
        /*Add 'end of string' char*/
        *(buf_it->str+cur_str_len) = '\0';
        eow_flag = 1; cur_str_len = 0; cur_str_count += !brace_flag;   
        list_allocate_next(&buf_it, &def_str_size);
        continue;
    }
    eow_flag = 0;
    list_write_char(buf_it, &tmp, &cur_str_len, &def_str_size);
  }
  buf_it->next = NULL;
  free_list(head);
  return gl_str_count;
}
