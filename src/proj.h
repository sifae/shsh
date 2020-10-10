#define ODDBR -2 /* 'Exception' status*/
#define SLNT 1  /*  Execution  status*/
#define NORM 0  /*  Execution  status*/

#define RED   "\x1B[31m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

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
