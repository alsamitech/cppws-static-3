#include "include/utils.h"

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