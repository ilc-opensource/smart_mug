#ifndef MUG_RES_MANAGER_H
#define MUG_RES_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <cstring>

#define SHM_NAME "/memmap"
#define LOCK_DISPLAY_TOUCH "/tmp/smart_mug_display_16x12"
#define LOCK_SYS_FRONT_END_APP "/tmp/smart_mug_front_end_app"
#define LOCK_SYS_NOTIFICATION "/tmp/smart_mug_notification"
#define NOTIFICATION "/tmp/smart_mug_notification.json"

extern int resource_init(const char* name);
extern int resource_wait(int fd);
extern int resource_post(int fd);

#endif //MUG_RESMANAGER_H
