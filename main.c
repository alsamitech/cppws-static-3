#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/utils.h"
#include "include/DString"

// network typedefs
typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr SA;

typedef unsigned char* NetStr;

#define KV_STRICT 0x4
#define KV_WARN 0x02

// if flag not set, ipv4 will be implied
#define WS_IPv6 0x200

#define WSCONF_PRINTALL 0x02
#define WSCONF_SUPRESS 0x400
#define AF_FREE 0x200

char* cnf_file;
long unsigned int cnf_file_len;

typedef char* cstr;

  typedef struct {
    uint16_t portno;
    int backlog;
    unsigned int maxline_in;
    uint16_t flags;
  }config_t;

  inline void fdump_conf(FILE* fp, config_t* cnf){
    fprintf(fp, "port: %u, maxline: %u", cnf->portno, cnf->maxline_in);
  }


  typedef struct{
    char* key;
    char* val;
  }keyval_t;
  keyval_t* keyval_alloc(long unsigned int keypolicy, long unsigned int valpolicy){
    keyval_t* keyval_i=(keyval_t*)malloc(sizeof(keyval_t));
    memset(keyval_i, 0, sizeof(keyval_t));

    keyval_i->key=(char*)malloc(sizeof(char)*keypolicy);

    keyval_i->val=(char*)malloc(sizeof(char)*valpolicy);

    return keyval_i;
  }
  /*inline*/ keyval_t* keyval_nalloc(){
    keyval_t* keyval_i=(keyval_t*)malloc(sizeof(keyval_t));
    memset(keyval_i, 0, sizeof(keyval_t));

    return keyval_i;
  }
  keyval_t* keyval_make(char* input){
      keyval_t* keyval_i=keyval_nalloc();
      {
       // printf("%s\n", input); 
        //char* tokdump=strdup(input);
        char* tokdump=input;
        keyval_i->key=strtok_r(tokdump, ":", &tokdump);
        keyval_i->val=strtok_r(tokdump, ":", &tokdump);
      }
      return keyval_i;
  }
  inline void keyval_mkd(keyval_t* keyval){
    free(keyval->key);
    free(keyval);
  }
  inline void keyval_destroy(keyval_t* keyval){
    free(keyval->key);
    free(keyval->val);
    free(keyval);
  }


/*namespace ws{*/
  typedef unsigned char* (*ws_resp) (unsigned char*, char*, unsigned short*);
  typedef struct{
    int listenfd, connfd, n;
    unsigned char* recvln;
    config_t info;
    FILE* webpage;
    struct sockaddr_in servaddr;
    ws_resp resp;
  }ws_system_t;
  ws_system_t* server_make(config_t* conf, ws_resp resp){
    ws_system_t* ws_system_i=(ws_system_t*)calloc(1, sizeof(ws_system_t));
    ws_system_i->resp=resp;
    ws_system_i->info=*conf;

    return ws_system_i;

  }
  unsigned char init_socket(ws_system_t* self){
    if((self->listenfd=socket(/*ipv4*/AF_INET, SOCK_STREAM, 0))<0){
      if(self->info.flags&WSCONF_SUPRESS){
        return 1;
      }else{
        fprintf(stderr, "\033[1;31msocket creation failed! | socket()\033[0m\n");
        
       return 1;
      }
    }
    bzero(&self->servaddr, sizeof(sockaddr_in_t));

    self->servaddr.sin_family=AF_INET;
    self->servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    self->servaddr.sin_port=htons(self->info.portno);

    return 0;

  }
  unsigned char listenAndBind(ws_system_t* self){
    if((bind(self->listenfd, (SA*)&self->servaddr, sizeof(sockaddr_in_t)))<0){
      if(self->info.flags&WSCONF_SUPRESS){
        return 2;
      } else{
        fprintf(stderr, "\033[1;31mbinding failed! | bind()\033[0m\n");
        return 2;
      }
    }
    if((listen(self->listenfd, self->info.backlog))<0){
      if(self->info.flags&WSCONF_SUPRESS){
        return 3;
      } else{
        fprintf(stderr, "\033[1;31mlistening failed! | listen()\033[0m\n");
        return 3;
      }
    }
    return 0;
  }
  unsigned char mainThreadLoop(ws_system_t* self){
    self->recvln=(unsigned char*)malloc(self->info.maxline_in);
    for(;;){
      sockaddr_in_t addr;
      socklen_t addr_len;
      if(self->info.flags&WSCONF_PRINTALL){
        printf("\033[1;32mWaiting for a connection on port %d\033[0m\n", self->info.portno);
      }

      self->connfd=accept(self->listenfd, (SA*)&addr, &addr_len);

      char clientaddr[128+1];
      inet_ntop(AF_INET, &addr, clientaddr, 128);
      if(self->info.flags&WSCONF_PRINTALL){
        printf("\033[1;32mAccepted Connection\033[0m\n");        
      }
      memset(self->recvln, 0, self->info.maxline_in);
      if((self->n=read(self->connfd, self->recvln, self->info.maxline_in))==-1){
        fprintf(stderr, "\033[1;31mError on read.\033[0m\n");
      }
      
      unsigned short respf=0x0;

      unsigned char* resp=self->resp(self->recvln, clientaddr, &respf);

      if(write(self->connfd,(char*)resp, strlen(resp))==-1){
        fprintf(stderr, "Unable to write to socket!\n");

      }
      if(respf&AF_FREE){
        free(resp);
      }
      close(self->connfd);



    }
    return 0;
  }
  /*namespace v6{
  typedef struct{
    int listenfd, connfd, n;
    unsigned char* recvln;
    config_t info;
    FILE* webpage;
    struct sockaddr_in6 servaddr;
    ws_resp resp;
  }ws_system_t;
  }*/



