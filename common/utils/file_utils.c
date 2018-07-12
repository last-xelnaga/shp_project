
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
    // set negative reply by default
    int result = -1;
    FILE* f = NULL;

    do
    {
        // nowhere to write
        if (file_name == NULL)
        {
            LOG_INFO ("file name is NULL");
            break;
        }

        // nothing to write
        if (data == NULL)
        {
            LOG_INFO ("data is NULL");
            break;
        }

        // paranoid mode
        if (size > 10000)
        {
            LOG_INFO ("size > 10000");
            break;
        }

        // finally open the file
        f = fopen (file_name, "wb");
        if (f == NULL)
        {
            LOG_INFO ("failed to open file \"%s\" - %s", file_name, strerror (errno));
            break;
        }

        // and do the work
        unsigned int processed = fwrite (data, 1, size, f);
        if (processed != size)
        {
            LOG_INFO ("failed to write to file \"%s\" - %s", file_name, strerror (errno));
            LOG_INFO ("processed %d, needed %d", processed, size);
            break;
        }

        fflush (f);

        // we have finished, set the success
        result = 0;
    } while (0);

    if (f != NULL)
        fclose (f);

    return result;
}

int read_from_file (
        const char* file_name,
        void** data,
        unsigned int* size)
{
    // set negative reply by default
    int result = -1;
    FILE* f = NULL;

    do
    {
        // nothing to read
        if (file_name == NULL)
        {
            LOG_INFO ("file name is NULL");
            break;
        }

        // nowhere to store
        if (data == NULL)
        {
            LOG_INFO ("data is NULL");
            break;
        }

        // nowhere to store
        if (*data != NULL)
        {
            LOG_INFO ("*data is not NULL");
            break;
        }

        // bad placeholder for size
        if (size == NULL)
        {
            LOG_INFO ("size is NULL");
            break;
        }

        // finally open the file
        f = fopen (file_name, "rb");
        if (f == NULL)
        {
            LOG_INFO ("failed to open file \"%s\" - %s", file_name, strerror (errno));
            break;
        }

        // get the size
        fseek (f, 0, SEEK_END);
        *size = ftell (f);
        fseek (f, 0, SEEK_SET);

        if (*size == 0)
        {
            LOG_INFO ("file \"%s\" is empty", file_name);
            break;
        }

        // and do the work
        *data = (char*) malloc (*size);
        unsigned int processed = fread (*data, 1, *size, f);
        if (processed != *size)
        {
            LOG_INFO ("failed to read from file \"%s\" - %s", file_name, strerror (errno));
            LOG_INFO ("processed %d, needed %d", processed, *size);
            free (*data);
            break;
        }

        // we have finished, set the success
        result = 0;
    } while (0);

    if (f != NULL)
        fclose (f);

    return result;
}

int drop_file (
        const char* file_name)
{
    // set negative reply by default
    int result = -1;

    do
    {
        // nothing to drop
        if (file_name == NULL)
        {
            LOG_INFO ("file name is NULL");
            break;
        }

        // does it exist
        struct stat buffer;
        if (stat (file_name, &buffer) != 0)
        {
            LOG_INFO ("file \"%s\" does not exist", file_name);
            result = 0;
            break;
        }

        if (unlink (file_name))
        {
            LOG_INFO ("failed to delete file \"%s\" - %s", file_name, strerror (errno));
            break;
        }

        result = 0;
    } while (0);

    return result;
}

int drop_old_file (
        const char* file_name,
        unsigned int exp_time)
{
    // set negative reply by default
    int result = -1;

    do
    {
        // nothing to drop
        if (file_name == NULL)
        {
            LOG_INFO ("file name is NULL");
            break;
        }

        // do we have the file?
        struct stat buffer;
        if (stat (file_name, &buffer) != 0)
        {
            LOG_INFO ("file \"%s\" does not exist", file_name);
            break;
        }

        // is it old enough to be deleted?
        time_t now = time (NULL);
        if ((unsigned long)now <= (unsigned long)(buffer.st_ctime + exp_time))
        {
            LOG_INFO ("file \"%s\" is not old enough", file_name);
            break;
        }

        // yeap, let's drop it
        if (unlink (file_name))
        {
            LOG_INFO ("failed to delete file \"%s\" - %s", file_name, strerror (errno));
            break;
        }

        result = 0;
    } while (0);

    return result;
}

int does_file_exist (
        const char* file_name)
{
    // by default file does not exist
    int result = 0;

    do
    {
        // nothing to check
        if (file_name == NULL)
        {
            LOG_INFO ("file name is NULL");
            break;
        }

        struct stat buffer;
        if (stat (file_name, &buffer) != 0)
        {
            LOG_INFO ("file \"%s\" does not exist", file_name);
            break;
        }

        // it's alive!
        result = 1;
    } while (0);

    return result;
}
