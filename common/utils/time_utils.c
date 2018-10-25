
#include "time_utils.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef ESP_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif // ESP_TARGET


//TODO migrate to clock_nanosleep
void sleep_milliseconds (
        const unsigned int millis)
{
#ifdef RPI_TARGET
    struct timeval tv;
    gettimeofday (&tv, NULL);
    unsigned long need  = (unsigned long)tv.tv_sec * (unsigned long)1000 + (unsigned long)(tv.tv_usec / 1000) + millis;

    unsigned long now;
    do
    {
        gettimeofday (&tv, NULL);
        now  = (unsigned long)tv.tv_sec * (unsigned long)1000 + (unsigned long)(tv.tv_usec / 1000);
    } while (now < need);
#endif // RPI_TARGET

#ifdef ESP_TARGET
    vTaskDelay (millis / portTICK_PERIOD_MS);
#endif // ESP_TARGET
}

const char* get_time_str (
        void)
{
    time_t now = time (NULL);
    struct tm* tm = localtime (&now);

    static char mon_name [12][4] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    static char result [32];

    sprintf (result, "%4d %s %.2d %.2d:%.2d:%.2d",
        1900 + tm->tm_year,
        mon_name [tm->tm_mon],
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec);
    return result;
}

void set_curr_time (
        const unsigned long now)
{
    struct timeval tv;
    tv.tv_sec = now;
    tv.tv_usec = 0;

    struct timezone tz;
    tz.tz_minuteswest = 120; // GMT+2
    tz.tz_dsttime = 0;

    int result = settimeofday (&tv, &tz);
    if (result != 0)
    {
        printf("settimeofday() successful.\n");
    }
}
