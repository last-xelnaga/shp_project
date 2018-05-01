
#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

int get_temperature (
        void);

int get_humidity (
        void);

int get_liquid_level (
        void);

int water_pump_start (
        void);

void water_pump_stop (
        void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_H
