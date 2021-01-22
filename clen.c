#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
  if(argc<2){
    fprintf(stderr, "Usage: %s \"<phrase>\"\n", argv[0]);
    exit(1);
  }
  printf("%zu\n", strlen(argv[1]));

  return 0;
}