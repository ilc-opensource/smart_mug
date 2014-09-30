#ifndef _LIBIOHUB_H_
#define _LIBIOHUB_H_

/* external api for iohub client library */

#include <stdint.h>
#include <errno.h>
#include "message.h"
#include "session.h"
#include "device.h"

#ifdef __cplusplus
#define PREFIX extern "C"
#else
#define PREFIX extern
#endif

#ifndef BOOL
#define BOOL int
#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif
#endif

#define IOHUB_CHANNEL_ADS1198     0
#define IOHUB_CHANNEL_TOUCHPANEL  1
#define IOHUB_CHANNEL_MPU6050     2

#define IOHUB_SESSION_NOT_OPENED NULL

/* return NULL if failed */
PREFIX session_context_t* iohub_open_session(device_t device_type);

PREFIX int iohub_channel_open(session_context_t *ctx, uint32_t chan);
PREFIX int iohub_channel_read(session_context_t *ctx, uint8_t chan, void *buf, size_t len);

PREFIX int iohub_channel_close(session_context_t *ctx, uint8_t chan);
#define iohub_open_stream iohub_channel_open

PREFIX session_context_t* iohub_open_session_with_name(const char *name);

PREFIX void iohub_close_session(session_context_t*);

PREFIX iohub_error_t iohub_send_command(session_context_t *ctx, cmd_t cmdtype, void *data, size_t message_len);
PREFIX int iohub_channel_timed_read(session_context_t *ctx, uint8_t chan, void *buf, size_t len, int millis);

#endif
