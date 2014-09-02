#include <string.h>
#include <mug_cpp.h>

Mug::Mug() 
{
  m_handleNum = DEVICE_MAX;

  m_handles = new handle_t[m_handleNum];

  memset(m_handles, 0, m_handleNum * sizeof(handle_t));
}

Mug::~Mug()
{
  closeHandles();
}

void Mug::closeHandles()
{
  for(int i = 0; i < m_handleNum; i++) 
    mug_close(m_handles[i]);
}

handle_t Mug::initHandle(device_t type)
{
  if(m_handles[type]) {
    return m_handles[type];
  }

  handle_t handle = mug_init(type);
  m_handles[type] = handle;
  MUG_ASSERT(handle, "can not init handle");
  return handle;
}

handle_t Mug::getHandle(device_t type)
{
  return m_handles[type];
}

int Mug::dispRaw(char* data, int number, int interval)
{
  handle_t handle = initHandle(DEVICE_LED);
  MUG_ASSERT(handle, "can not get handle\n");

  return mug_disp_raw_N(handle, data, number, interval); 
}


int Mug::dispImgs(char* names, int interval)
{
  handle_t handle = initHandle(DEVICE_LED);
  MUG_ASSERT(handle, "can not get handle\n");

  return mug_disp_img_N(handle, names, interval);
}

mug_error_t Mug::readMotion(motion_data_t *data)
{
  handle_t handle = initHandle(DEVICE_MPU);
  MUG_ASSERT(handle, "can not get handle\n");

  return mug_read_motion(handle, data);
}
