#define __error_t_defined
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include <pthread.h>

#include <list>
#include <map>

#include <iohub_client.h>
#include <mug.h>

#ifndef USE_IOHUB
#include <io.h>
#endif
using namespace std;

#define MT_INVALID_VALUE -1

#define TOUCH_READ_NUM 2

#define TRACE_MIN_NUM 10

#define HOLD_MIN_NUM  20

#define HOLD_MIN_PIXEL 2

#define IS_TWO_FINGER(tk) (tk->find(0) != tk->end() && tk->find(1) != tk->end())
//#define DEBUG

typedef list<input_event>     event_list_t;
typedef list<touch_point_t>   touch_trace_t;
typedef map<int, touch_trace_t> touch_track_t;
typedef map<gesture_t, gesture_cb_t> gesture_to_cb_t;

event_list_t events;
touch_track_t tracks;

gesture_to_cb_t gesture_to_cb;
touch_cb_t      touch_cb = NULL;

void dump_point(touch_point_t *p)
{
  printf("%2d: (%4d, %4d)", p->tracking_id, p->x, p->y);
}

void dump_trace(touch_trace_t *p)
{
  touch_point_t *pt;

  for(touch_trace_t::iterator itr = p->begin(); itr != p->end(); itr++) {
    pt = &(*itr);
    dump_point(pt);
    printf("\n");
  }
}

void dump_track(touch_track_t *p)
{
  touch_trace_t *tr;

  for(touch_track_t::iterator itr = p->begin(); itr != p->end(); itr++) {
    printf("trace %d\n", (*itr).first);
    dump_trace(&((*itr).second));
  }
}

void init_point(touch_point_t *p)
{
  memset(p, 0, sizeof(touch_point_t));

  p->x = MT_INVALID_VALUE;
  p->y = MT_INVALID_VALUE;
  p->tracking_id = MT_INVALID_VALUE;
}

void clear_tracks() 
{
  events.clear();
  tracks.clear();
}
bool validate_point(touch_point_t *p)
{
  return (p->x != MT_INVALID_VALUE
          && p->y != MT_INVALID_VALUE
          && p->tracking_id != MT_INVALID_VALUE);
}

void add_point(touch_point_t *p)
{
  if(!(p && validate_point(p))) {
#ifdef DEBUG
    printf("abondon ");
    dump_point(p);
    printf("\n");
#endif
    return;
  }

  touch_point_t last;
  touch_trace_t *trace = &(tracks[p->tracking_id]);
  bool run_cb = false;
  if(touch_cb) {
    if(!trace->empty()) {
      last = trace->back();
      if(last.x != p->x || last.y != p->y)
        run_cb = true;
    } else {
      run_cb = true;
    }

    if(run_cb) 
      touch_cb(p->x / TOUCH_WIDTH_SCALE, p->y / TOUCH_HEIGHT_SCALE, p->tracking_id);
  }

  tracks[p->tracking_id].push_back(*p);
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
    printf("drpped ");
    dump_point(&point_save);
    printf("\n");
#endif
  }

  if(event->type == EV_SYN && event->code == SYN_MT_REPORT) {
    normalize_point(&point_save);
    add_point(&point_save);
    init_point(&point_save);
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
  if(tr->size() < TRACE_MIN_NUM)
    return false;

  return true;
}

gesture_t calc_dir(touch_trace_t *tr)
{
  if(!validate_trace(tr))
    return MUG_NO_GESTURE;

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
  touch_point_t *start, *end;
  start = &(tr->front());
  end = &(tr->back());

  start->x /= TOUCH_WIDTH_SCALE;
  start->y /= TOUCH_HEIGHT_SCALE;

  end->x /= TOUCH_WIDTH_SCALE;
  end->y /= TOUCH_HEIGHT_SCALE;

  int xdiff = end->x - start->x;
  int ydiff = end->y - start->y;

  return(validate_trace(tr) 
         && tr->size() >= HOLD_MIN_NUM
         && (-HOLD_MIN_PIXEL < xdiff && xdiff < HOLD_MIN_PIXEL )
         && (-HOLD_MIN_PIXEL < ydiff && ydiff < HOLD_MIN_PIXEL ));

}

bool parse_hold(gesture_t g, gesture_cb_t cb, touch_track_t *tk)
{
  bool is_two_finger = IS_TWO_FINGER(tk);
  if(!is_two_finger
     && (g == MUG_GESTURE || g == MUG_HOLD)
     && tk->find(0) != tk->end()
     && is_hold(&(tk->at(0)))) {

    cb(MUG_HOLD, NULL);
    return true;
  }

  if(is_two_finger
     && (g == MUG_GESTURE || g == MUG_HOLD_2)
	 && tk->find(0) != tk->end()
     && is_hold(&(tk->at(0)))
     && tk->find(1) != tk->end()
     && is_hold(&(tk->at(1)))) {

    cb(MUG_HOLD_2, NULL);
    return true;
  }
  
  return false;
}

bool parse_swipe(gesture_t g, gesture_cb_t cb, touch_track_t *tk)
{

  bool is_two_finger = IS_TWO_FINGER(tk);

  if(is_two_finger 
     && (g == MUG_GESTURE || MUG_SWIPE_2 <= g && g <= MUG_SWIPE_DOWN_2)) {
    gesture_t rec0 = calc_dir(&(tk->at(0)));
    gesture_t rec1 = calc_dir(&(tk->at(0)));
    
    if(rec0 != MUG_NO_GESTURE && rec0 == rec1) {
      gesture_t rec = (gesture_t)(MUG_SWIPE_2 + (rec0 - MUG_SWIPE));
      if(g == MUG_GESTURE || g == MUG_SWIPE_2 || g == rec) {
        cb(rec, NULL);  
        is_two_finger = true;    
      }
    }
  }

  if(!is_two_finger 
     && (g == MUG_GESTURE || MUG_SWIPE <= g && g <= MUG_SWIPE_DOWN)
     && !is_two_finger) {
    gesture_t rec = calc_dir(&(tk->at(0)));

    if(rec != MUG_NO_GESTURE) {
      if(g == MUG_GESTURE || g == MUG_SWIPE || g == rec) {
        cb(rec, NULL);      
      }
    }
  }
 
  return true;
}

void parse_gesture(gesture_t g, gesture_cb_t cb, touch_track_t *tk) 
{
  if(g == MUG_GESTURE || MUG_SWIPE <= g && g <= MUG_SWIPE_DOWN_2)
    parse_swipe(g, cb, tk);

  if(g == MUG_GESTURE || MUG_HOLD <= g && g <= MUG_HOLD_2)
    parse_hold(g, cb, tk);

}

void parse_all_gesture()
{
  for(gesture_to_cb_t::iterator itr = gesture_to_cb.begin();
      itr != gesture_to_cb.end();
      itr++) {
    parse_gesture((*itr).first, (*itr).second, &tracks);
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
      parse_all_gesture();
      clear_tracks();
    }   
    is_reading = false;
  } else {
    is_reading = true;
  }

  for(int i = 0; i < TOUCH_READ_NUM; i++) {
    parse_event(&events[i]);
  } 
}

void mug_touch_loop(handle_t handle)
{
  while(1) {
    mug_read_touch_data(handle); 
  }
}

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
