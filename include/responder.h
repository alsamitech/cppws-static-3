#ifndef ALSWS_RESPONDER_H
#define ALSWS_RESPONDER_H

#include "utils.h"
#include "DString"

typedef struct FRONT_PGS_STRUCT{
  char* pg404;
} front_resp_t;

front_resp_t rf;

unsigned char* front_resp(unsigned char* recvln, char* ip, unsigned short* ret_flags){
  *ret_flags^=AF_FREE;
  puts(recvln);
  fflush(stdout);
  char* xtokdump=strdup(recvln);
  char* meth=strtok_r(xtokdump, " ", &xtokdump);
  char* getnm=strtok_r(xtokdump, " ", &xtokdump);

  /*if(strcmp("/", getnm)==0){
    puts(conf_homepg);
    system("pwd");
    getnm=conf_homepg;
  }*/

  // parse getnm
  long unsigned int getlen;
  char* fresp;
  if(strncmp("/", getnm, 2)==0){
    fresp=read_file(conf_homepg, &getlen);
  } else{
    fresp=read_file(getnm+1, &getlen);
    if(get_by_last(getnm+1, 1)=='/'){
      fresp=getDirList(getnm+1);
      if(fresp!=NULL)
      getlen=strlen(fresp);
    }
  }
  if(fresp==NULL){
pg404Def:

    if(rf.pg404==NULL){
    return (NetStr)strdup("HTTP/1.1 404 NOT FOUND\r\nserver: alsws\r\n\r\n"
    "<style>body{font-family: 'JetBrains Mono','Ubuntu Mono','Ubuntu', Roboto, Verdana, Arial;color:white;background-color:black;}#c404{text-align:center;}</style>"    
    "<html><head><title>404</title></head><body><p id=\"c404\">page not found</p></body></html>");
    }else{
      long unsigned int W404_len;
      char* W404;
      if((W404=read_file(rf.pg404, &W404_len))==NULL){
        rf.pg404=NULL;
        goto pg404Def;
      }else{
        char* retx=(char*)malloc(W404_len+41+1+2);
        sprintf(retx, "HTTP/1.1 200 OK\r\nserver: alsws\r\n\r\n\%s\r\n", W404);
        return retx;
      }
    }
  }
  char* retx=(char*)malloc(getlen+36+1);
  sprintf(retx, "HTTP/1.1 200 OK\r\nserver: alsws\r\n\r\n%s\r\n", fresp);

  free(fresp);

  return (NetStr)retx;
}

#endif