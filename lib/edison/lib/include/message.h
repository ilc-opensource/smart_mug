#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "session.h"
#include "device.h"

/**/
typedef enum {
  MSG_HELLO_WITH_DEVICE_TYPE = 0,
  MSG_HELLO_WITH_DEVICE_TYPE_ACK,
  MSG_HELLO_WITH_SESSION_ID,
  MSG_HELLO_WITH_SESSION_ID_ACK,
  MSG_HELLO_WITH_STREAM_ID,
  MSG_HELLO_WITH_STREAM_ID_ACK,
  MSG_CMD,
  MSG_CMD_ACK,
  MSG_DATA
} msg_t;

typedef enum {
  IOHUB_CMD_ADC = 1,
  IOHUB_CMD_FB = 2,
  IOHUB_CMD_MOTION_SENSOR = 3,
  IOHUB_CMD_TOUCH_PANEL   = 4,
  IOHUB_CMD_CHANNEL_CTRL  = 6,
  IOHUB_CMD_SHUT_DOWN     = 7,
  CMD_MAX
} cmd_t;

typedef struct {
  msg_t msg_type;
  char name[DEVICE_NAME_MAX + 1]; //device name
} hello_with_device_type_msg;

typedef struct {
  msg_t msg_type;
  session_id_t session_id;
} hello_with_device_type_ack_msg;

typedef struct {
  msg_t msg_type;
  session_id_t session_id;
} hello_with_session_id_msg;

typedef struct {
  msg_t msg_type;
  session_id_t session_id;
  int  stream_id;
} hello_with_stream_id_msg;

typedef struct {
  msg_t msg_type;
  int ret;
} hello_ack_msg;

typedef struct {
  msg_t msg_type;
  ret_t ret;
} hello_with_session_id_ack_msg;

typedef struct {
  msg_t msg_type;
  msg_t cmd;
  int len;
  unsigned char buf[];
} cmd_msg;

typedef struct {
  msg_t msg_type;
  ret_t ret;
  int buf_len;
  unsigned char buf[];
} cmd_ack_msg;


#endif
