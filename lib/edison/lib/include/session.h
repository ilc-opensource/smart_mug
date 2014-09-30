#ifndef _SESSION_H_
#define _SESSION_H_

#define UNIX_SOCKET_PATH "/tmp/iohub_daemon_socket"
#define MAX_Q_LENGTH 20
#define MAX_MESSAGE_LENGTH 4096
#define DEVICE_NAME_MAX 15
#define IOHUB_CHANNEL_MAX 3

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>

typedef enum {
  INACTIVE = 0,
  ACTIVE,
  ALWAYS_ON,
} state_t;

typedef unsigned int session_id_t;

/* support mutile clients to daemon */
typedef struct session_state_t {
  state_t state;
  char flag;
  int datafd;
//  int ctlfd;
  session_id_t session_id;
  struct session_state_t *next;
} session_state_t;

typedef struct {
    int datafd;
    session_id_t session_id;
    char name[DEVICE_NAME_MAX + 1];
    char cmd_buff[MAX_MESSAGE_LENGTH];
    int channels[IOHUB_CHANNEL_MAX];
} session_context_t;

/*error message*/
typedef enum {
  ERROR_NONE = 0,
  ERROR_NOT_AVAILABLE = -1,
  ERROR_MESSAGE_NOT_SENT = -2,
  ERROR_CAN_NOT_GET_REPLY = -3,
} iohub_error_t;

typedef enum {
  SUCCESS = 0,
  ERR_DEVICE_NOT_SUPPORT = -1,
  ERR_SESSION_NOT_EXIST = -2,
  ERR_DEVICE_NO_RESPONSE = -3,
  ERR_CMD_NOT_SUPPORT = -4,
} ret_t;


/* system message management */

//socket server
static inline int socket_start_server(const char *name)
{
  int fd;
  struct sockaddr_un addr;

  fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0) return -1;

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, name);

  int on=1;  
  if((setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
  {  
     perror("setsockopt failed");  
    goto error;
   }  

  if(bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
    perror("bind");
    goto error;
  }
  return fd;

error:
  close(fd);
  return -1;
 
}

//socket client
static inline int socket_local_client(const char *name)
{
  int fd;
  struct sockaddr_un addr;

  fd = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(fd < 0) {
    perror("socket");
    return -1;
  }

#if 0
  struct timeval timeout = {0, 100*1000};

  if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0) {
    perror("can not set recv timeout");
    goto error;
  }
#endif

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, name);

  if(connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
    perror("connect");
    goto error;
  }


  return fd;

error:
  close(fd);
  return -1;
}
#endif