/*}*/

char* conf_homepg;

#include "include/responder.h"

int main(int argc, char** argv){
  char* cnf_file=read_file(".wsconf", &cnf_file_len);
  if(cnf_file==NULL){
    fprintf(stderr, "\033[1;31mWeb server config file does not exist! (.wsconf==NULL)\033[0m\n");
    exit(1);
  }
  long unsigned int cnf_toks_l;
  char** cnf_toks;
  config_t config;
  config.flags=0x0;
  config.backlog=10;
  {
    cnf_toks=atokl(cnf_file, "\n", &cnf_toks_l);
    
  }

  unsigned char conf_syn=0x0;

  for(int i=0; i<(cnf_toks_l-1); i++){ 
    // This loops through the .wsconf file to find relevant key-value data pairs
    keyval_t* conf_kv=keyval_make(cnf_toks[i]);

    //printf("%d\n", i);
    if(conf_kv->key[0]=='-'&&conf_kv->key[1]=='#'){
      //printf("Comment!\n");
      goto C_FST;
    }else if(conf_kv->key[0]=='p'&&conf_kv->key[1]=='#'){
      puts(conf_kv->val);
      goto C_FST;
    }
    if(strlen(conf_kv->key)<4){
      fprintf(stderr,"\033[1;31m(!) Error on parsing config file. Length of key is too short\033[0m\n");
      goto C_FST;
    }

    if(conf_kv->key[0]=='p'&&conf_kv->key[1]=='o'&&conf_kv->key[2]=='r'&&conf_kv->key[3]=='t'){
      sscanf(conf_kv->val, "%hu",&config.portno);

    }else if(strcmp(conf_kv->key, "syntax")==0){
        //printf("ayy");
        if(strcmp(conf_kv->val,"strict")==0){
          //printf("set config to strict\n");
          conf_syn^=KV_STRICT;
        }else if(strcmp(conf_kv->val, "warn")==0){
          conf_syn^=KV_WARN;
        }
    }else if(strcmp(conf_kv->key, "maxline_in")==0){
      sscanf(conf_kv->val,"%ui",&config.maxline_in);
    }else if(strcmp(conf_kv->key, "LogStyle")==0){
      if(strcmp(conf_kv->val, "PrintAll")==0){
        config.flags^=WSCONF_PRINTALL;
      }else if(strcmp(conf_kv->val, "Supress")==0){
        config.flags^=WSCONF_SUPRESS;
      }
    }else if(strcmp(conf_kv->key, "server-backlog")==0){
      sscanf(conf_kv->val, "%d", &config.backlog);
    }else if(strcmp(conf_kv->key, "homepg")==0){
      conf_homepg=conf_kv->val;
    } else if(strcmp(conf_kv->key, "throw(404)")==0){
      printf("\033[0;32mConfig :%s\033[0m\n", conf_kv->val);
      rf.pg404=conf_kv->val;
      
    }
C_FST: 
    //free(cnf_file);
    continue;
    //keyval_mkd(conf_kv);
    }
// Configuration Final stage. essentially continue but with a bit of memory being freed.

  // Now we can start with the buisness of making the server
  ws_system_t* serv = server_make(&config, front_resp); 
  {
    init_socket(serv);
    listenAndBind(serv);
    mainThreadLoop(serv);

  }
}