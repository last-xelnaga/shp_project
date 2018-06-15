
#include "log.h"
#include "sensors.h"
#include "rpi_gpio.h"
#include "sys_utils.h"
#include "time_utils.h"

//#include <mcp3004.h>
#include <unistd.h>


//#define BASE 100
//#define SPI_CHAN 0
//#define AN_CHAN_DAC 0
//#define FREQUENCE 150000

// pump relay gpio
#define SENSOR_RELAY_WATER_PUMP_GPIO    18 //4

// dht11 temperature and humidity sensor gpio
#define SENSOR_DHT11_GPIO       4 //6
// retry count for the read_data process
#define RETRY_COUNT             3

// dth output data size
//#define DATA_LENGTH             5
// init macro
//#define INIT_DATA               p_data[0]=0;p_data[1]=0;p_data[2]=0;p_data[3]=0;p_data[4]=0;

//unsigned char p_data [DATA_LENGTH];

// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
//#define DHT_MAXCOUNT 32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
//#define DHT_PULSES 41


// wiringPi setup
int raspberry_board_setup (
        void)
{
    int result = 0;

#ifdef USE_WIRINGPI_LIB
    if (geteuid () != 0)
    {
        DEBUG_LOG_ERROR ("need to be root to run");
        result = -1;
    }

    if (result == 0)
    {
        if (rpi_gpio_init () == -1)
        {
            DEBUG_LOG_ERROR ("gpio_base_init has failed");
            result = -1;
        }
    }

#endif // #ifdef USE_WIRINGPI_LIB

    return result;
}
/*
// temperature and humidity sensor
#ifdef USE_WIRINGPI_LIB
static void read_data (
        void)
{
    // Store the count that each DHT bit pulse is low and high.
    // Make sure array is initialized to start at zero.
    unsigned int pulseCounts [DHT_PULSES * 2] = { 0 };

    // send 'start' command
    // pull the bus down
    set_pin_voltage (SENSOR_DHT11_GPIO, LOW);
    // and wait at least 18ms
    sleep_milliseconds(20);

    // now pull the bus up
    //set_pin_voltage (SENSOR_DHT11_GPIO, HIGH);

    // and wait for response (20 - 40 us)
    //sleep_milliseconds (20);

    // at this point the sensor should start to react
    // on our 'start' command
    set_pin_direction (SENSOR_DHT11_GPIO, INPUT);

    // Need a very short delay before reading pins or else value is sometimes still low.
    for (volatile int i = 0; i < 50; ++i) {
    }

    do {

        // sensor has to lower the bus, so wait for it
        unsigned int count = 0;
        while (get_bus_state(SENSOR_DHT11_GPIO)) {
          if (++count >= DHT_MAXCOUNT) {
            // Timeout waiting for response.

            break;
          }
        }

        // Record pulse widths for the expected result bits.
        for (int i=0; i < DHT_PULSES*2; i+=2) {
          // Count how long pin is low and store in pulseCounts[i]
          while (!get_bus_state(SENSOR_DHT11_GPIO)) {
            if (++pulseCounts[i] >= DHT_MAXCOUNT) {
              // Timeout waiting for response.
              //set_app_priority (PRIORITY_DEFAULT);
              break;
            }
          }
          // Count how long pin is high and store in pulseCounts[i+1]
          while (get_bus_state(SENSOR_DHT11_GPIO)) {
            if (++pulseCounts[i+1] >= DHT_MAXCOUNT) {
              // Timeout waiting for response.
              //set_app_priority (PRIORITY_DEFAULT);
              break;
            }
          }
        }
    } while (0);

    set_app_priority (PRIORITY_DEFAULT);

    // Compute the average low pulse width to use as a 50 microsecond reference threshold.
    // Ignore the first two readings because they are a constant 80 microsecond pulse.
    unsigned int threshold = 0;
    for (int i=2; i < DHT_PULSES*2; i+=2) {
      threshold += pulseCounts[i];
    }
    threshold /= DHT_PULSES-1;

    // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
    // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
    // then it must be a ~70us 1 pulse.
    //unsigned char data[5] = {0};
    for (int i=3; i < DHT_PULSES*2; i+=2) {
      int index = (i-3)/16;
      p_data [index] <<= 1;
      if (pulseCounts[i] >= threshold) {
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

    DEBUG_LOG_INFO ("0x%x 0x%x 0x%x 0x%x 0x%x, crc %s",
            p_data [0], p_data [1], p_data [2], p_data [3], p_data [4],
            result == 1 ? "OK" : "FAILED");

    return result;
}
#endif // #ifdef USE_WIRINGPI_LIB

static void do_read (
        void)
{
    INIT_DATA

#ifdef USE_WIRINGPI_LIB
    int retry_count = RETRY_COUNT;

    while (1)
    {
        read_data ();

        sensor_dht11_setup ();

        //is_data_crc_valid ();
        if (is_data_crc_valid ())
            break;

        INIT_DATA

        if (-- retry_count < 0)
            break;

        sleep (1);
    }
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_dht11_setup (
        void)
{
    DEBUG_LOG_INFO ("sensor_dht11_setup");
#ifdef USE_WIRINGPI_LIB
    set_pin_direction (SENSOR_DHT11_GPIO, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    set_pin_voltage (SENSOR_DHT11_GPIO, HIGH);
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_dht11_get_data (
        int* temperature,
        int* humidity)
{
    DEBUG_LOG_INFO ("sensor_dht11_get_data");

    do_read ();

    *humidity = p_data [0];
    *humidity <<= 8;
    *humidity += p_data [1];

    *temperature = p_data [2] & 0x7F;
    *temperature <<= 8;
    *temperature += p_data [3];

    // negative temperature
    if (p_data [2] & 0x80)
        *temperature *= -1;
}
*/
// liquid_level sensor
void liquid_level_setup (
        void)
{
    DEBUG_LOG_INFO ("liquid_level_setup");
}

int get_liquid_level (
        void)
{
    DEBUG_LOG_INFO ("get_liquid_level");
    return 0;
}


// water pump operated by relay
void sensor_relay_water_pump_setup (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_setup");
#ifdef USE_WIRINGPI_LIB
    set_pin_direction (SENSOR_RELAY_WATER_PUMP_GPIO, OUTPUT);
    set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_relay_water_pump_start (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_start");
#ifdef USE_WIRINGPI_LIB
    set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_relay_water_pump_stop (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_stop");
#ifdef USE_WIRINGPI_LIB
    set_pin_voltage (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}
