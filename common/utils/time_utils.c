
#include "time_utils.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>


//TODO migrate to clock_nanosleep
void sleep_milliseconds (
        const unsigned int millis)
{
    struct timeval tv ;
    gettimeofday (&tv, 0) ;
    unsigned long need  = (unsigned long)tv.tv_sec * (unsigned long)1000 + (unsigned long)(tv.tv_usec / 1000) + millis;

    unsigned long now;
    do
    {
        gettimeofday (&tv, 0) ;
        now  = (unsigned long)tv.tv_sec * (unsigned long)1000 + (unsigned long)(tv.tv_usec / 1000);
    } while (now < need);
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
