#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>

#include "shell.h"
#include "list.h"


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
  i=open("/dev/null", O_RDWR);  /* Stdin*/
  dup(i); /* Stdout*/
  dup(i); /* Stderr*/
}

int flag_check_redir(int flag)
{
  int redir_flags = REDIR_STDIN | REDIR_STDOUTA | REDIR_STDOUTW;
  return (flag & redir_flags) != 0;
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
    if((*status & SLNT) != 0){
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
  if((*status & SLNT) != 0 && !flag_check_redir(*status))
    stdio_to_null();
  execvp(argv[0], argv);
  perror(argv[0]);
  exit(1);
}

/* 0 - correct cd command*/
int check_cd(char *const *argv)
{
  if(strcmp(*argv, "cd") == 0){
    if(argv[1] != NULL && argv[2] == NULL)
      return 0;
    else{ 
      fprintf(stderr, "shsh:cd: wrong number of arguments\n");
      return ERRCD;
    }
  }
  return NOTCD;
}

void exec_cd(char const *argv)
{ 
  int res;
  res = chdir(argv);
  if(res != 0)
    fprintf(stderr, "shsh:cd: '%s' not a directory\n", argv);
}

void buf_write_char(char *str, int *pos, 
                    const char tmp, 
                    const int *def_str_size)
{
  /* If the string length is <n> times <def_str_size>*/
  /* reallocate <n+1>*<def_str_size>*/
  /* Handle only boundary case*/
  if((*pos-1) / *def_str_size != *pos / *def_str_size){
    str = realloc(str, 
        *def_str_size * (*pos / *def_str_size + 1));
  }
  str[*pos] = tmp;
  (*pos)++;
}

void prompt()
{
  long size = pathconf(".", _PC_PATH_MAX);
  char *cwd = malloc(size * sizeof(*cwd));
  getcwd(cwd, size);
  printf("%s $ ", cwd);
  free(cwd);
}

int read_str(char *buf, const int *def_str_size)
{
  int tmp;
  int str_len = 0;  
  int brace_flag = 0;             /* Open br => 1, Cl br => 0*/
  prompt();
  while((tmp = getchar()) != '\n'){
    switch(tmp){
      case EOF:
        return EOF;
      case '"':
        brace_flag = !brace_flag;
        break;
    }
    if(!brace_flag){
      /* Add space before >,>>,<,&*/
      if(strchr("><&", tmp) != NULL && 
        (tmp != '>' || buf[str_len-1] != '>'))
          buf_write_char(buf, &str_len, ' ', def_str_size);
      /* Remove spaces after >,>>,<*/
      if(tmp == ' ' && strchr("<>",buf[str_len-1]) != NULL)
        continue;
    }
    buf_write_char(buf, &str_len, tmp, def_str_size);
  }
  buf[str_len] = '\0';
  if(brace_flag){
    fprintf(stderr, "shsh: odd number of quotation marks\n");
    return 1;
  }
  return 0;
}

char *skip_white(char *str)
{
	while (isspace(*str)) str++;
	return str;
}

void rm_braces(char *str)
{
  if(*str == '"'){
    const int len = strlen(str);
    memmove(str, str+1, len);
    str[len-2] = '\0';
  }  
}

int cmd_error_hdl(const char *file,char *operation, 
                  int *exec_stat,int flag_check)
{
  char *fmt_wrong_cmd = "shsh: parse error near '%s'\n";
  char *fmt_file_expt = "shsh: parse error near '%s': \
                         expected file name\n";
  if((*exec_stat & flag_check) != 0){ /* Multiple special chars*/
    fprintf(stderr, fmt_wrong_cmd, operation);
    return 1;
  }
  if(flag_check_redir(flag_check)) /* Check only redirect options*/
    if(*file == '\0'){
      fprintf(stderr, fmt_file_expt, operation);
      return 1;
    }
  return 0;
}

int cmd_prs_check_set(const char *cmd, char *instruct, int *exec_stat,
                      const int flag_check, const int flag_set)
{
  const int instr_len = strlen(instruct);
  int res;
  if(memcmp(cmd, instruct, instr_len) == 0){
    cmd += instr_len; /* cmd = file name*/
    res = cmd_error_hdl(cmd, instruct, exec_stat, flag_check);
    if(res != 0) 
      return PRSERROR;
    *exec_stat |= flag_set;
    return PRSIGNORE;
  }
  return 0;
}

int redirect(char *file, int srcfd, int mode)
{
  int filefd; 
  filefd = open(file, mode, 777);
  if(filefd == -1){
    perror("shsh:open");
    return 1;
  }
  dup2(filefd, srcfd);
  close(filefd);
  return 0;
}

int exec_custom_cmd(char *cmd, int *exec_stat)
{
  int prs_res, redir_res;
  size_t i;
  char *filename;
  char *operators[] = {"<",">>",">"};
  int  flag_check[] = {
    REDIR_STDIN,
    REDIR_STDOUTA | REDIR_STDOUTW,
    REDIR_STDOUTA | REDIR_STDOUTW};
  int flag_set[] = {REDIR_STDIN,REDIR_STDOUTA,REDIR_STDOUTW};
  int fd[] = {0, 1, 1};
  int fd_mode[] = {
    O_RDONLY | O_CREAT,
    O_APPEND | O_CREAT, 
    O_WRONLY | O_CREAT | O_TRUNC};

  prs_res = cmd_prs_check_set(cmd,"&",exec_stat,SLNT,SLNT);
  if(prs_res != 0) return prs_res;

  for(i = 0; i < 3; i++){
    prs_res = cmd_prs_check_set(cmd, operators[i], exec_stat,
                            flag_check[i], flag_set[i]);
    if(prs_res == PRSERROR) return PRSERROR;
    if(prs_res != 0){
      filename = cmd + strlen(operators[i]);
      rm_braces(filename);
      redir_res = redirect(filename, fd[i], fd_mode[i]);
      if(redir_res != 0) return PRSERROR;
      return PRSIGNORE;
    }
  }
  rm_braces(cmd);
  return 0;
}

int handle_cmd(list **lst, char *cmd, int *len, 
               int *exec_stat, const int *def_str_size)
{
  int prs_res;
  prs_res = exec_custom_cmd(cmd, exec_stat);
  if(prs_res == PRSERROR) return 1;
  if(prs_res != PRSIGNORE){
    strcpy((*lst)->str, cmd);
    list_allocate_next(lst, def_str_size);
    (*len)++;
  } 
  return 0;
}

int split_command(list *lst, char *buf, int *len, 
                  int *exec_stat, const int *def_str_size)
{
  list *head = lst;
  char *next;
  int res;
  buf = skip_white(buf);
  next = strchr(buf, ' ');
  while(next != NULL){
    next[0] = '\0';
    res = handle_cmd(&lst, buf, len, exec_stat, def_str_size);
    if(res != 0) return 1; 
    buf = skip_white(next + 1);
    next = strchr(buf, ' ');
  }	
  if(*buf != '\0'){
    res = handle_cmd(&lst, buf, len, exec_stat, def_str_size);
    if(res != 0) return 1; 
  }
  if(*lst->str == '\0') 
    list_rm_last(head);
  return 0;
}

int stream(const int def_str_size)
{
  int res, str_count = 0;
  int len = 0, exec_stat = NORM;
  int fd[2];
  char *buf = malloc(def_str_size * sizeof(*buf));
  list *head;

  list_init(&head, &def_str_size);
  res = read_str(buf, &def_str_size);
  while(res != EOF){
    /* Save stdin, stdout*/
    fd[0] = dup(0); fd[1] = dup(1);
    res = res || 
      split_command(head, buf, &len, &exec_stat, &def_str_size);
    /* Check empty input and errors while parsing*/
    if(res == 0 && *head->str != '\0'){
      list_exec(head, &len, &exec_stat);
      str_count++; 
    }
    /* Restore stdin, stdout*/
    dup2(fd[0],0); dup2(fd[1],1);
    close(fd[0]); close(fd[1]);
    list_free(head);
    list_init(&head, &def_str_size);
    exec_stat = NORM; len = 0;
    res = read_str(buf, &def_str_size);
  }
  list_free(head);
  free(buf);
  return str_count;  
}
