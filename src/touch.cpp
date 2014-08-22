#define __error_t_defined
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <pthread.h>

#include <list>
#include <vector>
#include <map>

#include <iohub_client.h>
#include <mug.h>

#ifndef USE_IOHUB
#include <io.h>
#endif
using namespace std;

#define MT_INVALID_VALUE -1

#define TOUCH_TRACE_NUM 2

#define TOUCH_READ_NUM 2

#define TRACE_MIN_NUM 5

#define HOLD_MIN_NUM  30

#define HOLD_MIN_PIXEL 2

#define IS_TWO_FINGER(tk) (tk[0]->size() != 0 && tk[1]->size() != 0)

#define SCALE_X(x) ((x) / TOUCH_WIDTH_SCALE)
#define SCALE_Y(y) ((y) / TOUCH_HEIGHT_SCALE)

//#define DEBUG

typedef list<input_event>              event_list_t;
typedef list<touch_point_t>            touch_trace_t;
typedef vector<touch_trace_t*>         touch_track_t;
typedef map<gesture_t, gesture_cb_t>   gesture_to_cb_t;
typedef map<touch_event_t, touch_event_cb_t> touch_event_to_cb_t;

touch_track_t touch_tracks(TOUCH_TRACE_NUM);

gesture_to_cb_t     gesture_to_cb;
touch_cb_t          touch_cb = NULL;
touch_event_to_cb_t touch_event_to_cb;

char default_info[128];

#define DEFAULT_INFO default_info

//#define debug_printf printf
#define debug_printf(...) 

#ifdef USE_LIBUV

#include <uv.h>


static uv_loop_t   *touch_loop = NULL;

// async handler
uv_async_t  async_touch;
uv_async_t  async_gesture;
uv_async_t  async_touch_event;
 
#define LOCK_ uv_mutex_lock(&uv_mutex)
#define UNLOCK_  uv_mutex_unlock(&uv_mutex)

typedef struct uv_touch_ {
  int        x, y;
  int        id;
  touch_cb_t cb;
} uv_touch_t;

void uv_touch_cb(uv_async_t *handle, int status) 
{
  static int i = 0;

  uv_touch_t *touch = (uv_touch_t*)(handle->data);
  debug_printf("%s: %d (%d x %d, %d)\n", __FUNCTION__, i++, touch->x, touch->y, touch->id);
  fflush(0);
  touch->cb(touch->x, touch->y, touch->id);

  free(touch);
}

void involk_touch_cb(touch_cb_t cb, int x, int y, int id)
{
  static int i = 0;
  MUG_ASSERT(cb, "no callback for touch\n");

  uv_touch_t *touch = (uv_touch_t*)malloc(sizeof(uv_touch_t));
  touch->cb = cb;
  touch->x = x;
  touch->y = y;
  touch->id = id;
  async_touch.data = (void*)touch;

  debug_printf("%s: %d (%d x %d, %d)\n", __FUNCTION__, i++, touch->x, touch->y, touch->id);
  //fflush(0);

  uv_async_send(&async_touch);  
}

typedef struct uv_touch_event_ {
  touch_event_t event;
  int        x, y;
  int        id;
  touch_event_cb_t cb;
} uv_touch_event_t;

void uv_touch_event_cb(uv_async_t *handle, int status)
{
  uv_touch_event_t *touch_event = (uv_touch_event_t*)(handle->data);

  debug_printf("%s event %d @ (%d, %d, %d)\n", __FUNCTION__, touch_event->event, touch_event->x, touch_event->y, touch_event->id);

  touch_event->cb(touch_event->event, touch_event->x, touch_event->y, touch_event->id);

  free(touch_event);
}

