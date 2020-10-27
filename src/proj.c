#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <fcntl.h>
#include "proj.h"

void check_allocated_mem(void *ptr)
{
  if(!ptr){
    fprintf(stderr, "Allocation error\n");
    exit(1);
  }
}

void form_exec_argv(char **dest, const list *src, 
                   const int *len)
{
  int i;
  for(i = 0; i < *len; i++){
    dest[i] = src->str;
    src = src->next;
  }
  dest[i] = NULL;
}

void stdio_to_null()
{
  int i = 0; 
  close(0);
  close(1);
  close(2);
  i=open("/dev/null", O_RDWR);
  dup(i);
  dup(i);
}

void exec_command(char *const *argv, const int *status)
{
  int pid, p;
  pid = fork();
  if(pid < 0){
    perror("fork");
    exit(1);
  }
  if(pid > 0){
    if(*status == SLNT){
      printf("%d Started\n", pid);
      while((p = waitpid(-1, NULL, WNOHANG)) > 0)
        printf("%d Terminated\n", p);
    } else {
      /* Terminate background processes in Normal mode*/
      while((p = wait(NULL)) != pid) 
        printf("%d Terminated\n", p);
    }
    return;
  }
  /* Child process*/
  if(*status == SLNT)
    stdio_to_null();
  execvp(argv[0], argv);
  perror(argv[0]);
  exit(1);
}

/* 0 - Not a cd command, 1 - correct cd command*/
/* -1 - Not correct cd command*/
int check_cd(char *const *argv)
{
  if(**argv == 'c' &&
     *(*argv+1) == 'd' && 
     *(*argv+2) == '\0'){
    if(argv[1] != NULL && argv[2] == NULL)
      return 1;
    else{ 
      fprintf(stderr, "cd: Wrong number of arguments\n");
      return -1;
    }
  }
  return 0;
}

void exec_cd(char const *argv)
{ 
  int res;
  res = chdir(argv);
  if(res != 0)
    fprintf(stderr, "'%s' Not a directory\n", argv);
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
                     const int *tmp, 
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

void list_exec(const list *head, const int *len, const int *exec_stat)
{ 
  char **exec_argv;
  int cd_check_res;
  /* Add position for NULL pointer*/
  exec_argv = malloc((*len+1) * sizeof(*exec_argv)); 
  form_exec_argv(exec_argv, head, len);
  cd_check_res = check_cd(exec_argv);
  if(cd_check_res == 1)
    exec_cd(exec_argv[1]);
  if(cd_check_res == 0)
    exec_command(exec_argv, exec_stat);
  free(exec_argv);
}

void prompt()
{
  long size = pathconf(".", _PC_PATH_MAX);
  char *cwd = malloc(size * sizeof(*cwd));
  check_allocated_mem(cwd);
  getcwd(cwd, size);
  printf("%s $ ", cwd);
  free(cwd);
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
/* Also set execution state flag - NORMAL or SILENT*/
int read_command(list *head, int *exec_stat, const int *def_str_size)
{
  int tmp;
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
        if(*head->str != '\0'){   /* Command starts with spaces*/
          list_allocate_next(&head, def_str_size);
          eow_flag = 1; str_len = 0; str_count += !brace_flag;   
        }
        continue;
      case '&':
        if(!brace_flag){ 
          *exec_stat = SLNT;
          continue;
        }
    }
    eow_flag = 0; 
    list_write_char(head, &str_len, &tmp, def_str_size);
  }
  return check_errors(&brace_flag, &eow_flag, &str_count); 
}

int stream(const int def_str_size)
{
  int res, str_count = 0;
  int exec_stat = NORM;
  list *head;

  list_init(&head, &def_str_size);
  res = read_command(head, &exec_stat, &def_str_size);
  while(res != EOF){
    if(res == ODDBR){
      fprintf(stderr, "Odd number of quotation marks\n");
    } else { 
      if(*head->str != '\0'){   /* Check empty input*/
        list_exec(head, &res, &exec_stat);
        str_count += 1;
      }
    }
    list_free(head);
    list_init(&head, &def_str_size);
    exec_stat = NORM;
    res = read_command(head, &exec_stat, &def_str_size);
  }
  list_free(head);
  return str_count;  
}
