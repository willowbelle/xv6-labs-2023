#include "../kernel/types.h"
#include "user.h"

#define BSIZE 1

int main(){
  int fp[2]; // used to p->c
  int cp[2]; // used to c->p
  pipe(fp);
  pipe(cp);
  char *buf [BSIZE];
  int pid = fork();
  if(pid < 0){
    fprintf(2,"fork failed\n");
    close(fp[0]);
    close(fp[1]);
    close(cp[0]);
    close(cp[1]);
    exit(1);
  }else if(pid == 0){
    close(cp[0]);
    close(fp[1]);
    // 可以加上read，write的错误处理机制
    read(fp[0],buf,1);
    printf("%d: received ping\n",getpid());
    write(cp[1],buf,1);
  }else{
    close(fp[0]);
    close(cp[1]);
    write(fp[1],buf,1);
    read(cp[0],buf,1);
    printf("%d: received pong\n",getpid());
  }
  exit(0);
}