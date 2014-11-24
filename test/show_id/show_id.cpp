#include <unistd.h>
#include <mug.h>
#include <string>
#include <fstream>

using namespace std;

#define  DEV_FILE "/etc/device_id"

int main(int argc, char** argv)
{

  mug_init_font(NULL);

  handle_t disp = mug_disp_init();
  mug_init_font(NULL);

  ifstream input(DEV_FILE, std::ifstream::in);


  if(!input) {
    string str("No ");
    str += DEV_FILE;
    mug_disp_text_marquee(disp, str.c_str(), "red", 100, -1);  
    return 1;
  }
  
  string contents((std::istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
  input.close();

  mug_disp_text_marquee(disp, contents.c_str(), "cyan", 200, -1);

  return 0;
}
