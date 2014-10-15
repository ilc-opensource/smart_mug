#define cimg_display 0 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <string>
using namespace std;

#include <mug.h>

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
  handle_t disp_handle = mug_disp_init();

  char *if_name = (char*)"wlan0";

  if(argc == 2) {
    if_name = argv[1];
  }

  char *ip_str = get_ip(if_name);

  printf(">>>> %s\n", ip_str);

  if(ip_str != NULL) {
    mug_disp_text_marquee(disp_handle, ip_str, "cyan", 150, DISP_INFINITE);
  } else {
    mug_disp_text_marquee(disp_handle, "no ip", "red", 100, DISP_INFINITE);
  }

}