void involk_touch_event_cb(touch_event_cb_t cb, touch_event_t event, int x, int y, int id)
{
  uv_touch_event_t *touch_event = (uv_touch_event_t*)malloc(sizeof(uv_touch_event_t));

  MUG_ASSERT(cb, "no callback for touch event\n");

  touch_event->cb = cb;
  touch_event->event = event;
  touch_event->x = x;
  touch_event->y = y;
  touch_event->id = id;

  async_touch_event.data = (void*)touch_event;

  debug_printf("%s event: %d @ (%d, %d, %d)\n", __FUNCTION__, event, x, y, id);

  uv_async_send(&async_touch_event);
  
#ifdef USE_LIBUV
  //dirty work around
  usleep(1 * 1000);
#endif
} 

typedef struct uv_gesture_ {
  gesture_t     gesture;
  char         *info;
  gesture_cb_t  cb;
} uv_gesture_t;

void uv_gesture_cb(uv_async_t *handle, int status)
{
  uv_gesture_t *gesture = (uv_gesture_t*)(handle->data);
  debug_printf("%s\n", __FUNCTION__);
  gesture->cb(gesture->gesture, gesture->info);

  free(gesture);
}

void involk_gesture_cb(gesture_cb_t cb, gesture_t g, char* info)
{
  uv_gesture_t *gesture = (uv_gesture_t*)malloc(sizeof(uv_gesture_t));

  MUG_ASSERT(cb, "no callback for gesture\n");

  gesture->gesture = g;
  gesture->info = info;
  gesture->cb = cb;
  async_gesture.data = (void*)gesture;

  debug_printf("%s\n", __FUNCTION__);

  uv_async_send(&async_gesture);
}

#define INVOLK_TOUCH_CB(cb, x, y, id) involk_touch_cb(cb, x, y, id)
#define INVOLK_GESTURE_CB(cb, g, info) involk_gesture_cb(cb, g, info)
#define INVOLK_TOUCH_EVENT_CB(cb, e, x, y, id) involk_touch_event_cb(cb, e, x, y, id)
#else
#define LOCK_
#define UNLOCK_
#define INVOLK_TOUCH_CB(cb, x, y, id) cb(x, y, id)
#define INVOLK_GESTURE_CB(cb, g, info) cb(g, info)
#define INVOLK_TOUCH_EVENT_CB(cb, e, x, y, id) cb(e, x, y, id)
#endif

void dump_point(touch_point_t *p)
{
  debug_printf("%2d: (%4d, %4d)", p->tracking_id, p->x, p->y);
}

void dump_trace(touch_trace_t *p)
{
  touch_point_t *pt;

  for(touch_trace_t::iterator itr = p->begin(); itr != p->end(); itr++) {
    pt = &(*itr);
    dump_point(pt);
    debug_printf("\n");
  }
}

void dump_track(touch_track_t *p)
{
  touch_trace_t *tr;

  for(int i = 0; i < TOUCH_TRACE_NUM; i++) {
    debug_printf("trace %d\n", i);
    dump_trace(p->at(0));
  }
}

void reset_point(touch_point_t *p)
{
  memset(p, 0, sizeof(touch_point_t));

  p->x = MT_INVALID_VALUE;
  p->y = MT_INVALID_VALUE;
  p->tracking_id = MT_INVALID_VALUE;
}

void init_tracks()
{
  strcpy(default_info, "no info");
  for(int i = 0; i < TOUCH_TRACE_NUM; i++) {
    touch_tracks[i] = new touch_trace_t();
  }
}

void clear_tracks() 
{
  for(int i = 0; i <TOUCH_TRACE_NUM; i++) {
    touch_tracks[i]->clear();
  }
}

bool validate_point(touch_point_t *p)
{
  return (p->x != MT_INVALID_VALUE
          && p->y != MT_INVALID_VALUE
          && p->tracking_id != MT_INVALID_VALUE);
}

touch_event_cb_t get_touch_event_cb(touch_event_t event)
{
  
  touch_event_cb_t cb = NULL;

  if(touch_event_to_cb.find(TOUCH_EVENT_ALL) != touch_event_to_cb.end())
    cb = touch_event_to_cb[TOUCH_EVENT_ALL];

  if(touch_event_to_cb.find(event) != touch_event_to_cb.end())
    cb = touch_event_to_cb[event];

  return cb;
}

