#include <mug.h>

int main(int argc, char** argv)
{
  char *content = argv[1];

  cimg_handle_t canvas = mug_new_canvas();

  mug_draw_number_cimg_handle(canvas, 0, 0, content, CYAN);

  handle_t disp = mug_disp_init();

  mug_disp_cimg_handle(disp, canvas);

  mug_destroy_cimg_handle(canvas);
  return 0;
}
