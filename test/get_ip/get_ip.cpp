#define cimg_display 0 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "CImg.h"
using namespace cimg_library;

#include <string>
using namespace std;

#include <mug.h>

#define INTERVAL 1000

CImg<unsigned char> canvas(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 3, 0);
handle_t disp_handle;

void disp_canvas()
{
  mug_disp_cimg(disp_handle, &canvas); 
}

void clear_canvas()
{
  canvas.draw_rectangle(0, 0, 0,
                        SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                        black);

}

char** split_ip(char *ip_str)
{
  char **ip_split;
  char *p = ip_str;

  ip_split = (char**) malloc(4 * sizeof(char*));

  int cnt = 0;

  ip_split[cnt] = p;

  while(*p != '\0') {
    if(*p == '.') {
      *p = '\0';
      cnt++;
      ip_split[cnt] = p + 1;
    }
    p++;
  }

  for(int i = 0; i < 4; i++) {
    printf("====> %s\n", ip_split[i]);
  }
  
  return ip_split; 
}

void show_ip(char** splits)
{

  int width, height;
  mug_number_text_shape(&width, &height);

  for(int i = 0; i < 4; i++) {
    clear_canvas();
    mug_draw_number_cimg(&canvas, 0, 0, splits[i], red);
    disp_canvas();
    usleep(INTERVAL * 1000);    
  }
}

void init()
{
  disp_handle = mug_disp_init();
}

char* get_ip(char *if_name)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name, if_name)==0)&&(ifa->ifa_addr->sa_family==AF_INET))
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("Interface : <%s>\n",ifa->ifa_name );
            printf("Address   : <%s>\n", host);
            char *temp = (char*)malloc(strlen(host) + 1);
            strcpy(temp, host);
            return temp;
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

int main(int argc, char **argv)
{
  char *if_name = (char*)"wlan0";
  if(argc == 2) {
    if_name = argv[1];
  }

  init();

  char *ip_str = get_ip(if_name);

  printf(">>>> %s\n", ip_str);

  char **split_str;

  if(ip_str != NULL) {
    split_str = split_ip(ip_str);
  }

  show_ip(split_str);
}
