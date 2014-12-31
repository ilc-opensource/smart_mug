#define __error_t_defined
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <list>
using namespace std;

#include <iohub_client.h>
#include <mug.h>

#define MOTION_DEFAULT_INTERVAL 200
#define MPU_ACC_G               16384

#ifndef USE_IOHUB
#include <io.h>
#endif

void mix(float gx, float gy, float gz, float ax, float ay, float az)  
{
  const float Kp= 3.0f;
  const float Ki= 0.0005f;
  const float halfT=0.16f;
  const float angl2rad=3.14/180;
  const float rad2angl=180/3.14;
  static float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
  static float exInt = 0, eyInt = 0, ezInt = 0;
  float  angle_by_mix_x=0, angle_by_mix_y=0, angle_by_mix_z=0;
  float norm;
  float vx, vy, vz;
  float ex, ey, ez;
  float t11;
  float t12;
  float t13;
  float t23;
  float t33;

  norm = sqrt(ax*ax + ay*ay + az*az);
  ax = ax / norm;
  ay = ay / norm;
  az = az / norm;

  vx = 2*(q1*q3 - q0*q2);
  vy = 2*(q0*q1 + q2*q3);
  vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

  ex = (ay*vz - az*vy);
  ey = (az*vx - ax*vz);
  ez = (ax*vy - ay*vx);
  exInt = exInt + ex*Ki;
  eyInt = eyInt + ey*Ki;
  ezInt = ezInt + ez*Ki;

  gx = gx + Kp*ex + exInt;
  gy = gy + Kp*ey + eyInt;
  gz = gz + Kp*ez + ezInt;

  q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
  q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
  q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
  q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;

  norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  q0 = q0 / norm;
  q1 = q1 / norm;
  q2 = q2 / norm;
  q3 = q3 / norm;

  t11=q0*q0+q1*q1-q2*q2-q3*q3;
  t12=2.0*(q1*q2+q0*q3);
  t13=2.0*(q1*q3-q0*q2);
  t23=2.0*(q2*q3+q0*q1);
  t33=q0*q0-q1*q1-q2*q2+q3*q3;

  angle_by_mix_x = atan2(t23,t33);
  angle_by_mix_y = -asin(t13);
  angle_by_mix_z = atan2(t12,t11);

  if (angle_by_mix_z < 0){
    angle_by_mix_z += 360*angl2rad;
  }

  angle_by_mix_x *=  rad2angl;
  angle_by_mix_y *=  rad2angl;
  angle_by_mix_z *=  rad2angl;
// printf("\n  angle_by_mix_x = %d, angle_by_mix_y = %d, angle_by_mix_z = %d ", (int)angle_by_mix_x, (int)angle_by_mix_y, (int)angle_by_mix_z);
}



