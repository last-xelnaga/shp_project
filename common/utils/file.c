
#include "file.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


int save_to_file (
        const char* file_name,
        const void* data,
        const unsigned int size)
{
    FILE* f = fopen (file_name, "wb");
    if (f != NULL)
    {
        fwrite (data, size, 1, f);
        fclose (f);
    }

    return 0;
}

int read_from_file (
        const char* file_name,
        void** data,
        unsigned int* size)
{
    FILE* f = fopen (file_name, "rb");
    if (f != NULL)
    {
        fseek (f, 0, SEEK_END);
        *size = ftell (f);
        fseek (f, 0, SEEK_SET);

        *data = (char*) malloc (*size);
        fread (*data, *size, 1, f);
        fclose (f);
    }

    return 0;
}

void drop_file (
        const char* file_name)
{
    unlink (file_name);
    does_file_exist (file_name);
}

void drop_old_file (
        const char* file_name,
        unsigned int exp_time)
{
    struct stat buffer;
    if (stat (file_name, &buffer) == 0)
    {
        time_t now = time (NULL);
        if ((unsigned long)now > (unsigned long)(buffer.st_ctime + exp_time))
        {
            DEBUG_LOG_INFO ("file \"%s\" is old", file_name);
            unlink (file_name);
        }
    }
}

int does_file_exist (
        const char* file_name)
{
    int result = 1;

    struct stat buffer;
    if (stat (file_name, &buffer) != 0)
    {
        DEBUG_LOG_ERROR ("file \"%s\" does not exist", file_name);
        result = 0;
    }

    return result;
}
