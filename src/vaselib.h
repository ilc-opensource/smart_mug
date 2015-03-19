#ifndef __INCLUDE_VASELIB_H__
#define __INCLUDE_VASELIB_H__
#include<sys/time.h>
#include<stdint.h>

void init();
//get the current Temperature,Humidity and Body Feel Temperature
int get_Temp_Hum_FT(float *T,float *H,float *FT);

int get_CO2_TVOC(int *co2,int *tvoc);

int get_PM25_AQI(float *PM25,int *aqi);

int get_Light(float *light,float *proximity);

/* get the sound pressure level */
int get_SPL(int *spl);

int get_vase_data(struct timeval *ts,float *T,float *H,float *FT,int *co2,int *tvoc,float *pm25,int *aqi,float *light,int *spl);

#endif
