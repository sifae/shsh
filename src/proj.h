#define PRSERR 0  /* Error parsing command*/

#define NORM 0  /* Execute in standart mode */
#define SLNT 1  /* Execute in background    */
#define REDIR_STDIN   2  /* Stdin  redirection (<)   */
#define REDIR_STDOUTW 4  /* Stdout redirection (>)   */
#define REDIR_STDOUTA 8  /* Stdout redirection (>>)  */

typedef struct list list; 

struct list {
  char *str;
  struct list *next;
};

/* Read strings from stdin and write it to stdout*/
/* Returns number of executed commands*/
int stream(const int def_str_size);

/* Write <n> strings from buffer to stdout
 * If actual buffer size is less than <n> 
 * only available strings will be processed
*/
void print_str(list *buf, int n);
