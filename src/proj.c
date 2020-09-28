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

void reset_vars(
    int *str_len, int *str_count, 
    int *brace_flag, int *eow_flag, 
    LIST **buf, LIST **buf_ptr 
    )
{
  *str_len = 0; *str_count = 1; *brace_flag = 0; *eow_flag = 0;
  *buf = malloc(sizeof(**buf));
  check_allocated_mem(*buf);
  (*buf)->str = calloc(DEF_STR_SIZE, sizeof(*(*buf)->str));
  check_allocated_mem((*buf)->str);
  *buf_ptr = *buf;
}

void print_str(LIST *buf, int n){
  //Description in the header file
  int i;
  for(i = 0;buf != NULL && i < n; i++){
   printf("[%s]\n", buf->str);
   buf = buf->next;
  }
}

void list_write_char(LIST *buf, char *tmp, int *pos)
{
  //If the string length is <n> times <DEF_STR_SIZE>
  //reallocate <n+1>*<DEF_STR_SIZE> as much memory for it
  //Handle only boundary case
  if((*pos-1)/DEF_STR_SIZE != *pos/DEF_STR_SIZE){
    buf->str = realloc(buf->str,DEF_STR_SIZE*(*pos/DEF_STR_SIZE+1));
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

void read_str()
{  
  char tmp;       //Temp var to store current char
  int brace_flag; //Open br => 1, Cl br => 0
  int eow_flag;   //End of word flag(just to handle multiple spaces)
  int str_count;  //Nubmer of entered strings
  int str_len;    //Current string lenght
  LIST *head;     //Buffer with strings
  LIST *buf_ptr;  //Buffer 'iterator'

  reset_vars(&str_len, &str_count, &brace_flag, 
      &eow_flag, &head, &buf_ptr);

  printf(">> ");
  while((tmp = getchar()) != EOF){
    switch(tmp) {
      case '\n':
        buf_ptr->next = NULL;
        //Current line ends with spaces
        if(eow_flag){
          str_count--;
          list_rm_last(head);
        } else {
          //Add 'end of string' char
          *(buf_ptr->str+str_len) = '\0';
        }
        //Handle odd number of braces case
        if(brace_flag){
          fprintf(stderr, "Odd number of quotation marks\n");
        } else {
          //Print entered strings
          print_str(head, str_count);
        }
        printf(">> ");        
        //Reset everything
        free_list(head);
        reset_vars(&str_len, &str_count, &brace_flag,
            &eow_flag, &head, &buf_ptr);
        continue;
      case '"':
        str_count += brace_flag;
        brace_flag = !brace_flag;
        continue;
      case ' ':
        //Handle multiple spaces case
        if(eow_flag) continue;
        //Handle "abc def" case
        if(brace_flag){
          list_write_char(buf_ptr, &tmp, &str_len);
          continue;
        }
        //Add 'end of string' char
        *(buf_ptr->str+str_len) = '\0';
        eow_flag = 1; str_len = 0; str_count += !brace_flag;
        
        //Allocate memory for next sring
        buf_ptr->next = malloc(sizeof(*buf_ptr));
        check_allocated_mem(buf_ptr->next);
        buf_ptr = buf_ptr->next;
        buf_ptr->str = calloc(DEF_STR_SIZE, sizeof(*buf_ptr->str));
        check_allocated_mem(buf_ptr->str);
        continue;
    }
    eow_flag = 0;
    list_write_char(buf_ptr, &tmp, &str_len);
  }
  buf_ptr->next = NULL;
  free_list(head);
}
