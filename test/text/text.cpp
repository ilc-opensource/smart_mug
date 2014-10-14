#include <mug.h>

int main(int argc, char** argv)
{

  if(argc != 2) {
    printf("no content\n");
    return 1;
  }
    
  char *content = argv[1];


  //mug_draw_number_cimg(canvas, 0, 0, content, CYAN);
  handle_t disp = mug_disp_init();
  mug_init_font(NULL);
  //mug_disp_text_marquee(disp, "刘章林abcABC1234567890", cyan, 100, -1);
  mug_disp_text_marquee(disp, content, cyan, 100, -1);

  //mug_save_cimg(canvas, "save.bmp");
  //mug_disp_cimg_marquee(disp, canvas, 100, -1);
  //mug_disp_cimg_handle(disp, canvas);
  //mug_destroy_cimg(canvas);
  return 0;
}
