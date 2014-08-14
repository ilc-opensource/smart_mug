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

#define RESOURCE_DISPLAY_TOUCH "/tmp/smart_mug_display_16x12"

#define SHM_NAME "/memmap"  
#define RESOURCE_SYS_FRONT_END_APP "/tmp/smart_mug_front_end_app"   

extern int resource_init(char* name);
extern int resource_wait(int fd);
extern int resource_post(int fd);

#endif //MUG_RESMANAGER_H
