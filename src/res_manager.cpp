#include <time.h>
#include <errno.h>
#include <res_manager.h>
extern int isFrontEndApp();
using namespace std;

int resFd = -1;
int resource_init(char* name) {
  if (resFd == -1) {
    int resFd = open(name, O_RDWR | O_CREAT, 0666);
    if (resFd == -1) {
      cout<<strerror(errno)<<endl;
    }
  }

  return resFd;
}

int resource_wait(int fd) {
  while(true) {
    lockf(fd, F_LOCK, 4);
    //sem_wait(sem);
    // Check if there is a process with higher priority also wait for this resource
    // Check if current process is the front end app
    if (isFrontEndApp()) {
      // Go through
      break;
    } else {
      lockf(fd, F_ULOCK, 4);
      //sem_post(sem);
    }
  }
}

int resource_post(int fd) {
  return lockf(fd, F_ULOCK, 4);
  //return sem_post(sem);
}

time_t timer;
pid_t* shareMemPtr = NULL;
int lockFd = -1;
int isFrontEndApp() {
  int fd;
  int retv;
  
  if (lockFd == -1) {
    lockFd = open(RESOURCE_SYS_FRONT_END_APP, O_RDWR | O_CREAT, 0666);
    if (lockFd == -1) {
      cout<<strerror(errno)<<endl;
      return false;
    }
  }
  
  if (shareMemPtr == NULL) {
    fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
      cout<<strerror(errno)<<endl;
      return false;
    }
    retv = ftruncate(fd, sizeof(pid_t));
    if (retv == -1) {
      cout<<strerror(errno)<<endl;
      return false;
    }
    shareMemPtr = (pid_t *)mmap(NULL, sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shareMemPtr == MAP_FAILED) {
      cout<<strerror(errno)<<endl;
      return false;
    }
    close(fd);
  }

  lockf(lockFd, F_LOCK, 4);
  if (*shareMemPtr == 0 || *shareMemPtr == getpid()) {
    retv = true;
  } else {
    if (difftime(timer, time(NULL)) > 10) {
      timer = time(NULL);
      cout<<"C program, wait for display, frontEndApp="<<*shareMemPtr<<", currentApp="<<getpid()<<endl;
    }
    retv = false;
  }
  lockf(lockFd, F_ULOCK, 4);
  return retv;
}
