
#ifndef SENSOR_DHT22_H
#define SENSOR_DHT22_H


#ifdef __cplusplus
extern "C" {
#endif

int dht22_get_data (
        const unsigned int gpio_num,
        unsigned int* humidity,
        int* temperature);

#ifdef __cplusplus
}
#endif

#endif // ifndef SENSOR_DHT22_H
