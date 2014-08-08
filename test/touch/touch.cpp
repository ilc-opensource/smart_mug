#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mug.h>

void dump_touch(int x, int y, int id)
{
  printf("%d: [%4d, %4d]\n", id, x, y);
}

void dump_swipe(gesture_t gesture, char* info)
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
  } 
}

int main(int argc, char** argv)
{
  handle_t handle = mug_touch_init();

  // register call backs
  mug_touch_on(dump_touch);
  mug_gesture_on(MUG_SWIPE, dump_swipe);

  // run the loop
  mug_touch_loop(handle);
}