void involk_touch_down(touch_point_t *p) 
{ 

  touch_event_cb_t cb = get_touch_event_cb(TOUCH_DOWN);

  if(cb != NULL)
    INVOLK_TOUCH_EVENT_CB(cb, TOUCH_DOWN, SCALE_X(p->x), SCALE_Y(p->y), p->tracking_id);

}
void add_point(touch_point_t *p)
{
  if(!(p && validate_point(p))) {
#ifdef DEBUG
    debug_printf("abondon ");
    dump_point(p);
    debug_printf("\n");
#endif
    return;
  }

  touch_point_t last;
  touch_trace_t *trace = touch_tracks[p->tracking_id];
  bool run_cb = false;

  if(trace->empty()) {
    involk_touch_down(p);
  }

  if(touch_cb) {
    if(!trace->empty()) {
      last = trace->back();
      if(last.x != p->x || last.y != p->y)
        run_cb = true;
    } else {
      run_cb = true;
    }

    if(run_cb) 
      INVOLK_TOUCH_CB(touch_cb, SCALE_X(p->x), SCALE_Y(p->y), p->tracking_id);
  }

  touch_point_t save = *p;

  debug_printf("+(%d, %d, %d) %d\n", p->x, p->y, p->tracking_id, trace->size());
  trace->push_back(save);
  
}

void normalize_point(touch_point_t *point)
{
  int oldx = point->x;
  int oldy = point->y; 
  
  point->x = TOUCH_WIDTH - oldy; 
  point->y = oldx;
}

void parse_event(input_event *event)
{
  static touch_point_t point_save = {MT_INVALID_VALUE};

  if(event->type == EV_SYN && event->code == SYN_DROPPED) {
#ifdef DEBUG
    debug_printf("drpped ");
    dump_point(&point_save);
    debug_printf("\n");
#endif
  }

  if(event->type == EV_SYN && event->code == SYN_MT_REPORT) {
    normalize_point(&point_save);
    add_point(&point_save);
    reset_point(&point_save);
  };

  if(event->type == EV_ABS) {
    switch(event->code) {

    case ABS_MT_POSITION_X:
      point_save.x = event->value;
      break;

    case ABS_MT_POSITION_Y:
      point_save.y = event->value;
      break;

    case ABS_MT_PRESSURE:
      point_save.pressure = event->value;
      break;

    case ABS_MT_TRACKING_ID:
      point_save.tracking_id = event->value;
      break;

    case ABS_MT_TOUCH_MAJOR:
      point_save.touch_major = event->value;
      break;
    }
  }
}

bool validate_trace(touch_trace_t *tr)
{
  if(tr->size() < TRACE_MIN_NUM) {
    return false;
  }

  return true;
}

void validate_track()
{
  touch_trace_t *tr;
	
  for(int i = 0; i < TOUCH_TRACE_NUM; i++) {
    tr = touch_tracks[i];
    if(!validate_trace(tr)) {
      debug_printf("abondon tr[%d] size: %d\n", i, tr->size());
      tr->clear();
    }
  }
}

gesture_t calc_dir(touch_trace_t *tr)
{
  debug_printf("%s ", __FUNCTION__);
  dump_trace(tr);
  touch_point_t *start, *end;
  start = &(tr->front());
  end = &(tr->back());
  
  int x = end->x - start->x;
  int y = end->y - start->y;

  int absx = x;
  int absy = y;

  gesture_t rec = MUG_NO_GESTURE;

  if(x < 0)
    absx = 0 - absx;

  if(y < 0)
    absy = 0 - absy;

  if(absx > absy) {

    // make sure swip 1/4 distance    
    if(absx < TOUCH_WIDTH / 4) return rec;

    if(x > 0)
      rec = MUG_SWIPE_RIGHT;
    else
      rec = MUG_SWIPE_LEFT;
  } else if(absx < absy){

    if(absy < TOUCH_HEIGHT / 4) return rec;
    if( y > 0)
      rec = MUG_SWIPE_DOWN;
    else
      rec = MUG_SWIPE_UP;
  } else {
  }

  return rec;
}

