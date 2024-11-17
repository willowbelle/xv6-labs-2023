#include "kernel/types.h"
#include "user.h"



int main(){
  //unsigned int i = 0x00646c72;
  //printf("H%x Wo%s", 57616, (char *)&i);

  //printf("x = %d, y = %d",3);
  int dummy = 5; // 添加一个栈变量
  printf("Stack address: %p\n", (void *)&dummy);
  //printf("x=%d y=%d\n", 3);
  return 0;
}