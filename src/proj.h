#define LIST struct list

enum {
  DEF_STR_SIZE = 32
};

struct list{
 char *str;
 struct list *next;
};

//Read strings from stdin and write it to stdout
void read_str();

//Write <n> strings from buffer to stdout
//If actual buffer size is less than <n> 
//only available strings will be processed
void print_str(struct list *buf, int n);
