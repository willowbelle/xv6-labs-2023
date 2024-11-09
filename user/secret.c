#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

/// @brief 通过syscall sbrk增加进程的内存大小，并将secret string写入内存的相应位置
///        内部陷入操作系统后通过sys_sbrk->growproc->uvmalloc->memset
/// @param argc 
/// @param argv 
/// @return 
int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf("Usage: secret the-secret\n");
    exit(1);
  }
  char *end = sbrk(PGSIZE*32);
  end = end + 9 * PGSIZE;
  strcpy(end, "my very very very secret pw is:   ");
  strcpy(end+32, argv[1]);
  exit(0);
}

