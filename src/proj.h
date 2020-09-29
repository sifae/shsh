#define LIST struct list

struct list{
 char *str;
 struct list *next;
};

/* Read strings from stdin and write it to stdout*/
/* Returns number of entered strings*/
int read_str(const int init_str_size);

/* Write <n> strings from buffer to stdout
 * If actual buffer size is less than <n> 
 * only available strings will be processed
*/
void print_str(struct list *buf, int n);
