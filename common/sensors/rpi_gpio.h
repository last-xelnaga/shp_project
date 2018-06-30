
#ifndef RPI_GPI_H
#define RPI_GPI_H


#ifdef __cplusplus
extern "C" {
#endif

int rpi_gpio_init (
        void);

#define	INPUT               0
#define	OUTPUT              1

void set_pin_direction (
        const int gpio_number,
        const int direction);

#define	LOW                 0
#define	HIGH                1

void set_pin_voltage (
        const int gpio_number,
        const int level);

unsigned int get_bus_state (
        const int gpio_number);

#ifdef __cplusplus
}
#endif

#endif // ifndef RPI_GPI_H
