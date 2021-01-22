#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* read_file(char* filenm, long unsigned int* len){
  char* buf=0;
  FILE* f=fopen(filenm, "rb");
  if(f){
    fseek(f,0,SEEK_END);
    *len=ftell(f);
    fseek(f,0,SEEK_SET);
    buf=(char*)calloc(1, *len);
    if(buf)
      fread(buf, 1, *len, f);
    fclose(f);
  }
  return buf;
}

char** tok2(char* InC, char* delim){
  char** tok=(char**)malloc(2*sizeof(char*));
  //printf("%p\n", tok);

  tok[0]=strtok(strdup(InC), delim);
  {
  int i=1;
  while(tok[i-1]!=NULL){
       tok=(char**)realloc(tok, (i+1)*sizeof(char*));
       tok[i]=strtok(NULL, delim);
       printf("%p\n", tok[i]);
       i++;
  
  }
  /*EBRACE*/}
   
  return tok;
}

int main(int argc, char** argv){
  long unsigned int cn_len;
  char* cn_c=read_file(".china", &cn_len);
  //puts(cn_c);
  printf("%zu %zu", sizeof(long unsigned int), sizeof(long unsigned int*));
  char** my_nice_lines=tok2(cn_c, "\n");
  printf("\033[1;32mHello, World!\033[0m\n%s\n%s\n%s\n%s\n%s\n", my_nice_lines[0], my_nice_lines[1], my_nice_lines[2], my_nice_lines[3], my_nice_lines[4]);
  return 0;
}