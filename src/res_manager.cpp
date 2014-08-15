#include <time.h>
#include <errno.h>
#include <res_manager.h>
using namespace std;

extern int isFrontEndApp();

int resFd = -1;
int resource_init(char* name) {
  if (resFd == -1) {
    cout<<getpid()<<" open file "<<name<<endl;
    resFd = open(name, O_RDWR | O_CREAT, 0666);
    if (resFd == -1) {
      cout<<strerror(errno)<<endl;
    }
  }

  return resFd;
}

int resource_wait(int fd) {
  while(true) {
    lockf(fd, F_LOCK, 0);
    // Check if current process is the front end app
    if (isFrontEndApp()) {
      // Go through
      break;
    } else {
      lockf(fd, F_ULOCK, 0);
    }
  }
}

int resource_post(int fd) {
  return lockf(fd, F_ULOCK, 0);
}

time_t timer;
pid_t* shareMemPtr = NULL;
int lockFd = -1;
int isFrontEndApp() {
  int fd;
  int retv;
  lockFd = resFd;

  if (lockFd == -1) {
    cout<<getpid()<<" open file "<<RESOURCE_SYS_FRONT_END_APP<<endl;
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

//  lockf(lockFd, F_LOCK, 0);
  if (*shareMemPtr == 0 || *shareMemPtr == getpid()) {
    retv = true;
  } else {
    if (difftime(time(NULL), timer) > 10) {
      timer = time(NULL);
      cout<<"C program, wait for display, frontEndApp="<<*shareMemPtr<<", currentApp="<<getpid()<<endl;
    }
    retv = false;
  }
//  lockf(lockFd, F_ULOCK, 0);
  return retv;
}
