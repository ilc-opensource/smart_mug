#include <stdio.h>
#include <stdlib.h>
#include <mug_cpp.h>

#define IMG "./test.bmp"
int main(int argc, char** argv)
{

  handle_t handle = mug_disp_init();
  mug_stop_mcu_disp(handle);
  printf("stopped mcu flush\n");
  mug_close(handle);
  return 0;
}
