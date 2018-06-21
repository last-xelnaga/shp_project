
#include "rpi_spi.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


/*! This means pin HIGH, true, 3.3volts on a pin. */
#define HIGH 0x1
/*! This means pin LOW, false, 0volts on a pin. */
#define LOW  0x0

// Peripherals block base address on RPi 1
#define BCM2835_PERI_BASE               0x20000000
// Base Address of the SPI0 registers
#define BCM2835_SPI0_BASE               0x204000
// Size of the peripherals block on RPi 1
#define BCM2835_PERI_SIZE               0x01000000

// Base Address of the GPIO registers
#define BCM2835_GPIO_BASE               0x200000

unsigned int* bcm2835_spi0;
unsigned int* bcm2835_gpio;

void bcm2835_init ()
{
    int  memfd;
    /* Open the master /dev/mem device */
    if ((memfd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
    {
        fprintf(stderr, "bcm2835_init: Unable to open /dev/mem: %s\n", strerror(errno)) ;
        return;
    }

    unsigned int* bcm2835_peripherals_base = (unsigned int*)BCM2835_PERI_BASE;
    unsigned int bcm2835_peripherals_size = BCM2835_PERI_SIZE;

    // Base of the peripherals block is mapped to VM
    unsigned int* bcm2835_peripherals = (unsigned int*) mmap (NULL, bcm2835_peripherals_size, (PROT_READ | PROT_WRITE), MAP_SHARED, memfd, (unsigned int)bcm2835_peripherals_base);
    bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE / 4;
    bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE / 4;
}

/* Write with memory barriers to peripheral
 */
void bcm2835_peri_write(volatile unsigned int* paddr, unsigned int value)
{
    __sync_synchronize();
    *paddr = value;
    __sync_synchronize();
}

/* write to peripheral without the write barrier */
void bcm2835_peri_write_nb(volatile unsigned int* paddr, unsigned int value)
{
	*paddr = value;
}

/* Read with memory barriers from peripheral
 *
 */
unsigned int bcm2835_peri_read(volatile unsigned int* paddr)
{
    unsigned int ret;
   __sync_synchronize();
   ret = *paddr;
   __sync_synchronize();
   return ret;
}

/* read from peripheral without the read barrier
 * This can only be used if more reads to THE SAME peripheral
 * will follow.  The sequence must terminate with memory barrier
 * before any read or write to another peripheral can occur.
 * The MB can be explicit, or one of the barrier read/write calls.
 */
unsigned int bcm2835_peri_read_nb(volatile unsigned int* paddr)
{
	return *paddr;
}


#define RPI_GPIO_P1_26  7  // Version 1, Pin P1-26, CE1 when SPI0 in use
#define RPI_GPIO_P1_24  8  // Version 1, Pin P1-24, CE0 when SPI0 in use
#define RPI_GPIO_P1_21  9  // Version 1, Pin P1-21, MISO when SPI0 in use
#define RPI_GPIO_P1_19  10  // Version 1, Pin P1-19, MOSI when SPI0 in use
#define RPI_GPIO_P1_23  11  // Version 1, Pin P1-23, CLK when SPI0 in use

#define BCM2835_GPIO_FSEL_ALT0  0x04 // Alternate function 0 0b100


void bcm2835_peri_set_bits(volatile unsigned int* paddr, unsigned int value, unsigned int mask)
{
    unsigned int v = bcm2835_peri_read(paddr);
    v = (v & ~mask) | (value & mask);
    bcm2835_peri_write(paddr, v);
}

/*! GPIO register offsets from BCM2835_GPIO_BASE.
  Offsets into the GPIO Peripheral block in bytes per 6.1 Register View
*/
#define BCM2835_GPFSEL0                      0x0000 /*!< GPIO Function Select 0 */

#define BCM2835_GPIO_FSEL_MASK  0x07    /*!< Function select bits mask 0b111 */

void bcm2835_gpio_fsel(unsigned char pin, unsigned char mode)
{
    /* Function selects are 10 pins per 32 bit word, 3 bits per pin */
    volatile unsigned int* paddr = bcm2835_gpio + BCM2835_GPFSEL0 / 4 + (pin/10);
    unsigned char   shift = (pin % 10) * 3;
    unsigned int  mask = BCM2835_GPIO_FSEL_MASK << shift;
    unsigned int  value = mode << shift;
    bcm2835_peri_set_bits(paddr, value, mask);
}

#define BCM2835_SPI0_CS                      0x0000 /*!< SPI Master Control and Status */
#define BCM2835_SPI0_CLK                     0x0008 /*!< SPI Master Clock Divider */
#define BCM2835_SPI0_FIFO                    0x0004 /*!< SPI Master TX and RX FIFOs */

#define BCM2835_SPI0_CS_CLEAR                0x00000030 /*!< Clear FIFO Clear RX and TX */
#define BCM2835_SPI0_CS_CPOL                 0x00000008 /*!< Clock Polarity */
#define BCM2835_SPI0_CS_CPHA                 0x00000004 /*!< Clock Phase */
#define BCM2835_SPI0_CS_CS                   0x00000003 /*!< Chip Select */
#define BCM2835_SPI0_CS_TA                   0x00000080 /*!< Transfer Active */
#define BCM2835_SPI0_CS_TXD                  0x00040000 /*!< TXD TX FIFO can accept Data */
#define BCM2835_SPI0_CS_RXD                  0x00020000 /*!< RXD RX FIFO contains Data */
#define BCM2835_SPI0_CS_DONE                 0x00010000 /*!< Done transfer Done */

int bcm2835_spi_begin(void)
{
    volatile unsigned int* paddr;

    // Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK

    // Set the SPI CS register to the some sensible defaults
    paddr = bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    bcm2835_peri_write(paddr, 0); // All 0s

    // Clear TX and RX fifos
    bcm2835_peri_write_nb(paddr, BCM2835_SPI0_CS_CLEAR);

    return 1; // OK
}

void bcm2835_spi_setDataMode(unsigned char mode)
{
    volatile unsigned int* paddr = bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    // Mask in the CPO and CPHA bits of CS
    bcm2835_peri_set_bits(paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

// defaults to 0, which means a divider of 65536.
// The divisor must be a power of 2. Odd numbers
// rounded down. The maximum SPI clock rate is
// of the APB clock
void bcm2835_spi_setClockDivider(unsigned short divider)
{
    volatile unsigned int* paddr = bcm2835_spi0 + BCM2835_SPI0_CLK / 4;
    bcm2835_peri_write(paddr, divider);
}

void bcm2835_spi_chipSelect(unsigned char cs)
{
    volatile unsigned int* paddr = bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    // Mask in the CS bits of CS
    bcm2835_peri_set_bits(paddr, cs, BCM2835_SPI0_CS_CS);
}

void bcm2835_spi_setChipSelectPolarity(unsigned char cs, unsigned char active)
{
    volatile unsigned int* paddr = bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    unsigned char shift = 21 + cs;
    // Mask in the appropriate CSPOLn bit
    bcm2835_peri_set_bits(paddr, active << shift, 1 << shift);
}

#define BCM2835_SPI_MODE0 0  /*!< CPOL = 0, CPHA = 0 */
#define BCM2835_SPI_CLOCK_DIVIDER_256   256     /*!< 256 = 976.5625kHz on Rpi2, 1.5625MHz on RPI3 */
#define BCM2835_SPI_CS0 0     /*!< Chip Select 0 */


int rpi_spi_init (
        void)
{
    bcm2835_init ();
    bcm2835_spi_begin ();
    bcm2835_spi_setDataMode (BCM2835_SPI_MODE0); //Data comes in on falling edge
    bcm2835_spi_setClockDivider (BCM2835_SPI_CLOCK_DIVIDER_256); //250MHz / 256 = 976.5kHz
    bcm2835_spi_chipSelect (BCM2835_SPI_CS0); //Slave Select on CS0
    bcm2835_spi_setChipSelectPolarity (BCM2835_SPI_CS0, LOW);

    return 0;
}

/* Writes (and reads) an number of bytes to SPI */
void bcm2835_spi_transfernb (char* tbuf, char* rbuf, unsigned int len)
{
    volatile unsigned int* paddr = bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    volatile unsigned int* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO / 4;
    unsigned int TXCnt = 0;
    unsigned int RXCnt = 0;

    /* This is Polled transfer as per section 10.6.1
    // BUG ALERT: what happens if we get interupted in this section, and someone else
    // accesses a different peripheral?
    */

    /* Clear TX and RX fifos */
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    /* Set TA = 1 */
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    /* Use the FIFO's to reduce the interbyte times */
    while((TXCnt < len)||(RXCnt < len))
    {
        /* TX fifo not full, so add some more bytes */
        while(((bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))&&(TXCnt < len ))
        {
           bcm2835_peri_write_nb(fifo, tbuf[TXCnt]);
           TXCnt++;
        }
        /* Rx fifo not empty, so get the next received bytes */
        while(((bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD))&&( RXCnt < len ))
        {
           rbuf[RXCnt] = bcm2835_peri_read_nb(fifo);
           RXCnt++;
           //printf ("RXCnt [%d] %d\n", RXCnt, rbuf[RXCnt]);
        }
    }
    /* Wait for DONE to be set */
    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
	;

    /* Set TA = 0, and also set the barrier */
    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}
