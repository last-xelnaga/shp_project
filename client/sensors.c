
#include "log.h"
#include "sensors.h"

// pump relay gpio
#define PUMP_RELAY_GPIO         4

// dht11 temperature and humidity sensor gpio
#define DHT_TERMO_GPIO          6
// retry count for the read_data process
#define RETRY_COUNT             3

// dth output data size
#define DATA_LENGTH             5
// init macro
#define INIT_DATA               p_data[0]=0;p_data[1]=0;p_data[2]=0;p_data[3]=0;p_data[4]=0;
unsigned char p_data [DATA_LENGTH];

// wiringPi setup
int board_setup (
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
    digitalWrite (DHT_TERMO_GPIO, LOW);
    // and wait at least 18ms
    delayMicroseconds (30000);

    // now pull the bus up
    digitalWrite (DHT_TERMO_GPIO, HIGH);

    // at this point the sensor should start to react
    // on our 'start' command
    pinMode (DHT_TERMO_GPIO, INPUT);

    // and wait for response (20 - 40 us)
    delayMicroseconds (20);

    // sensor has to lower the bus, so wait for it
    while (digitalRead (DHT_TERMO_GPIO) == HIGH);
    //wait_for_level (fp, LOW);
    // he has to keep low level for 80 us
    delayMicroseconds (80);

    // and than put it high again. wait for it as well
    while (digitalRead (DHT_TERMO_GPIO) == LOW);
    //wait_for_level (fp, HIGH);
    // it will keep hight level for 80 us, so skip them
    delayMicroseconds (80);

    // and it's ready to send 40 bits of data
    for (unsigned int i = 0; i < DATA_LENGTH; ++ i)
    {
        // receive temperature and humidity data, the parity bit is not considered
        unsigned char byte = 0;
        for (int bit = 0; bit < 8; ++ bit)
        {
            // wait for low bus. indicates that the transmission has started
            while (digitalRead (DHT_TERMO_GPIO) == HIGH);

            // wait for 50 us of the low level. skip it
            while (digitalRead (DHT_TERMO_GPIO) == LOW);

            // now the sensor has switched the bus to high level
            // wait for 30 us to check the bus level again
            delayMicroseconds (28);

            // check the bus again
            if (digitalRead (DHT_TERMO_GPIO) == HIGH)
            {
                // high bus after 30 ms means that we got '1',
                // since '1' requires 70 ms of high bus.
                // '0' should be finished within 26-28 ms.
                byte |= (1 << (7 - bit));
            }
        }
        p_data [i] = byte;
    }

    pinMode (DHT_TERMO_GPIO, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    digitalWrite (DHT_TERMO_GPIO, HIGH);
}

static int is_data_crc_valid (
        void)
{
    int result = 1;

    // check the crc of the data buffer
    unsigned int crc = p_data [0] + p_data [1] + p_data [2] + p_data [3];
    crc |= 0x0F;

    if (crc != p_data [4])
    {
        result = 0;
        DEBUG_LOG_ERROR ("%s temp %i.%i, hum %i.%i, crc %i", "dht11", p_data [0], p_data [1], p_data [2], p_data [3], p_data [4]);
    }
    else
    {
        DEBUG_LOG_INFO ("crc OK");
        DEBUG_LOG_INFO ("%d %d %d %d", p_data [0], p_data [1], p_data [2], p_data [3]);
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

        if (is_data_crc_valid ())
            break;

        INIT_DATA

        if (-- retry_count < 0)
            break;

        sleep (1);
    }
#endif // #ifdef USE_WIRINGPI_LIB
}

void dht_setup (
        void)
{
    DEBUG_LOG_INFO ("dht_setup");
#ifdef USE_WIRINGPI_LIB
    pinMode (DHT_TERMO_GPIO, OUTPUT);

    // pull the bus high. this one will show to sensor
    // that it could stay in low-power-consumption mode
    digitalWrite (DHT_TERMO_GPIO, HIGH);
#endif // #ifdef USE_WIRINGPI_LIB
}

void dht_get_data (
        int* temperature,
        int* humidity)
{
    DEBUG_LOG_INFO ("dht_get_data");

    do_read ();

    *humidity = p_data [0];
    *humidity <<= 8;
    *humidity += p_data [1];

    *temperature = p_data [2];
    *temperature <<= 8;
    *temperature += p_data [3];
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
void water_pump_setup (
        void)
{
    DEBUG_LOG_INFO ("water_pump_setup");
#ifdef USE_WIRINGPI_LIB
    pinMode (PUMP_RELAY_GPIO, OUTPUT);
    digitalWrite (PUMP_RELAY_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}

void water_pump_start (
        void)
{
    DEBUG_LOG_INFO ("water_pump_start");
#ifdef USE_WIRINGPI_LIB
    digitalWrite (PUMP_RELAY_GPIO, HIGH);
#endif // #ifdef USE_WIRINGPI_LIB
}

void water_pump_stop (
        void)
{
    DEBUG_LOG_INFO ("water_pump_stop");
#ifdef USE_WIRINGPI_LIB
    digitalWrite (PUMP_RELAY_GPIO, LOW);
#endif // #ifdef USE_WIRINGPI_LIB
}