void motion_data_to_angel(int ax, int ay, int az, int gx, int gy, int gz,
                        float *angle_x, float *angle_y, float *angle_z)
{
  //printf("async ax:%8d, ay:%8d, az:%8d, gx:%8d, gy:%8d, gz:%8d\n",
    //      ax, ay, az, gx, gy, gz);
  static struct  timeval  nowtime;
  static struct  timeval  lasttime;
  float timer;

  const float acc_r=0.00006103;
  const float gyro_r=0.0001335;
  const float rad2angl = 57.3;
  float acc_x, acc_y, acc_z;
  float gyro_x, gyro_y, gyro_z;
  float acc_offset_x, acc_offset_y, acc_offset_z;
  float gyro_offset_x, gyro_offset_y, gyro_offset_z;
  float  angle_by_acc_x, angle_by_acc_y, angle_by_acc_z;
  static  float  angle_by_gyro_x, angle_by_gyro_y, angle_by_gyro_z;
  static  float  angle_by_gyro_xx, angle_by_gyro_yy, angle_by_gyro_zz;

  gx += 200;    //offset
  gy -= 100;
  gz -= 80;
  acc_x = (float)ax*acc_r;
  acc_y = (float)ay*acc_r;
  acc_z = (float)az*acc_r;
  gyro_x = (float)gx*gyro_r;
  gyro_y = (float)gy*gyro_r;
  gyro_z = (float)gz*gyro_r;

  if(gyro_y > -0.01 && gyro_x < 0.01)
    gyro_x = 0;

  if(gyro_y > -0.01 && gyro_y < 0.01)
    gyro_y = 0;

  if(gyro_z > -0.01 && gyro_z < 0.01)
    gyro_z = 0;

  angle_by_acc_x= atan(acc_x/sqrt(acc_z*acc_z + acc_y*acc_y))*rad2angl;
  angle_by_acc_y= atan(acc_y/sqrt(acc_x*acc_x + acc_z*acc_z))*rad2angl;
  angle_by_acc_z= atan(acc_z/sqrt(acc_x*acc_x + acc_y*acc_y))*rad2angl;

  gettimeofday(&nowtime,NULL);
  timer = 1000000 * (-lasttime.tv_sec+nowtime.tv_sec)- lasttime.tv_usec+nowtime.tv_usec;
  timer /= 1000000;

  angle_by_gyro_x = (angle_by_gyro_x + gyro_x * timer);
  angle_by_gyro_y = (angle_by_gyro_y + gyro_y * timer);
  angle_by_gyro_z = (angle_by_gyro_z + gyro_z * timer);
  lasttime = nowtime;
  angle_by_gyro_xx = rad2angl*angle_by_gyro_x;
  angle_by_gyro_yy = rad2angl*angle_by_gyro_y;
  angle_by_gyro_zz = rad2angl*angle_by_gyro_z;
  //printf("\n  angle_by_acc_x = %f, angle_by_acc_y = %f, angle_by_acc_z = %f ", angle_by_acc_x, angle_by_acc_y, angle_by_acc_z);

  *angle_x = angle_by_acc_x;
  *angle_y = angle_by_acc_y;
  *angle_z = angle_by_acc_z;

}


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
  motion_angel_cb_t acb;
  motion_shake_cb_t scb;

  int           interval;
}req_motion_t;


static req_motion_t *reqm = NULL;
typedef list<int> acc_list_t;

typedef struct _shake_t {

  acc_list_t x_trace;
  acc_list_t y_trace;
  
  int period;
  int count;
  int length;
  int least_times;

  int least_acc;

}shake_t;

static shake_t shake;

#define DEFAULT_SHAKE_PERIOD 1000
#define DEFAULT_SHAKE_TIMES  1
#define DEFAULT_SHAKE_SENSITIVITY 10

void mug_set_shake(int period, int times) 
{
  shake.period = period;
  shake.least_times = times;

  int wanted_times  = (2 * 2 * times);

  int real_times = (period / reqm->interval);

  if(real_times > wanted_times) {
    shake.length = real_times;
  } else {
    shake.length = wanted_times;
    reqm->interval = period / wanted_times;
  }
}

void init_shake()
{
  shake.x_trace.clear();
  shake.y_trace.clear();
  shake.count = 0;
  shake.least_times = 0;
  shake.length = 0;
  shake.period = 0;
  shake.least_acc = MPU_ACC_G / DEFAULT_SHAKE_SENSITIVITY;

  mug_set_shake(DEFAULT_SHAKE_PERIOD, DEFAULT_SHAKE_TIMES);
}

int get_shake_interval()
{
  return shake.period / (2 * 2 * shake.least_times);
}

bool is_shake(acc_list_t *trace, int least_times, int least_acc) 
{
  int neg = 0, pos = 0;

  for(acc_list_t::iterator itr = trace->begin();
      itr != trace->end();
      itr++) {

    if(-1 * least_acc <= *itr && *itr <= least_acc)
      continue;

    if(*itr > 0)
      pos++;
    else
      neg++;
  }

  int min = (pos > neg) ? neg : pos;

  return (min >= least_times);
}

