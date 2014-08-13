#include <mug.h>

#include <list>
using namespace std;

#define cimg_display 0
#include <CImg.h>
using namespace cimg_library;

#define LATCH 128

unsigned char RGB_2_raw(unsigned char R, unsigned char G, unsigned B)
{
  unsigned char raw = 0;

  if(R > LATCH)
    raw |= 1;
  
  if(G > LATCH)
    raw |= 2;

  if(B > LATCH)
    raw |= 4;

  return raw; 
}

int mug_read_cimg(void* cimg, char *buf)
{
  CImg<unsigned char> src = *(CImg<unsigned char>*)cimg;

  unsigned char *p =(unsigned char*) buf;
  int width = src.width();
  int height = src.height();
 
  if(!(MAX_ROWS == height && MAX_COLS == width)) {
    printf("ERROR, height: %d, width %d\n", height, width);
    return IMG_ERROR;
  }

  unsigned char R, G, B, raw, offset;
  for(int r = 0; r < height; r++) {
    for(int c = 0; c < width; c++) {
      R = (unsigned char)src(c, r, 0, 0);
      G = (unsigned char)src(c, r, 0, 1);
      B = (unsigned char)src(c, r, 0, 2);

      raw = RGB_2_raw(R, G, B);
     
      if( c % 2) {
        (*p) &= 0xf; 
        (*p) |= raw << 4;
        p++;
      } else {
        (*p) &= 0xf0;
        (*p) |= raw;
      } 
    }
  }

  return IMG_OK;
}

int mug_read_img(char *fname, char *buf) 
{
  CImg<unsigned char> src(fname);
  return mug_read_cimg(&src, buf);
}

char* mug_create_raw_buffer() 
{
  char *ret;
  ret = (char*)malloc(COMPRESSED_SIZE);
  return ret;
}

void mug_free_raw_buffer(char *buf) 
{
  free(buf);
};

int mug_disp_img(handle_t handle, char* name) 
{
  int ret;
  char *buf;

  buf = mug_create_raw_buffer();
  if(!buf)
    return IMG_ERROR;

  ret = mug_read_img(name, buf);
  if(ret != IMG_OK)
    return ret;

  mug_disp_raw(handle, buf);

  mug_free_raw_buffer(buf);
 
  return IMG_OK;
}

int mug_disp_cimg(handle_t handle, void *cimg) 
{
  int ret;
  char *buf;

  buf = mug_create_raw_buffer();
  if(!buf)
    return IMG_ERROR;

  ret = mug_read_cimg(cimg, buf);
  if(ret != IMG_OK)
    return ret;

  mug_disp_raw(handle, buf);

  mug_free_raw_buffer(buf);

  return IMG_OK;
}

char* mug_read_img_N(char* names, int *num)
{
  char *p = (char*)malloc(strlen(names) + 1);
  strcpy(p, names);

  list<char*> parsed;

  *num = 0;
  char *head = p;
  while(*p != '\0') {
    if(*p == ',' || *p == ';') {
      (*p) = '\0';
      parsed.push_back(head);
      head = p + 1;
    }
    p++;
  };

  if(p != head) {
    parsed.push_back(head);
  }

  (*num) = parsed.size();
  char *raw = (char*)malloc(COMPRESSED_SIZE * parsed.size());
  p = raw;
  int err;
 
  for(list<char*>::iterator itr = parsed.begin();
      itr != parsed.end();
      itr++) {
    printf("+%s\n", *itr);
    err = mug_read_img(*itr, p);
    if(err != IMG_OK) {
      printf("read image %s error\n", *itr);
      return 0;
    }
 
    p += COMPRESSED_SIZE;
  }

  return raw;
}

int mug_disp_img_N(handle_t handle, char *names, int interval)
{
  int num;
  char *raw = mug_read_img_N(names, &num);

  mug_disp_raw_N(handle, raw, num, interval); 
}

