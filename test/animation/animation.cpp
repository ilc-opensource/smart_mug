#include <stdio.h>
#include <stdlib.h>
#include <mug_cpp.h>

#define IMG "./test.bmp"
int main(int argc, char** argv)
{

  if(argc < 3) {
    printf("please input image list separate with , and image number: \n");
    return 0;
  }
#if 1
  handle_t handle = mug_disp_init();
  mug_disp_img_N(handle, argv[1], atoi(argv[2]));
  mug_close(handle);
#else
  int img_num = argc - 1;
  
  char *raw, *p;

  raw = (char*)malloc(COMPRESSED_SIZE * img_num);
  p = raw;
  
  for(int i = 0; i < img_num; i++) { 
    char* name = argv[i + 1];
    printf("+ %s\n", name);
    mug_read_img(name, p);
    p += COMPRESSED_SIZE;
  }

  handle_t handle = mug_init();

  mug_disp_raw_N(handle, raw, img_num, 200);

  mug_close(handle); 
#endif

  return 0;
}
