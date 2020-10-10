#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include "proj.h"

void check_allocated_mem(void *ptr)
{
  if(!ptr){
    fprintf(stderr, "Allocation error\n");
    exit(1);
  }
}

/* Return command execution status: SLNT(silent) or NORM*/
int form_exec_argv(char **dest, const list *src, const int *len)
{
  int i;
  if(src->next == NULL){
    dest[0] = src->str;
    dest[1] = NULL;
    return NORM;
  }
  for(i = 0; i < *len - 1; i++){
    dest[i] = src->str;
    src = src->next;
  }
  if(src->str[0] == '&'){
    dest[i] = NULL;
    return SLNT;
  }
  dest[i] = src->str;
  dest[i+1] = NULL;
  return NORM;
}

void exec_command(char *const *argv, const int status)
{
  int pid;
  if(status == NORM){
    switch(pid = fork()){
      case -1:
        perror("fork");
        exit(1);
      case 0:
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(1);
      default:
        wait(NULL);
    }
  }
}

void list_free(list *head)
{
  list *tmp;
  while(head != NULL){
    tmp = head;
    head = head->next;
    free(tmp->str);
    free(tmp);
  }
}

void list_write_char(list *head, int *pos, 
                     const char *tmp, 
                     const int *def_str_size)
{
  /* If the string length is <n> times <def_str_size>*/
  /* reallocate <n+1>*<def_str_size> as much memory for it*/
  /* Handle only boundary case*/
  if((*pos-1) / *def_str_size != *pos / *def_str_size){
    head->str = realloc(head->str, *def_str_size * (*pos / *def_str_size+1));
    check_allocated_mem(head->str);
  }
  head->str[*pos] = *tmp;
  (*pos)++;
}

/* Allocate memory for next sring*/
void list_allocate_next(list **head, const int *def_str_size)
{
  (*head)->next = malloc(sizeof(**head));
  check_allocated_mem((*head)->next);
  *head = (*head)->next;
  (*head)->str = calloc(*def_str_size + 1, sizeof(*(*head)->str));
  check_allocated_mem((*head)->str);
  (*head)->next = NULL;
}

void list_init(list **head, const int *def_str_size)
{
  *head = malloc(sizeof(**head));
  check_allocated_mem(*head);
  (*head)->str = calloc(*def_str_size + 1, sizeof(*(*head)->str));
  check_allocated_mem((*head)->str);
  (*head)->next = NULL;
}

void list_print(const list *head, const int n)
{
  int i;
  for(i = 0; head != NULL && i < n; i++){
    printf("[%s]\n", head->str);
    head = head->next;
  }
}

void list_exec(const list *head, const int *len)
{ 
  int exec_status;
  char **exec_argv;
  /* Add one argument for NULL pointer*/
  exec_argv = malloc((*len+1) * sizeof(*exec_argv)); 
  exec_status = form_exec_argv(exec_argv, head, len);
  exec_command(exec_argv, exec_status);
  free(exec_argv);
}

void prompt()
{
  long size = pathconf(".", _PC_PATH_MAX);
  char cwd[size];
  if (getcwd(cwd, size) != NULL) {
     printf(BLU "%s" RESET " " RED "$" RESET " ", cwd);
  } else {
     perror("getcwd");
     exit(1);
  }
}

int check_errors(const int *brace_flag, 
                 const int *eow_flag, 
                 const int *str_count)
{
  /* Handle odd number of braces case*/
  if(*brace_flag)
    return ODDBR;
  /* Current line ends with spaces*/
  if(*eow_flag){
    return *str_count - 1; 
  }
  return *str_count;
}

/* Return EOF exception, ODDBR exception or number of strings readed*/
int read_str(list *head, const int *def_str_size)
{
  char tmp;
  int str_len = 0, str_count = 1;
  int brace_flag = 0;             /* Open br => 1, Cl br => 0*/
  int eow_flag = 0;               /* End of word flag*/
  prompt();
  while((tmp = getchar()) != '\n'){
    switch(tmp){
      case EOF:
        return EOF;
      case '"':
        brace_flag = !brace_flag;
        continue;
      case ' ': 
        if(eow_flag)              /* Handle multiple spaces case*/
          continue; 
        if(brace_flag){           /* Handle "abc def" case*/
          list_write_char(head, &str_len, &tmp, def_str_size);
          continue;
        }
        eow_flag = 1; str_len = 0; str_count += !brace_flag;   
        list_allocate_next(&head, def_str_size);
        continue;
    }
    eow_flag = 0;
    list_write_char(head, &str_len, &tmp, def_str_size);
  }
  return check_errors(&brace_flag, &eow_flag, &str_count); 
}

int stream(const int def_str_size)
{
  int res, str_count = 0;
  list *head;

  list_init(&head, &def_str_size);
  res = read_str(head, &def_str_size);
  while(res != EOF){
    if(res == ODDBR){
      fprintf(stderr, "Odd number of quotation marks\n");
    } else { 
      if(*head->str != '\0') { /* Check empty input*/
        list_exec(head, &res);
        str_count += 1;
      }
    }
    list_free(head);
    list_init(&head, &def_str_size);
    res = read_str(head, &def_str_size);
  }
  list_free(head);
  return str_count;  
}
