#ifndef MUG_H
#define MUG_H

#define MAX_ROWS 12
#define MAX_COLS 16
#define MAX_COMPRESSED_ROWS MAX_ROWS
#define MAX_COMPRESSED_COLS (MAX_COLS/2)
#define COMPRESSED_SIZE (MAX_COMPRESSED_ROWS * MAX_COMPRESSED_COLS)

#ifndef _LIBIOHUB_H_
typedef long handle_t;

/*error message*/
typedef enum {
  ERROR_NONE = 0,
  ERROR_NOT_AVAILABLE = -1,
  ERROR_MESSAGE_NOT_SENT = -2,
  ERROR_CAN_NOT_GET_REPLY = -3,
} error_t;
#endif

#define IMG_OK 0
#define IMG_ERROR -1

#ifdef __cplusplus
extern "C" { 
#endif

// device control
handle_t mug_init();
void     mug_close(handle_t handle);

// display
error_t  mug_disp_raw(handle_t handle, char* imgData);
error_t  mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval);

// raw image buffer
char* mug_create_raw_buffer();
void  mug_free_raw_buffer(char *buf);

// image
int   mug_read_img(char *fname, char *buf);
int   mug_disp_img(handle_t handle, char* name); 

char* mug_read_img_N(char* names, int *num);
int   mug_disp_img_N(handle_t handle, char *names, int interval);

#ifdef __cplusplus
}
#endif

class Mug {
public: 
  Mug();
  ~Mug();
  int dispRaw(char* data, int number = 1, int interval = 40);
  int dispImgs(char* names, int interval = 40);

private:
  handle_t m_handle;
};
#endif
