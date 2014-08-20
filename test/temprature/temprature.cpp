#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>


int main(int argc, char** argv)
{
  temp_data_t data;
  handle_t handle;

  handle = mug_temp_init();

  if(!handle) {
    printf("can not initialize temprature sensor\n");
    exit(1);
  }

  printf("handle: 0%x\n", handle);
  
  while(1) {
#if 0
    mug_read_temp(handle, &data);
    printf("board: %4d, mug: %4d, battery: %4d\n", data.board_temp, data.mug_temp, data.battery_temp);
#else
   int temp = mug_read_board_temp(handle);
   printf("board temp: %d\n", temp);
#endif
    usleep(500*1000);
  }

  return 0;
}
