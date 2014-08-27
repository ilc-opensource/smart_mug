#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

void dump_touch(int x, int y, int id)
{
  //if(id != 0) return;
  printf("%d: [%4d, %4d]\n", id, x, y);
}

void dump_touch_event(touch_event_t e, int x, int y, int id)
{
  printf("event ");
  
  switch(e) {
    case TOUCH_CLICK:
      printf("click");
      break;
   
    case TOUCH_DOWN:
      printf("touch down");
      break;

    case TOUCH_UP:
      printf("touch up");
      break;

    case TOUCH_HOLD:
      printf("hold");
      break;
  }

  printf(" @ %d ( %d x %d)\n", id, x, y);
}
void dump_gesture(gesture_t gesture, char* info)
{
  switch(gesture) {

  case MUG_SWIPE_LEFT:
    printf("left\n");
    break;

  case MUG_SWIPE_RIGHT:
    printf("right\n");
    break;

  case MUG_SWIPE_UP:
    printf("up\n");
    break;

  case MUG_SWIPE_DOWN:
    printf("down\n");
    break;

  case MUG_SWIPE_LEFT_2:
    printf("left_2\n");
    break;

  case MUG_SWIPE_RIGHT_2:
    printf("right_2\n");
    break;

  case MUG_SWIPE_UP_2:
    printf("up_2\n");
    break;

  case MUG_SWIPE_DOWN_2:
    printf("down_2\n");
    break;

  } 
}

int main()
{
  handle_t handle = mug_touch_init();
  mug_touch_on(handle, dump_touch);

  mug_gesture_on(handle, MUG_GESTURE, dump_gesture);

  mug_touch_event_on(handle, TOUCH_EVENT_ALL, dump_touch_event);

  mug_run_touch_thread(handle);
  mug_wait_for_touch_thread(handle);
}
