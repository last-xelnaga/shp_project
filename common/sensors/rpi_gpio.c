
#include "rpi_gpio.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// rpi2 range check taken from http://sourceforge.net/p/raspberry-gpio-python/
// more in "General Purpose I/O (GPIO)"

#define GPIO_BASE_OFFSET        0x200000
#define GPIO_LENGTH             4096

static unsigned int* p_gpio_base = NULL;

static int is_valid_rpi (
        void)
{
    int result = 1;

    // open cpuinfo
    
    
    // check revision
 
    return result;
}

int rpi_gpio_init (
        void)
{
    if (p_gpio_base != NULL)
        return 0;

    if (!is_valid_rpi ())
    {
        DEBUG_LOG_ERROR ("this board in not suitable for that app");
        return -1;
    }

    // peripheral addresses from device tree
    FILE* fp = fopen ("/proc/device-tree/soc/ranges", "rb");
    if (fp == NULL)
    {
        DEBUG_LOG_ERROR ("failed to open ranges %s", strerror (errno));
        return -1;
    }

    fseek (fp, 4, SEEK_SET);

    unsigned char p_range [4];
    if (fread (p_range, 1, sizeof (p_range), fp) != sizeof (p_range))
    {
        DEBUG_LOG_ERROR ("failed to read ranges %s", strerror (errno));
        fclose (fp);
        return -1;
    }
    fclose (fp);

    int fd = open ("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd == -1)
    {
        DEBUG_LOG_ERROR ("failed to open gpiomem %s", strerror (errno));
        return -1;
    }

    unsigned int range = p_range [0]; range <<= 8;
    range += p_range [1]; range <<= 8;
    range += p_range [2]; range <<= 8;
    range += p_range [3];

    unsigned int gpio_base = range + GPIO_BASE_OFFSET;
    p_gpio_base = (unsigned int*) mmap (NULL, GPIO_LENGTH,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base);
    close (fd);

    if (p_gpio_base == MAP_FAILED)
    {
        DEBUG_LOG_ERROR ("failed to map gpiomem %s", strerror (errno));
        p_gpio_base = NULL;
        return -1;
    }

    return 0;
}

void set_pin_direction (
        const int gpio_number,
        const int direction)
{
    switch (direction)
    {
        case INPUT:
            // set three zeros "000" as an input
            *(p_gpio_base + ((gpio_number) / 10)) &= ~(7 << (((gpio_number) % 10) * 3));
            break;

        case OUTPUT:
            // check that 'input bits" are emty
            *(p_gpio_base + ((gpio_number) / 10)) &= ~(7 << (((gpio_number) % 10) * 3));
            // set lowest bit to indicate the output
            *(p_gpio_base + ((gpio_number) / 10)) |=  (1 << (((gpio_number) % 10) * 3));
            break;
        default:
            break;
    }
}

void set_pin_voltage (
        const int gpio_number,
        const int level)
{
    switch (level)
    {
        case LOW:
            *(p_gpio_base + 10) = 1 << gpio_number;
            break;
        case HIGH:
            *(p_gpio_base + 7) = 1 << gpio_number;
            break;
        default:
            break;
    }
}

unsigned int get_bus_state (
        const int gpio_number)
{
    return *(p_gpio_base + 13) & (1 << gpio_number);
}
