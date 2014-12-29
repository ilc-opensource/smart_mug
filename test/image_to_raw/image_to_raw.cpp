#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mug.h>
#define _print(...) fprintf(fp, __VA_ARGS__)

//#define DISP

int main(int argc, char** argv)
{

  if(argc < 4) {
    printf("image_to_raw input_file_list duration output\n");
    return 0;
  }

#ifdef DISP
  handle_t handle = mug_disp_init();
#endif

  char *file_list = argv[1];
  int duration = atoi(argv[2]);
  char *output = argv[3];

  char *buf, *p;
  int   num, size;

  FILE *fp; 
  fp = fopen(output, "w+");

  buf = mug_read_img_N(file_list, &num, &size);
  p = buf;

  printf("translated %d images and create %d size of memory\n", num, size);

  _print("#ifndef __INIT_ANIMATION_H__\n");
  _print("#define __INIT_ANIMATION_H__\n\n");
  _print("/*********************************** \n");
  _print("  files   : %s\n", file_list);
  _print("  duration: %d\n", duration);
  _print("  num     : %d\n", num);
  _print("  size    : %d bytes\n", size);
  _print("************************************/\n\n");

  _print("struct LedFrame ledFrames[MAX_LED_FRAMES] = {\n");

  for(int i = 0; i < num; i++) {
    usleep(duration * 1000);
#ifdef DISP
    mug_disp_raw(handle, p);
#endif
    
    _print("  { // %d\n", i);
    _print("    %d, \n", duration);
    _print("    {\n");
    for(int r = 0; r < MAX_COMPRESSED_ROWS; r++) {
      _print("      { "); 
      for(int c = 0; c < MAX_COMPRESSED_COLS; c++) {
        _print("0x%02x", *(unsigned char*)p++);
        
        if(c == MAX_COMPRESSED_COLS - 1) 
          _print(" }");

        if(r == MAX_COMPRESSED_ROWS -1 && c == MAX_COMPRESSED_COLS -1) {
        } else {
          _print(", ");
        }
      }
      _print("\n");
    } 
    _print("    }\n");
   
    _print("  }");
    if(i != num -1)
      _print(",");
    
    _print("\n");
  }

  _print("};\n\n");

  _print("struct LedAnim ledAnimation = {\n");
  _print("  MAX_LED_FRAMES-1, \n");
  _print("  0,\n");
  _print("  FRAME_ACTIVE_FLAG,\n");
  
  _print("  { ");

  for(int i = 0; i < num / 8; i++) {
    _print("0xff");
    if(i != (num / 8) - 1)
      _print(", ");
  }

  int res = num % 8;

  if(res) {
    _print(", 0x%02x ", (1 << res) - 1);
  }
  _print("}\n};\n");
  _print("\n#endif\n");

  printf("generated: %s\n", output);
  return 0;
}