void print_shake_trace(acc_list_t *trace)
{
  printf("[ ");

  for(acc_list_t::iterator itr = trace->begin();
      itr != trace->end();
      itr++) {
    printf("%d ", *itr);
  }

  printf("]");
}

void detect_shake(int ax, int ay, int az, motion_shake_cb_t scb)
{
  static bool is_shaking = false;

#ifdef DEBUG_SHAKE
  printf("X: ");
  print_shake_trace(&shake.x_trace);
  printf("\n");

  printf("Y: ");
  print_shake_trace(&shake.y_trace);
  printf("\n");
#endif

  if(shake.count < shake.length) {
    shake.x_trace.push_back(ax);
    shake.y_trace.push_back(ay);
    shake.count++;
    return;
  }

  bool this_shaking = (is_shake(&(shake.x_trace), shake.least_times, shake.least_acc) 
                      || is_shake(&(shake.y_trace), shake.least_times, shake.least_acc));

  if(is_shaking) {
    if(this_shaking) {
      // continue shaking 
    } else {
      is_shaking = false;
      scb(false);
    }
  } else {
    if(this_shaking) {
      is_shaking = true;
      scb(true);
    } else {
      // nothing to do
    }
  }

  // clear shake traces 
  shake.x_trace.clear();
  shake.y_trace.clear();
  shake.count = 0;

}

void mug_config_shake(handle_t handle, int period, int times)
{
  mug_set_shake(period, times);
}

void run_motion_timer(uv_timer_t *req, int status)
{
  req_motion_t *motion = (req_motion_t*)(req->data); 
  motion_data_t *data = &(motion->data);
  motion->error = mug_read_motion(motion->handle, &(motion->data));  

  if(motion->error == ERROR_NONE && status == 0) {

    if(motion->cb != NULL) {
      (motion->cb)(data->ax, data->ay, data->az, data->gx, data->gy, data->gz);
    }

    if(motion->acb != NULL) {
      float angle_x, angle_y, angle_z;
      motion_data_to_angel(data->ax, data->ay, data->az, data->gx, data->gy, data->gz,
                           &angle_x, &angle_y, &angle_z);
      (motion->acb)(angle_x, angle_y, angle_z);
    }

    if(motion->scb != NULL) {
      detect_shake(data->ax, data->ay, data->az, motion->scb);
    }

  }

}
 
void run_motion_thread(void *arg)
{
  uv_run(motion_loop, UV_RUN_DEFAULT);
}

void mug_motion_on(handle_t handle, motion_cb_t cb)
{
  reqm->cb = cb;
}

void mug_motion_angle_on(handle_t handle, motion_angel_cb_t acb)
{
  reqm->acb = acb;
}

void mug_motion_shake_on(handle_t handle, motion_shake_cb_t scb)
{
  reqm->scb = scb;
  init_shake();
}

void mug_run_motion_watcher(handle_t handle)
{
  if(reqm->scb) {
    if(shake.least_times < 0) {

    }
  }

  uv_timer_start(&motion_timer, run_motion_timer, 0, reqm->interval);
  uv_run(motion_loop, UV_RUN_DEFAULT);
}

void motion_init(handle_t handle)
{
  // lazy initialization
  if(motion_loop == NULL)
    motion_loop = uv_default_loop();

  uv_timer_init(motion_loop, &motion_timer);

  reqm = (req_motion_t*)malloc(sizeof(req_motion_t));
  memset(reqm, 0, sizeof(req_motion_t));
  reqm->handle = handle;

  motion_timer.data = (void*)reqm;

  reqm->interval = MOTION_DEFAULT_INTERVAL;
}
#else

void motion_init(handle_t handle)
{
}

#endif


void mug_set_motion_timer(handle_t handle, int interval)
{
  MUG_ASSERT(motion_loop != NULL && reqm != NULL, "mug is not initialized\n");
  reqm->interval = interval;
}

handle_t mug_motion_init()
{
  handle_t handle = mug_init(DEVICE_MPU);
  motion_init(handle);

  return handle;
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

