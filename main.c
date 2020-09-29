#include <stdio.h>
#include <stdlib.h>
#include "src/proj.h"

int main(int argc, char *argv[])
{
  if(argc > 2) {
    fprintf(stderr, "Too many arguments"); 
    return EXIT_FAILURE;
  }
  const int def_str_size = 32;
  int init_str_size = (argc==1) ? def_str_size : atoi(argv[1]);
  if(init_str_size > 10000){
    fprintf(stderr, "Init size too large");
    return EXIT_FAILURE;
  }
  init_str_size = (init_str_size <= 0) ? def_str_size : init_str_size; 
  printf("Strings read: %d\n",read_str(init_str_size));
  return 0;
}
