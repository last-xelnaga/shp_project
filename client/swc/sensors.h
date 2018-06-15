
#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif


int raspberry_board_setup (
        void);


/*void sensor_dht11_setup (
        void);

void sensor_dht11_get_data (
        int* temperature,
        int* humidity);*/


void liquid_level_setup (
        void);

int get_liquid_level (
        void);


// water pump
void sensor_relay_water_pump_setup (
        void);

void sensor_relay_water_pump_start (
        void);

void sensor_relay_water_pump_stop (
        void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
