#include <stdio.h>
#include <stdlib.h>
#include <utf8.h>
#include <string.h>

#define IS_VALIDE(start, offset, end) ((start + offset) < end)

bool utf8_to_unicode_uint(const string &src, vector<unsigned int> &dst) {
  dst.clear();

  unsigned int itr = 0, offset, temp;
  unsigned int length = src.size();
  unsigned char* ptr;
  while (itr < length) {
    ptr = (unsigned char*)&src[itr];
      if (*ptr < 0xC0) {
        offset = 0;
        temp = ptr[0];
      } else if (*ptr < 0xE0) {
        /*2:<11000000>*/
        offset = 1;
        if (!IS_VALIDE(itr, offset, length)) return false;
        temp = (ptr[0] & 0x1f) << 6;
        temp |= (ptr[1] & 0x3f);
     } else if (*ptr < 0xF0) {
        /*3:<11100000>*/
        offset = 2;
        if (!IS_VALIDE(itr, offset, length)) return false;
        temp = (ptr[0] & 0x0f) << 12;
        temp |= (ptr[1] & 0x3f) << 6;
        temp |= (ptr[2] & 0x3f);
     } else if (*ptr < 0xF8) {
       /*4:<11110000>*/
       offset = 3;
       if (!IS_VALIDE(itr, offset, length)) return false;
       temp = (ptr[0] & 0x07) << 18;
       temp |= (ptr[1] & 0x3f) << 12;
       temp |= (ptr[2] & 0x3f) << 6;
       temp |= (ptr[3] & 0x3f);
    } else if (*ptr < 0xFC) {
       /*5:<11111000>*/
       offset = 4;
       if (!IS_VALIDE(itr, offset, length)) return false;
       temp = (ptr[0] & 0x03) << 24;
       temp |= (ptr[1] & 0x3f) << 18;
       temp |= (ptr[2] & 0x3f) << 12;
       temp |= (ptr[3] & 0x3f) << 6;
       temp |= (ptr[4] & 0x3f);
    } else {
       /*6:<11111100>*/
       offset = 5;
       if (!IS_VALIDE(itr, offset, length)) return false;
       temp = (ptr[0] & 0x01) << 30;
       temp |= (ptr[1] & 0x3f) << 24;
       temp |= (ptr[2] & 0x3f) << 18;
       temp |= (ptr[3] & 0x3f) << 12;
       temp |= (ptr[4] & 0x3f) << 6;
       temp |= (ptr[5] & 0x3f);
    }
    dst.push_back(temp);
    itr += offset + 1;
  }
  return true;
}

char* utf8_to_unicode_char(char* src)
{
  string str(src);
  unicode_uint_t dst;
  utf8_to_unicode_uint(src, dst);

  int wsize = dst.size() * sizeof(unsigned int) + 1;
  char *uc = (char*)malloc(wsize); 
  memset(uc, 0, wsize);

  char *p = uc;
  for(int i = 0; i < dst.size(); i++) {
    *(unsigned int*)p = dst[i];
    p += sizeof(unsigned int);
  }
  
  *(wchar_t*)p = L'\0';
  return uc;
}

wchar_t* utf8_to_unicode_wchar(char* src)
{
  return(wchar_t*)(utf8_to_unicode_char(src));
}
