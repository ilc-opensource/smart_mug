#include <mug.h>

Mug::Mug() 
{
  m_handle = mug_init();
}

Mug::~Mug()
{
  mug_close(m_handle);
}

int Mug::dispRaw(char* data, int number, int interval)
{
  return mug_disp_raw_N(m_handle, data, number, interval); 
}


int Mug::dispImgs(char* names, int interval)
{
  return mug_disp_img_N(m_handle, names, interval);
}

