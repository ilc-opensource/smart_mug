#ifndef BME280_SUPPORT_H
#define BME280_SUPPORT_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>

#define BME280_I2C_ADDRESS 0x76
#define IAQ_I2C_ADDRESS    0x5A


// Note all API functions have the same return value:
// return 0: ok, <0: error


// return 0: ok, <0: error
int i2c_init(void);

// app init
int bme280_appinit(void);

// soft reset
int bme280_reset(void);

// get T/P/H
// t in Deg centigrade
// p in Pa
// h in %RH
int bme280_gettph(float* t, float* p, float* h);

// t in Deg centigrade
int bme280_gett(float* t);

// p in Pa
int bme280_getp(float* p);

// h in %RH
int bme280_geth(float* h);


// T/P/H oversampling:
// 	0: off, 1: x1, 2: x2, 3: x4, 4: x8, 5: x16
int bme280_settos(int os);
int bme280_setpos(int os);
int bme280_sethos(int os);

// power mode:
// 0: sleep, 1/2: forced, 3: normal
int bme280_setmode(int mode);

// filter:
// 0: off, 1: 2x, 2: 4x, 3: 8x, 4: 16x
int bme280_setfilter(int filter);

// standby duration:
// 0: 0.5ms, 1: 62.5ms, ... 7: 20ms
int bme280_settsb(int tsb);

// co2 in ppm
// resistance in ohm
// tvoc in ppb
int iaq_getall(int* co2, int* resistance, int* tvoc);

#endif

