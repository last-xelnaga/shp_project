
#include "log.h"
#include "sensors.h"

#include <unistd.h>


// pump relay gpio
#define SENSOR_RELAY_WATER_PUMP_GPIO    4

// dht11 temperature and humidity sensor gpio
#define SENSOR_DHT11_GPIO       6
// retry count for the read_data process
#define RETRY_COUNT             3

// dth output data size
#define DATA_LENGTH             5
// init macro
#define INIT_DATA               p_data[0]=0;p_data[1]=0;p_data[2]=0;p_data[3]=0;p_data[4]=0;

unsigned char p_data [DATA_LENGTH];

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
        if (wiringPiSetup () == -1)
        {
            DEBUG_LOG_ERROR ("wiringPiSetup has failed");
            result = -1;
        }
    }

    if (result == 0)
        wiringPiSetupGpio ();
#endif // #ifdef USE_WIRINGPI_LIB

    return result;
}


// temperature and humidity sensor
#ifdef USE_WIRINGPI_LIB
static void read_data (
        void)
{
    // send 'start' command
    // pull the bus down
    digitalWrite (SENSOR_DHT11_GPIO, LOW);
    // and wait at least 18ms
    delayMicroseconds (18000);

    // now pull the bus up
    digitalWrite (SENSOR_DHT11_GPIO, HIGH);

    // and wait for response (20 - 40 us)
    delayMicroseconds (20);

    // at this point the sensor should start to react
    // on our 'start' command
    pinMode (SENSOR_DHT11_GPIO, INPUT);


    // sensor has to lower the bus, so wait for it
    //while (digitalRead (SENSOR_DHT11_GPIO) == HIGH);
    unsigned long loop_count = 1000000;
    while (digitalRead (SENSOR_DHT11_GPIO) == HIGH)
    {
        if (-- loop_count == 0)
            return;
    }

    // he has to keep low level for 80 us
    delayMicroseconds (70);

    // and than put it high again. wait for it as well
    //while (digitalRead (SENSOR_DHT11_GPIO) == LOW);
    loop_count = 1000000;
    while (digitalRead (SENSOR_DHT11_GPIO) == LOW)
    {
        if (-- loop_count == 0)
            return;
    }

    // it will keep hight level for 80 us, so skip them
    //delayMicroseconds (50);

    DEBUG_LOG_INFO ("read dht bits");

    // and it's ready to send 40 bits of data
    /*for (unsigned int i = 0; i < DATA_LENGTH; ++ i)
    {
        // receive temperature and humidity data, the parity bit is not considered
        unsigned char byte = 0;
        for (int bit = 0; bit < 8; ++ bit)
        {
            // wait for low bus. indicates that the transmission has started
            while (digitalRead (SENSOR_DHT11_GPIO) == HIGH);

            unsigned long t = micros ();

            // wait for 50 us of the low level. skip it
            while (digitalRead (SENSOR_DHT11_GPIO) == LOW);

            // now the sensor has switched the bus to high level
            // wait for 30 us to check the bus level again
            //delayMicroseconds (28);

            // check the bus again
            //if (digitalRead (SENSOR_DHT11_GPIO) == HIGH)
            if ((micros () - t) > 40)
            {
                // high bus after 30 ms means that we got '1',
                // since '1' requires 70 ms of high bus.
                // '0' should be finished within 26-28 ms.
                byte |= (1 << (7 - bit));
            }
        }
        p_data [i] = byte;
    }*/

    unsigned char mask = 128;
    unsigned char idx = 0;

    for (unsigned int i = 0; i < 40; ++ i)
    {
        loop_count = 1000000;
        while (digitalRead (SENSOR_DHT11_GPIO) == HIGH)
        {
            //delayMicroseconds (5);
            if (-- loop_count == 0)
                return;
        }
        // wait for 50 us of the low level
        delayMicroseconds (40);

        loop_count = 1000000;
        while (digitalRead (SENSOR_DHT11_GPIO) == LOW)
        {
            //delayMicroseconds (5);
            if (-- loop_count == 0)
                return;
        }

        unsigned long t = micros ();

        loop_count = 1000000;
        while (digitalRead (SENSOR_DHT11_GPIO) == HIGH)
        {
            //delayMicroseconds (10);
            if (-- loop_count == 0)
                return;
        }

        if ((micros () - t) > 40)
        {
            p_data [idx] |= mask;
        }
        mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx ++;
        }
    }
}

static int is_data_crc_valid (
        void)
{
    int result = 1;

    // check the crc of the data buffer
    unsigned char crc = p_data [0] + p_data [1] + p_data [2] + p_data [3];
    if (crc != p_data [4])
    {
        result = 0;
        DEBUG_LOG_ERROR ("crc FAILED, hum %d %d, temp %d %d, crc %d (%d)",
                p_data [0], p_data [1], p_data [2], p_data [3], p_data [4], crc);
    }
    else
    {
        DEBUG_LOG_INFO ("crc OK, hum %d %d, temp %d %d, crc %d (%d)",
                p_data [0], p_data [1], p_data [2], p_data [3], p_data [4], crc);
    }

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
    pinMode (SENSOR_DHT11_GPIO, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    digitalWrite (SENSOR_DHT11_GPIO, HIGH);
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
    *humidity *= .1;

    *temperature = p_data [2] & 0x7F;
    *temperature <<= 8;
    *temperature += p_data [3];
    *temperature *= .1;

    // negative temperature
    if (p_data [2] & 0x80)
        *temperature *= -1;
}


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

    return 55;
}


// water pump operated by relay
void sensor_relay_water_pump_setup (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_setup");
#ifdef USE_WIRINGPI_LIB
    pinMode (SENSOR_RELAY_WATER_PUMP_GPIO, OUTPUT);
    digitalWrite (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_relay_water_pump_start (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_start");
#ifdef USE_WIRINGPI_LIB
    digitalWrite (SENSOR_RELAY_WATER_PUMP_GPIO, HIGH);
#endif // #ifdef USE_WIRINGPI_LIB
}

void sensor_relay_water_pump_stop (
        void)
{
    DEBUG_LOG_INFO ("sensor_relay_water_pump_stop");
#ifdef USE_WIRINGPI_LIB
    digitalWrite (SENSOR_RELAY_WATER_PUMP_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}
