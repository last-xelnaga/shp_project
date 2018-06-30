
// http://www.hertaville.com/interfacing-an-spi-adc-mcp3008-chip-to-the-raspberry-pi-using-c.html
// http://www.airspayce.com/mikem/bcm2835/index.html
// https://github.com/MartinMiller/mcp3008lib-spi

#ifndef RPI_SPI_H
#define RPI_SPI_H


#ifdef __cplusplus
extern "C" {
#endif

int rpi_spi_init (
        void);

int rpi_spi_mcp3008_read (
        const unsigned char channel);

#ifdef __cplusplus
}
#endif

#endif // ifndef RPI_SPI_H
