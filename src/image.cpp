#include <mug.h>

#include <list>
#include <vector>
#include <string>

using namespace std;

#define cimg_display 0
#include <CImg.h>
using namespace cimg_library;

#include "ft2build.h"
#include FT_FREETYPE_H
FT_Library ftlib;
FT_Face face;

#define DEFAULT_FONT "simhei.ttf"

unsigned char red[]    = {255, 0,   0  };
unsigned char green[]  = {0,   255, 0  };
unsigned char blue[]   = {0,   0,   255};
unsigned char yellow[] = {255, 255, 0  };
unsigned char cyan[]   = {0,   255, 255};
unsigned char magenta[]= {255, 0,   255};
unsigned char white[]  = {255, 255, 255};
unsigned char black[]  = {0,   0,   0  };

#define LATCH 128

#define MAX_FILE_NAME 512

char *get_proc_dir() {
  char *buf = (char*) malloc(sizeof(char) * 512);

  readlink("/proc/self/exe", buf, MAX_FILE_NAME);

  char *p = buf + strlen(buf);
  while(*p != '/' && p != buf) {
    p--;
  }

  *p = '\0';

  return buf;
}

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

unsigned char color_2_raw(unsigned char *color) {
  return RGB_2_raw(color[0], color[1], color[2]);
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

void mug_set_pixel_raw_color(char *raw, int col, int row, unsigned char color) {
  int pack_col = col / 2;
  int col_offset = col % 2;
  
  *(unsigned char*)(raw + row * MAX_COMPRESSED_COLS + pack_col) |= (color & 0xf) << (4 * col_offset);
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

char* mug_cimg_to_raw(void *cimg)
{
  
}
char* mug_read_img_N(char* names, int *num, int *size)
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
  (*size) = COMPRESSED_SIZE * parsed.size();

  char *raw = (char*)malloc(*size);
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
  int num, size;
  char *raw = mug_read_img_N(names, &num, &size);

  mug_disp_raw_N(handle, raw, num, interval); 
}

void normalize_color(CImg<unsigned char> &img)
{
  for(int r = 0; r < img.height(); r++) {
    for(int c = 0; c < img.width(); c++) {
      for(int color = 0; color < 3; color++) {
        if(img(c, r, 0, color) < 128) {
          img(c, r, 0, color) = 0;
        } else {
          img(c, r, 0, color) = 255;
        }
      }
    }
  }
}

#define NUMBER_PIC_DIR "number_pic"
vector< CImg<unsigned char> > numbers;

void init_number_text(const char *path)
{
  if(path == NULL) {
    path = get_proc_dir();
  }

  char temp[256];
  
  for(int i = 0; i < 10; i++) {
    sprintf(temp, "%s/%s/%d.bmp", path, NUMBER_PIC_DIR, i);
    CImg<unsigned char> img(temp);
    normalize_color(img);
    numbers.push_back(img);
  }
}

void change_color(CImg<unsigned char> &img, unsigned char *color)
{
  for(int r = 0; r < img.height(); r++) {
    for(int c = 0; c < img.width(); c++) {
      if(img(c, r, 0, 0) == img(c, r, 0, 1)
         && img(c, r, 0, 0) == img(c, r, 0, 2)
         && img(c, r, 0, 0) > 0) {
        img(c, r, 0, 0) = color[0];
        img(c, r, 0, 1) = color[1];
        img(c, r, 0, 2) = color[2];
      }
   }
  }
}

void draw_number(CImg<unsigned char> *pimg, int c, int r, char *str, unsigned char *color)
{
  char *p = str;
  int next_c = c;

  CImg<unsigned char> img;

  while('0' <= *p && *p <= '9') {
    img = numbers[*p - '0'];
    change_color(img, color);
    pimg->draw_image(next_c, r, 0, 0,
                      img);
    next_c += img.width();
    next_c++;
    p++;
  }
}

void resize(CImg<unsigned char> &img, int new_col, int new_row)
{
  img.resize(new_col, new_row, -100);

  printf("img -> %d x %d\n", img.width(), img.height());

}

void mug_draw_number_cimg(void *img, int col, int row, char *str, unsigned char* color)
{
  if(numbers.empty()) {
    init_number_text("./");
  }

  draw_number((CImg<unsigned char> *)img, col, row, str, color);
}

void mug_number_text_shape(int *width, int *height)
{
  if(numbers.empty()) {
    init_number_text("./");
  }

  CImg<unsigned char> img;
  img = numbers[0];

  *width = img.width() + 1;
  *height = img.height();
}

unsigned char *get_color_data(mug_color_t color) {
  unsigned char *val;

  switch(color) {
  case RED:
    val = red;
    break;

  case GREEN:
    val = green;
    break;

  case BLUE:
    val = blue;
    break;

  case YELLOW:
    val = yellow;
    break;

  case CYAN:
    val = cyan;
    break;

  case MAGENTA:
    val = magenta;
    break;

  case WHITE:
    val = white;
    break;

  case BLACK:
    val = black;
    break;

  default:
    val = NULL;
  }

  return val;
}

cimg_handle_t mug_new_cimg_handle(int width, int height)
{
  CImg<unsigned char> *cimg = new CImg<unsigned char>(width, height, 1, 3, 0);
  return (cimg_handle_t)cimg;
}

cimg_handle_t mug_new_canvas()
{
  return mug_new_cimg_handle(SCREEN_WIDTH, SCREEN_HEIGHT);
}

cimg_handle_t mug_load_cimg_handle(char* fname)
{
  CImg<unsigned char> *cimg = new CImg<unsigned char>(fname);
  return (cimg_handle_t)cimg;
}

int mug_disp_cimg_handle(handle_t handle, cimg_handle_t cimg)
{
  return mug_disp_cimg(handle,(void*)cimg);
}

void mug_draw_number_cimg_handle(cimg_handle_t canvas, int col, int row, int num, mug_color_t color)
{
  char temp[64];
  memset(temp, 0, sizeof(temp));
  sprintf(temp, "%d", num);
  return mug_draw_number_cimg((void*)canvas, col, row, temp, get_color_data(color));
}

void mug_draw_cimg_handle(cimg_handle_t c, int col, int row, cimg_handle_t img)
{
  CImg<unsigned char> *canvas = (CImg<unsigned char>*)c;
  CImg<unsigned char> *cimg = (CImg<unsigned char>*)img;

  canvas->draw_image(col, row, 0, 0, *cimg);
}

void mug_destroy_cimg_handle(cimg_handle_t hdl)
{
  delete((CImg<unsigned char>*)hdl);
}

char* mug_cimg_handle_to_raw(cimg_handle_t cimg)
{
  char *raw = mug_create_raw_buffer();
  mug_read_cimg((void*)cimg, raw);
  return raw;
}

/*
 libtruetype for cimg from https://github.com/tttzof351/cimg-and-freetype

 */
void initFreetype(
  FT_Library& ftlib,
  FT_Face& face,
  const std::string& fontFullName
){
  FT_Error fterr;

  if((fterr = FT_Init_FreeType( &ftlib ))) {
    throw "Error init freetype";
  }

  if((fterr = FT_New_Face(ftlib, fontFullName.c_str(), 0, &face))){
    if(fterr == FT_Err_Unknown_File_Format) {
      throw "Error feetype, Unsupported font";
    } else {
      throw "Error feetype, new face";
    }
  }
}

void closeFreetype(
  FT_Library& ftlib,
  FT_Face& face
){
  FT_Done_Face(face);
  FT_Done_FreeType(ftlib);
}

void drawGlyph(
  FT_GlyphSlot& glyphSlot,
  CImg<unsigned char>& image,
  const int& shiftX,
  const int& shiftY,
  unsigned char fontColor[] = NULL
){
  unsigned char buff[] = {255, 255, 255};
  if (fontColor == NULL){
    fontColor = buff;
  }

  float alpha = 0;
  for (int y = 0; y < glyphSlot->bitmap.rows; ++y){
    for (int x = 0; x < glyphSlot->bitmap.width; ++x){

      unsigned char glyphValue = glyphSlot->bitmap.buffer[y * glyphSlot->bitmap.width + x];
      alpha = (255.0f - glyphValue) / 255.0f;

      cimg_forC(image, c){
        unsigned char value = (float) glyphValue*fontColor[c]/(255.0f);
        image(x + shiftX, y + shiftY, c) = 
        alpha * image(x + shiftX, y + shiftY, c) + (1.0 - alpha) * value;
      }
    }
  }   
}

void drawText(
  FT_Face& face,
  CImg<unsigned char>& image,
  const int& heightText,
  const std::wstring& text,
  const int& leftTopX,
  const int& leftTopY,
  unsigned char fontColor[] = NULL,
  const int separeteGlyphWidth = 1
){
  
  FT_Set_Pixel_Sizes(face, 0, heightText);
  FT_GlyphSlot glyphSlot = face->glyph;  
  
  int shiftX = leftTopX;
  int shiftY = 0;
  for(int numberSymbol = 0; numberSymbol < text.length(); ++numberSymbol){
    shiftY = leftTopY;
 
    bool isSpace = false;
    FT_ULong symbol = text.at(numberSymbol);
    if (symbol == ' ') {
      symbol = 'a';
      isSpace = true;
    }

    if(FT_Load_Char(face, symbol, FT_LOAD_RENDER)){
       throw "Error, glyph not load!! \n";
    }

    float shiftFactor = glyphSlot->bitmap.rows - glyphSlot->bitmap_top; 
    shiftY += shiftFactor;
    shiftY +=  (heightText > glyphSlot->bitmap.rows) ? heightText - glyphSlot->bitmap.rows : 0;
    
    if(!isSpace){
      drawGlyph(glyphSlot, image, shiftX, shiftY, fontColor);
    }
    shiftX += glyphSlot->bitmap.width + separeteGlyphWidth;
  }
}

void mug_init_font(handle_t handle, char *font)
{
  if(font == NULL) 
    font = (char*)DEFAULT_FONT;

  initFreetype(ftlib, face, font);
}
