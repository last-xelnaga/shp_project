
#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_WIRINGPI_LIB
#include <wiringPi.h>
#endif // USE_WIRINGPI_LIB


int board_setup (
        void);

void dht_setup (
        void);

void dht_get_data (
        int* temperature,
        int* humidity);


void liquid_level_setup (
        void);

int get_liquid_level (
        void);


// water pump
void water_pump_setup (
        void);

void water_pump_start (
        void);

void water_pump_stop (
        void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
