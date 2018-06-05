
#include "time_utils.h"

#include <sys/time.h>


//TODO migrate to clock_nanosleep
void sleep_milliseconds (
        unsigned int millis)
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
