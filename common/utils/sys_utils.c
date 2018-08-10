
#include "sys_utils.h"
#include <sched.h>
#include <string.h>


void set_app_priority (
        const int priority)
{
    struct sched_param sched;
    memset (&sched, 0, sizeof (sched));

    switch (priority)
    {
        case PRIORITY_MAX:
        #ifdef RPI_TARGET
            // Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
            sched.sched_priority = sched_get_priority_max (SCHED_FIFO);
            sched_setscheduler (0, SCHED_FIFO, &sched);
        #endif // RPI_TARGET
            break;
        case PRIORITY_DEFAULT:
        #ifdef RPI_TARGET
            // Go back to default scheduler with default 0 priority.
            sched.sched_priority = 0;
            sched_setscheduler (0, SCHED_OTHER, &sched);
        #endif // RPI_TARGET
            break;
        default:
            break;
    }
}
