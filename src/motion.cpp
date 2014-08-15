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

uv_loop_t *loop = NULL;
typedef struct _req_motion_t {
  motion_data_t data;
  handle_t      handle;
  error_t       error;
  motion_cb_t   cb; 
}req_motion_t;

void run_req_motion(uv_work_t *req)
{
  req_motion_t *motion = (req_motion_t*)(req->data);
  motion->error = mug_read_motion_sensor(motion->handle, &(motion->data));  
}

void after_req_motion(uv_work_t *req, int status)
{
  req_motion_t *motion = (req_motion_t*)(req->data);
  
  if(motion->error != ERROR_NONE || status) {
    (motion->cb)(NULL);
  } else {
    (motion->cb)(&(motion->data));
  }

  free(motion);
}

void mug_read_motion_sensor_async(handle_t handle, motion_cb_t cb)
{
  // lazy initialization
  if(loop == NULL)
    loop = uv_default_loop();

  uv_work_t req;
  req_motion_t *reqm;

  reqm = (req_motion_t*)malloc(sizeof(req_motion_t));
  memset(reqm, 0, sizeof(req_motion_t));
  reqm->cb = cb;
  reqm->handle = handle;

  req.data = (void*)reqm;
  uv_queue_work(loop, &req, run_req_motion, after_req_motion);
  uv_run(loop, UV_RUN_DEFAULT);
}
#endif

handle_t mug_motion_sensor_init()
{
  return mug_init(DEVICE_MPU);
}

error_t mug_read_motion_sensor(handle_t handle, motion_data_t *data)
{
#ifdef USE_IOHUB
  error_t err = iohub_send_command(handle, 
                                   IOHUB_CMD_MOTION_SENSOR, 
                                   (char*)data, 
                                   sizeof(motion_data_t));
#else
  error_t err = dev_send_command(handle,
                                 IOHUB_CMD_MOTION_SENSOR,
                                 (char*)data,
                                 sizeof(motion_data_t));
#endif
  return err;
}



