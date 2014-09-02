#define __error_t_defined
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iohub_client.h>
#include <mug.h>

#ifndef USE_IOHUB
#include <io.h>
#endif

#ifdef USE_LIBUV
#include <uv.h>

static uv_loop_t *motion_loop = NULL;
static uv_timer_t motion_timer;

static uv_thread_t motion_thread; 
typedef struct _req_motion_t {
  motion_data_t data;
  handle_t      handle;
  mug_error_t   error;
  motion_cb_t   cb; 
}req_motion_t;

void run_motion_timer(uv_timer_t *req, int status)
{
  req_motion_t *motion = (req_motion_t*)(req->data); 
  motion_data_t *data = &(motion->data);
  motion->error = mug_read_motion(motion->handle, &(motion->data));  

  if(motion->error != ERROR_NONE || status) {
    (motion->cb)(0, 0, 0, 0, 0, 0);
  } else {
    (motion->cb)(data->ax, data->ay, data->az, data->gx, data->gy, data->gz);
  }

}
 
void run_motion_thread(void *arg)
{
  uv_run(motion_loop, UV_RUN_DEFAULT);
}

void mug_motion_on(handle_t handle, motion_cb_t cb, int interval)
{
  // lazy initialization
  if(motion_loop == NULL)
    motion_loop = uv_default_loop();

  uv_timer_init(motion_loop, &motion_timer);

  req_motion_t *reqm;

  reqm = (req_motion_t*)malloc(sizeof(req_motion_t));
  memset(reqm, 0, sizeof(req_motion_t));
  reqm->cb = cb;
  reqm->handle = handle;

  motion_timer.data = (void*)reqm;

  uv_timer_start(&motion_timer, run_motion_timer, 0, interval);
  //uv_run(motion_loop, UV_RUN_DEFAULT);
}

void mug_run_motion_watcher(handle_t handle)
{
  uv_run(motion_loop, UV_RUN_DEFAULT);
}

#if 0
typedef struct _motion_thread_arg_t {
  handle_t handle;
  motion_cb_t cb;
  int interval;
}motion_thread_arg_t;

void motion_thread_entry(void *arg)
{
  motion_thread_arg_t *motion_arg = (motion_thread_arg_t*)arg;
  _mug_read_motion_async(motion_arg->handle, motion_arg->cb, motion_arg->interval);
}

void wait_for_motion_thread(void) {
  uv_thread_join(&motion_thread);
}

void mug_read_motion_async(handle_t handle, motion_cb_t cb, int interval)
{
  motion_thread_arg_t *motion_arg = (motion_thread_arg_t*)malloc(sizeof(motion_thread_arg_t));
  motion_arg->handle = handle;
  motion_arg->cb = cb;
  motion_arg->interval = interval;
  uv_thread_create(&motion_thread, motion_thread_entry, motion_arg);
  atexit(wait_for_motion_thread);
}
#endif
#endif

handle_t mug_motion_init()
{
  return mug_init(DEVICE_MPU);
}

mug_error_t mug_read_motion(handle_t handle, motion_data_t *data)
{
#ifdef USE_IOHUB
  mug_error_t err = iohub_send_command(handle, 
                                   IOHUB_CMD_MOTION_SENSOR, 
                                   (char*)data, 
                                   sizeof(motion_data_t));
#else
  mug_error_t err = dev_send_command(handle,
                                 IOHUB_CMD_MOTION_SENSOR,
                                 (char*)data,
                                 sizeof(motion_data_t));
#endif
  return err;
}



