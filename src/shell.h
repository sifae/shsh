#ifndef SHELL_H
#define SHELL_H 

#include "list.h"

#define PRSIGNORE 1
#define PRSERROR -1

#define ERRCD -1
#define NOTCD 1

#define NORM 0  /* Execute in standart mode */
#define SLNT 1  /* Execute in background    */
#define REDIR_STDIN   2  /* Stdin  redirection (<)   */
#define REDIR_STDOUTW 4  /* Stdout redirection (>)   */
#define REDIR_STDOUTA 8  /* Stdout redirection (>>)  */

/* Read strings from stdin and write it to stdout*/
/* Returns number of executed commands*/
int stream(const int def_str_size);

void form_exec_argv(char **dest, const list *src, const int *len);
void exec_cd(char const *argv);
void exec_command(char *const *argv, const int *status);
int check_cd(char *const *argv);

#endif /* SHELL_H*/
