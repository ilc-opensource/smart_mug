#ifndef MUG_CPP_H
#define MUG_CPP_H

#include <mug.h>

class Mug {
public:
  Mug();
  ~Mug();
  int dispRaw(char* data, int number = 1, int interval = 40);
  int dispImgs(char* names, int interval = 40);
  error_t readMotion(motion_data_t *data);
private:
  handle_t *m_handles;
  int       m_handleNum;

  handle_t  initHandle(device_t type);
  void      closeHandles();

  handle_t  getHandle(device_t type);
};

#endif
