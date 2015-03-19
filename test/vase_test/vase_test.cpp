#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <mug.h>
int main() 
{
    struct timeval ts;
    float temperature=25.67,humidity=56.78,feltTemperature=0.0,pm25=0.0,lightAlux=0.0,lightProximity=0.0;
    int co2=0,tvoc=0,aqi=0,spl;

    vase_init();

    while(true) {
        char *p = get_vase_json();
        printf("%s\n", p);
        sleep(1);
    }
    return 0;
}
