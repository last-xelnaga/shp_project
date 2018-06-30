
#include "file_utils.h"
#include "log.h"

#include <errno.h>
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
    int result = 0;

    FILE* f = fopen (file_name, "wb");
    if (f == NULL)
    {
        LOG_INFO ("failed to open file \"%s\"", file_name);
        result = -1;
    }

    if (result == 0)
        fwrite (data, size, 1, f);

    if (f != NULL)
        fclose (f);

    return result;
}

int read_from_file (
        const char* file_name,
        void** data,
        unsigned int* size)
{
    int result = 0;
    
    FILE* f = fopen (file_name, "rb");
    if (f == NULL)
    {
        LOG_INFO ("failed to open file \"%s\"", file_name);
        result = -1;
    }

    if (result == 0)
    {
        fseek (f, 0, SEEK_END);
        *size = ftell (f);
        fseek (f, 0, SEEK_SET);
        //fseek (f, 0, SEEK_CUR);

        *data = (char*) malloc (*size);
        unsigned int processed = fread (*data, 1, *size, f);
        if (processed != *size)
        {
            LOG_INFO ("failed to read from file \"%s\" - %s", file_name, strerror (errno));
            LOG_INFO ("processed %d, needed %d", processed, *size);
            free (*data);
            result = -1;
        }
    }

    if (f != NULL)
        fclose (f);

    return result;
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
            LOG_INFO ("file \"%s\" is old", file_name);
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
        LOG_ERROR ("file \"%s\" does not exist", file_name);
        result = 0;
    }

    return result;
}
