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

extern handle_t mug_init();
extern void     mug_close(handle_t handle);
extern error_t  mug_disp_raw(handle_t handle, char* imgData);
extern error_t  mug_disp_raw_N(handle_t handle, char* imgData, int number, int interval);
#endif
