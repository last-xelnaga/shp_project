
#ifndef RPI_SPI_H
#define RPI_SPI_H


#ifdef __cplusplus
extern "C" {
#endif

#define	INPUT               0
#define	OUTPUT              1

#define	LOW                 0
#define	HIGH                1


int spi_init (
        void);

//void set_pin_direction (
//        const int gpio_number,
//        const int direction);

//void set_pin_voltage (
//        const int gpio_number,
//        const int level);

//unsigned int get_bus_state (
//        const int gpio_number);

#ifdef __cplusplus
}
#endif

#endif // RPI_SPI_H