bool is_hold(touch_trace_t *tr)
{
  touch_point_t start, end;
  start = tr->front();
  end   = tr->back();

  start.x = SCALE_X(start.x);
  start.y = SCALE_X(start.y);

  end.x = SCALE_X(end.x);
  end.y = SCALE_X(end.y);

  int xdiff = end.x - start.x;
  int ydiff = end.y - start.y;

  return((-HOLD_MIN_PIXEL < xdiff && xdiff < HOLD_MIN_PIXEL )
         && (-HOLD_MIN_PIXEL < ydiff && ydiff < HOLD_MIN_PIXEL ));

}

bool parse_swipe(gesture_t g, gesture_cb_t cb, touch_track_t *tk)
{
  touch_trace_t *tr0 = tk->at(0);
  touch_trace_t *tr1 = tk->at(1);

  if(!tr0->empty()&& !tr1->empty() // both trace 0 and 1 are all set
     && (g == MUG_GESTURE || MUG_SWIPE_2 <= g && g <= MUG_SWIPE_DOWN_2)) {
    gesture_t rec0 = calc_dir(tr0);
    gesture_t rec1 = calc_dir(tr1);
    
    if(rec0 != MUG_NO_GESTURE && rec0 == rec1) {
      gesture_t rec = (gesture_t)(MUG_SWIPE_2 + (rec0 - MUG_SWIPE));
      if(g == MUG_GESTURE || g == MUG_SWIPE_2 || g == rec) {
        INVOLK_GESTURE_CB(cb, rec, DEFAULT_INFO);  
      }
    }
  }

  if(!tr0->empty() && tr1->empty() //only trace 0 is set 
     && (g == MUG_GESTURE || MUG_SWIPE <= g && g <= MUG_SWIPE_DOWN)) {
    gesture_t rec = calc_dir(tr0);

    if(rec != MUG_NO_GESTURE) {
      if(g == MUG_GESTURE || g == MUG_SWIPE || g == rec) {
        INVOLK_GESTURE_CB(cb, rec, DEFAULT_INFO);      
      }
    }
  }
 
  return true;
}

void parse_gesture(gesture_t g, gesture_cb_t cb, touch_track_t *tk) 
{

  if(g == MUG_GESTURE || MUG_SWIPE <= g && g <= MUG_SWIPE_DOWN_2)
    parse_swipe(g, cb, tk);
}

void parse_all_gesture()
{
  for(gesture_to_cb_t::iterator itr = gesture_to_cb.begin();
      itr != gesture_to_cb.end();
      itr++) {
    parse_gesture((*itr).first, (*itr).second, &touch_tracks);
  }
}

void parse_touch_event(touch_event_t event, touch_trace_t *tr)
{
  touch_point_t back = tr->back();
  touch_point_t *last = &back;

  touch_event_cb_t cb = get_touch_event_cb(event);

  debug_printf("parse_touch_event: (%d, %d, %d)\n", last->tracking_id, last->x, last->y);
  if(event == TOUCH_EVENT_ALL || event == TOUCH_UP) {
    INVOLK_TOUCH_EVENT_CB(cb, TOUCH_UP, SCALE_X(last->x), SCALE_Y(last->y), last->tracking_id);
  }

  if(event == TOUCH_EVENT_ALL || event == TOUCH_CLICK) {
    if(is_hold(tr) && tr->size() < HOLD_MIN_NUM)
      INVOLK_TOUCH_EVENT_CB(cb, TOUCH_CLICK, SCALE_X(last->x), SCALE_Y(last->y), last->tracking_id);
  }

  if(event == TOUCH_EVENT_ALL || event == TOUCH_HOLD) {
    if(is_hold(tr) && tr->size() > HOLD_MIN_NUM)
      INVOLK_TOUCH_EVENT_CB(cb, TOUCH_HOLD, SCALE_X(last->x), SCALE_Y(last->y), last->tracking_id);
  }

}

