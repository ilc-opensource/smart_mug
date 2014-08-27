#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

void on_temp(int board_temp, int mug_temp, int battery_temp)
{
  printf("%d, %d, %d\n", board_temp, mug_temp, battery_temp);
}

int main(int argc, char** argv)
{
  temp_data_t data;
  handle_t handle;

  handle = mug_temp_init();

  if(!handle) {
    printf("can not initialize temprature sensor\n");
    exit(1);
  }

#ifdef USE_LIBUV
  mug_temp_on(handle, on_temp, 1000);
  mug_run_temp_watcher(handle);
#else
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
#endif
  return 0;
}
