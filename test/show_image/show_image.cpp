#include <stdio.h>
#include <mug.h>

#define IMG "./test.bmp"
int main(int argc, char** argv)
{

  if(argc == 1) {
    printf("please input image: \n");
    return 0;
  }

  handle_t handle = mug_init();
  
  mug_disp_img(handle, argv[1]); 

  mug_close(handle); 

  return 0;
}
