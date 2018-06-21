
#ifndef RPI_SPI_H
#define RPI_SPI_H


#ifdef __cplusplus
extern "C" {
#endif

int rpi_spi_init (
        void);

void bcm2835_spi_transfernb (char* tbuf, char* rbuf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif // RPI_SPI_H
