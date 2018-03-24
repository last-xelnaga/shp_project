
#include <stdlib.h>
#include <stdio.h>

int send_notification (
    void)
{
    int status = system("python messaging.py huj pizda");
    printf ("system result %d\n", status);

    return 0;
}
