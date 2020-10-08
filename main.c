#include <stdio.h>
#include <stdlib.h>
#include "src/proj.h"

int main(int argc, char *argv[])
{
  const int def_str_size = 32;
  int init_str_size = (argc==1) ? def_str_size : atoi(argv[1]);
  if(argc > 2) {
    fprintf(stderr, "Too many arguments"); 
    return 1;
  }
  if(init_str_size <= 0){
    fprintf(stderr, "Init size should be positive");
    return 1;
  }
  printf("\nCommands executed: %d\n", stream(init_str_size));
  return 0;
}
