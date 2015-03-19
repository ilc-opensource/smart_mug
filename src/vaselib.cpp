#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>
#include "./bme280_support.h"
#include "./bme280.h"
#include <mug.h>
//------------ util func -------------
int die(char *s)
{
	fprintf(stderr,"Error %s\n",s);
	exit(1);
	return 0;
}

float getFeltTempEquation1(float t)
{
	float v=1.08;
	float FletT;
	FletT=13.12+0.6125*t-11.37*pow(v,0.16)+0.3965*pow(v,0.16)*t;
	return FletT;
}

float getFeltTempEquation2(float t,float h)
{
	float hi;
	hi=0.5*(t+16.11111+1.2*(t-20)+0.052222*h);
	return hi;	
}
float getFeltTempEquation3(float t,float h)
{

	float hi;

	float c1,c2,c3,c4,c5,c6,c7,c8,c9;
	c1=(-8.7847);

	c2=1.6114;

	c3=2.3385;

	c4=(-0.1461);

	c5=(-0.0123);

	c6=(-0.0164);

	c7=2.2117*pow(10,-3);

	c8=7.2546*pow(10,-4);

	c9=-3.5820*pow(10,-6);	
	hi=c1+c2*t+c3*h+c4*t*h+c5*t*t+c6*h*h+c7*t*t*h+c8*t*h*h+c9*t*t*h*h;

	return hi;	
}
float getFeltTempDeta1(float t,float h)
{

	float deta1;

	deta1=(((13-h)/4)*pow((17-fabs(1.8*t-63)),0.5))/1.8;
	return deta1;
}
float getFeltTempDeta2(float t,float h)
{


	float deta2;

	deta2=(((h-85)/10)*((55-1.8*t)/5))/1.8;
	return deta2;
}
int getFeltTempAlg(float *FletT,float t,float h)
{

	float *FeltT;
	float HI;
	if(t<10){
		*FletT=getFeltTempEquation1(t);
		return 0;
		}
	else{
		HI=getFeltTempEquation2(t,h);
				
		if(HI<=26){
			*FletT=HI;
			return 0;
		}
		
		else{
			if(h<13 && t>26 &&t <44){
				*FletT=getFeltTempEquation3(t,h)-getFeltTempDeta1(t,h);
				return 0;
			}
			if(h>85 && t>26 &&t <31){
				*FletT=getFeltTempEquation3(t,h)+getFeltTempDeta2(t,h);
				return 0;
			}
			*FletT=getFeltTempEquation3(t,h);
			return 0;
		}
	}	
	*FletT=28;
	return 0;
}

int calAirQualityIndex(float pm25,int *aqi)
{
	if(pm25<0)
		return -1;

	float BPh=0.0,BPl=0.0;
	int Ih=0,Il=0;
	if(pm25>=0.0&&pm25<=15.4)
	{
		BPh=15.4,BPl=0.0;
		Ih = 50, Il=0;
	}else if(pm25>=15.5&&pm25<=40.4)
	{
		BPh=15.5,BPl=40.4;
		Ih = 51, Il=100;
	}
	else if(pm25>=40.5&&pm25<=65.4)
	{
		BPh=40.5,BPl=65.4;
		Ih = 101, Il=150;
	}
	else if(pm25>=65.5&&pm25<=150.4)
	{
		BPh=65.5,BPl=150.4;
		Ih = 151, Il=200;
	}
	else if(pm25>=150.5&&pm25<=250.4)
	{
		BPh=150.5,BPl=250.4;
		Ih = 201, Il=300;
	}
	else if(pm25>=250.5&&pm25<=350.4)
	{
		BPh=250.5,BPl=350.4;
		Ih = 301, Il=400;
	}
	else if(pm25>=350.5&&pm25<=500.4)
	{
		BPh=350.5,BPl=500.4;
		Ih = 401, Il=500;
	}
	*aqi = (Ih-Il)*(pm25-BPl)/(BPh-BPl) + Il;
	
	return 0;	
}
//----------------------get sensor data from drivers----------------------------
void init_bme280()
{
	//printf("Init I2C interface...\n");
	i2c_init() && die("i2c_init");
	
	//printf("Init BME280 sensor...\n");
	bme280_appinit() && die("bme280_appinit\n");
	bme280_settos(1) && die("bme280_settos\n");
	bme280_setpos(2) && die("bme280_setpos\n");
	bme280_sethos(3) && die("bme280_sethos\n");
	bme280_setfilter(4) && die("bme280_setfilter\n");
	bme280_settsb(5) && die("bme280_settsb\n");
	bme280_setmode(3) && die("bme280_setmode\n");
}