void parse_all_touch_event()
{
  for(int i = 0; i < TOUCH_TRACE_NUM; i++) {
    touch_trace_t *tr = touch_tracks[i];
    if(tr->empty())
      continue;
    for(touch_event_to_cb_t::iterator itr = touch_event_to_cb.begin();
        itr != touch_event_to_cb.end();
        itr++) {
        parse_touch_event((*itr).first, tr);
    }
  }
}

handle_t mug_touch_init() 
{

  handle_t handle = mug_init(DEVICE_TP);
  MUG_ASSERT(handle, "can not init touch\n");

  return handle;
}

void mug_touch_on(touch_cb_t cb) 
{
  touch_cb = cb;
}

void mug_touch_event_on(touch_event_t event, touch_event_cb_t cb)
{
  touch_event_to_cb[event] = cb;
}

void mug_gesture_on(gesture_t g, gesture_cb_t cb)
{
  gesture_to_cb[g] = cb;
}

void mug_read_touch_data(handle_t handle)
{
  struct input_event events[TOUCH_READ_NUM];
  static bool is_reading = false;

#ifdef USE_IOHUB
  error_t err = iohub_send_command(handle, 
                                   IOHUB_CMD_TOUCH_PANEL, 
                                   (char*)events,
                                   sizeof(events));
#else
  error_t err = dev_send_command(handle, 
                                 IOHUB_CMD_TOUCH_PANEL, 
                                 (char*)events,
                                 sizeof(events));
#endif

  if(err) {
    if(is_reading) {
      validate_track();
      parse_all_touch_event();
      parse_all_gesture();
      clear_tracks();
    }   
    is_reading = false;
  } else {
    is_reading = true;
    for(int i = 0; i < TOUCH_READ_NUM; i++) {
      parse_event(&events[i]);
    } 
  }

}

void mug_touch_loop(handle_t handle)
{
  init_tracks();
  while(1) {
    mug_read_touch_data(handle); 
  }
}

#ifdef USE_LIBUV

void uv_touch_loop(uv_work_t *req)
{
  handle_t handle = (handle_t)(req->data);
  mug_touch_loop(handle);
}

void mug_run_touch_thread()
{
  handle_t handle = mug_touch_init();

  //_mutex_init(&uv_mutex);

  touch_loop = uv_default_loop();
  uv_work_t req;
  req.data = (void*)handle;

  uv_async_init(touch_loop, &async_touch,   uv_touch_cb);
  uv_async_init(touch_loop, &async_gesture, uv_gesture_cb);
  uv_async_init(touch_loop, &async_touch_event, uv_touch_event_cb);

  uv_queue_work(touch_loop, &req, uv_touch_loop, NULL);

  uv_run(touch_loop, UV_RUN_DEFAULT);
}

void mug_wait_for_touch_thread()
{
  MUG_ASSERT(false, "can not run mug_wait_for_touch_thread\n");
}

void mug_stop_touch_thread()
{
  MUG_ASSERT(false, "can not run mug_stop_touch_thread\n");
}

#else
void* thread_entry(void* arg)
{
  handle_t handle = (handle_t) arg;
  mug_touch_loop(handle);

  return NULL;
}

pthread_t touch_thread_hdl = (pthread_t)NULL;

void mug_run_touch_thread()
{
  handle_t handle = mug_touch_init();
  pthread_t hdl;
  
  int err;
  err = pthread_create(&hdl, NULL, thread_entry, (void*)handle);

  MUG_ASSERT(!err, "can not create touch thread\n");

  touch_thread_hdl = hdl;
  
  //return hdl;
}

void mug_wait_for_touch_thread()
{
  void *value_ptr;

  MUG_ASSERT(touch_thread_hdl, "touch thread has not been started\n");

  pthread_join(touch_thread_hdl, &value_ptr);
}

void mug_stop_touch_thread()
{
  int err = pthread_cancel(touch_thread_hdl);

  MUG_ASSERT(!err, "can not cancel touch thread");
}
#endif
