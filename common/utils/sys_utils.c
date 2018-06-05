
#include "sys_utils.h"
#include <sched.h>
#include <string.h>


void set_app_priority (
        const int priority)
{
    struct sched_param sched;
    memset(&sched, 0, sizeof(sched));

    switch (priority)
    {
        case PRIORITY_MAX:
            // Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
            sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
            sched_setscheduler(0, SCHED_FIFO, &sched);
            break;
        case PRIORITY_DEFAULT:
            // Go back to default scheduler with default 0 priority.
            sched.sched_priority = 0;
            sched_setscheduler(0, SCHED_OTHER, &sched);
            break;
        default:
            break;
    }
}