void vase_init()
{
	init_bme280();
}
//get the current Temperature,Humidity and Body Feel Temperature
int get_Temp_Hum_FT(float *T,float *H,float *FT)
{
	float p=101.325;
	bme280_gettph(T, &p, H) && die("bme280_gettph\n");
	
	fflush(NULL);
	
	//use the t and h value to calculate the ft value
	getFeltTempAlg(FT,*T,*H);

	return 0;
}

int get_CO2_TVOC(int *co2,int *tvoc)
{
	int resistance = 0.0;
	int result = iaq_getall(co2, &resistance,tvoc) && die("iaq_getall");

	fflush(NULL);

	return result;
}

int get_PM25_AQI(float *PM25,int *aqi)
{
	char filename[40]="/dev/pm25";
	int fp;
	int ret;	
	int ratio[2];
	fp=open(filename,O_RDONLY);
	if(fp<0){
		printf("%s cant's open\n",filename);
		perror("open");
		exit(1);
	}
	else{
		ret=read(fp,&ratio,2*sizeof(int));
		//printf("pm10=%d pm25=%d \n",ratio[0], ratio[1]);
	}

	int d = ratio[0]-ratio[1];
	*PM25  = 0.162263*d*d+0.924214;

	//use the PM25 data to calculate the AQI
	calAirQualityIndex(*PM25,aqi);

	return 0;
}

int get_Light(float *light,float *proximity)
{
	char fileName1[60] = "/sys/devices/virtual/input/input3/als_lux";
	char fileName2[60] = "/sys/devices/virtual/input/input2/prx_raw";

	int fp1,fp2;
	int ret1,ret2;	
	static int preAlux=0,prePrx=0;
	char lux[32],prx[32];
	memset(lux,0,32);
	memset(prx,0,32);

	fp1=open(fileName1,O_RDONLY);
	fp2=open(fileName2,O_RDONLY);
	if(fp1<0 || fp2<0){
		exit(1);
	}
	else{
		read(fp1,&lux,32);
		read(fp2,&prx,32);
		
		*light = atoi(lux);
		*proximity = atoi(prx);
		
		if(prePrx && (*proximity-prePrx)>40)
		{
			*light	= preAlux;
			*proximity = prePrx;
		}
		preAlux = *light;
		prePrx = *proximity;

		//printf("Light light alux=%s\n",lux);
		//printf("Light light prx=%s\n",prx);
	}
	
	return 0;
}

/* get the sound pressure level */
int get_SPL(int *spl)
{
	*spl = 10;
	return 0;
}

int get_vase_data(struct timeval *ts,float *T,float *H,float *FT,int *co2,int *tvoc,float *pm25,int *aqi,float *light,int *spl)
{
	float lightPrx=0.0;

	gettimeofday(ts,NULL);

	get_Temp_Hum_FT(T,H,FT);
	get_CO2_TVOC(co2,tvoc);
	get_PM25_AQI(pm25,aqi);
	get_Light(light,&lightPrx);
	get_SPL(spl);

	return 0;
}

char buf[1024 * 4];

char* get_vase_json()
{
    float lightPrx = 0.0;

    char *p = buf;
    p += sprintf(p, "{ ");
    
    float T, H, FT, pm25, light;
    int co2, tvoc;
    int aqi, spl;

    get_Temp_Hum_FT(&T, &H, &FT);
    p += sprintf(p, "\"T\": %f, ", T);
    p += sprintf(p, "\"H\": %f, ", H);
    p += sprintf(p, "\"FT\": %f, ", FT);

    get_CO2_TVOC(&co2, &tvoc);
    p += sprintf(p, "\"co2\": %d, ", co2);
    p += sprintf(p, "\"tvoc\": %d, ", tvoc);

    get_PM25_AQI(&pm25, &aqi);
    p += sprintf(p, "\"pm25\": %f, ", pm25);
    p += sprintf(p, "\"aqi\": %d, ", aqi);

    get_Light(&light, &lightPrx);
    p += sprintf(p, "\"light\": %f, ", light);

    get_SPL(&spl);
    p += sprintf(p, "\"spl\": %d ", spl);

    p += sprintf(p, "}");

    return buf;    
}

