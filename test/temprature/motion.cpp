#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

void dump(motion_data_t *data)
{
  if(data == NULL)
    printf("error can not read data\n");
  else
    printf("async ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
          data->ax, data->ay, data->az, data->gx, data->gy, data->gz);
 
}

int main(int argc, char** argv)
{
  motion_data_t data;
  handle_t handle;

  handle = mug_motion_sensor_init();

  if(!handle) {
    printf("can not initialize sensor\n");
    exit(1);
  }

  printf("handle: 0%x\n", handle);
  
  while(1) {
    mug_read_motion_sensor(handle, &data);
    mug_read_motion_sensor_async(handle, dump);
    printf("      ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
           data.ax, data.ay, data.az, data.gx, data.gy, data.gz);
    usleep(500*1000);
  }

  return 0;
}
