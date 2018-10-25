
#include "log.h"
#include "rpi_gpio.h"
#include "sensor_dht22.h"
#include "time_utils.h"

// dth output data size
#define DATA_LENGTH                 5

// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT                32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES                  41


unsigned char p_data [DATA_LENGTH];

static void read_data (
        const unsigned int gpio_num)
{
    // Store the count that each DHT bit pulse is low and high.
    // Make sure array is initialized to start at zero.
    unsigned int pulseCounts [DHT_PULSES * 2] = { 0 };

    set_pin_direction (gpio_num, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    set_pin_voltage (gpio_num, HIGH);

    sleep_milliseconds (20);

    // send 'start' command
    // pull the bus down
    set_pin_voltage (gpio_num, LOW);
    // and wait at least 18ms
    sleep_milliseconds (20);

    // at this point the sensor should start to react
    // on our 'start' command
    set_pin_direction (gpio_num, INPUT);

    // Need a very short delay before reading pins or else value is sometimes still low.
    for (volatile int i = 0; i < 50; ++i) {
    }

    do {

        // sensor has to lower the bus, so wait for it
        unsigned int count = 0;
        while (get_bus_state (gpio_num))
        {
          if (++count >= DHT_MAXCOUNT)
          {
            // Timeout waiting for response.
            break;
          }
        }

        // Record pulse widths for the expected result bits.
        for (int i = 0; i < DHT_PULSES * 2; i += 2)
        {
          // Count how long pin is low and store in pulseCounts[i]
          while (!get_bus_state (gpio_num))
          {
            if (++pulseCounts[i] >= DHT_MAXCOUNT)
            {
              // Timeout waiting for response.
              break;
            }
          }
          // Count how long pin is high and store in pulseCounts[i+1]
          while (get_bus_state (gpio_num))
          {
            if (++pulseCounts[i+1] >= DHT_MAXCOUNT)
            {
              // Timeout waiting for response.
              break;
            }
          }
        }
    } while (0);

    set_pin_direction (gpio_num, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    set_pin_voltage (gpio_num, HIGH);

    // Compute the average low pulse width to use as a 50 microsecond reference threshold.
    // Ignore the first two readings because they are a constant 80 microsecond pulse.
    unsigned int threshold = 0;
    for (int i = 2; i < DHT_PULSES * 2; i += 2)
    {
      threshold += pulseCounts [i];
    }
    threshold /= DHT_PULSES - 1;

    // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
    // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
    // then it must be a ~70us 1 pulse.
    for (int i = 3; i < DHT_PULSES * 2; i += 2)
    {
      int index = (i - 3) / 16;
      p_data [index] <<= 1;
      if (pulseCounts [i] >= threshold)
      {
        // One bit for long pulse.
        p_data [index] |= 1;
      }
      // Else zero bit for short pulse.
    }
}

static int is_data_crc_valid (
        void)
{
    int result = 1;

    // check the crc of the data buffer
    unsigned char crc = p_data [0] + p_data [1] + p_data [2] + p_data [3];
    if (crc != p_data [4])
        result = 0;

    return result;
}

int dht22_get_data (
        const unsigned int gpio_num,
        unsigned int* humidity,
        int* temperature)
{
    for (int i = 0; i < 4; ++ i)
        p_data [i] = 0;
    p_data [4] = 1;

    read_data (gpio_num);
    if (!is_data_crc_valid ())
    {
        LOG_ERROR ("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x, crc FAILED",
                p_data [0], p_data [1], p_data [2], p_data [3], p_data [4]);
        return -1;
    }

    *humidity = p_data [0];
    *humidity <<= 8;
    *humidity += p_data [1];

    *temperature = p_data [2] & 0x7F;
    *temperature <<= 8;
    *temperature += p_data [3];

    // negative temperature
    if (p_data [2] & 0x80)
        *temperature *= -1;

    return 0;
}
