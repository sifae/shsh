#define LIST struct list
#define ODDBR 0

struct list{
  char *str;
  struct list *next;
};

/* Read strings from stdin and write it to stdout*/
/* Returns number of entered strings*/
int stream(const int def_str_size);

/* Write <n> strings from buffer to stdout
 * If actual buffer size is less than <n> 
 * only available strings will be processed
*/
void print_str(LIST *buf, int n);
