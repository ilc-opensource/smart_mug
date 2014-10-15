#include <unistd.h>
#include <mug.h>
#include <string>
#include <vector>

using namespace std;

#define DELAY (3*1000*1000)

int main(int argc, char** argv)
{

  if(argc != 2) {
    printf("no content\n");
    return 1;
  }

  vector<string> colors;
  vector<cimg_handle_t> imgs;

  colors.push_back("red");
  colors.push_back("cyan");
  colors.push_back("green");
  colors.push_back("yellow");
  colors.push_back("magenta");
  
  char *content = argv[1];

  mug_init_font(NULL);

  imgs.push_back(mug_new_text_cimg(content, "red"));
  imgs.push_back(mug_new_text_cimg(content, "blue"));  

  handle_t disp = mug_disp_init();
  mug_init_font(NULL);
 
  int cnt = 0;
#if 1 
  int size = colors.size();

  while(true) {
    for(int i = 0; i < size; i++) {
      mug_disp_text_marquee_async(disp, content, colors[i].c_str(), 100, -1);
      usleep(DELAY);
      printf("-----> %d\n", i);
    }
    mug_disp_text_marquee(disp, content, colors[0].c_str(), 100, -1);

  }
#else
/*
  while(true) {
    printf("%d\n", cnt++);
    mug_disp_cimg_marquee(disp, imgs[0], 100, 1);
    usleep(DELAY / 30);
  }
*/
  while(true) {
    printf("==========\n");
    for(int i = 0; i < imgs.size(); i++) {
      printf("%d\n", cnt++);
      printf("img: %x\n", imgs[i]);
      mug_disp_cimg_marquee_async(disp, imgs[i], 100, -1);
      //usleep(DELAY);
    }
    printf("==========\n");
    for(int i = 0; i < imgs.size(); i++) {
      printf("%d\n", cnt++);
      printf("img: %x\n", imgs[i]);
      mug_disp_cimg_marquee_async(disp, imgs[i], 100, -1);
      usleep(DELAY);
    }
  }
#endif
  return 0;
}
