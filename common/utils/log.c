
#include "log.h"


//#ifdef ESP_TARGET
//#include "esp_log.h"
//#else
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
//#endif // ESP_TARGET

#define MAX_LOG_MSG_SIZE            1024


void debug_log_print (
        const char* p_tag,
        const int prio,
        const char* p_file_name,
        const unsigned int line,
        const char* p_text,
        ...)
{
    char buffer [MAX_LOG_MSG_SIZE + 1] = { 0 };

    // set the priority
    char prio_letter [] = "E";
    if (prio == LOG_INFO_ID)
        prio_letter [0] = 'I';
    else if (prio == LOG_DEBUG_ID)
        prio_letter [0] = 'D';

    time_t now = time (NULL);
    struct tm* tm = localtime (&now);

    // organize the prefix
    if (p_file_name != NULL)
        snprintf (buffer, MAX_LOG_MSG_SIZE, "[%02d:%02d:%02d] %s:%s  %s:%d\t",
                tm->tm_hour, tm->tm_min, tm->tm_sec, p_tag, prio_letter,
                p_file_name, line);
    else
        snprintf (buffer, MAX_LOG_MSG_SIZE, "%s:%s\t", p_tag, prio_letter);
    unsigned int offset = strlen (buffer);

    va_list args;
    va_start (args, p_text);
    vsnprintf (&buffer [offset], MAX_LOG_MSG_SIZE - offset, p_text, args);
    fprintf (stderr, "%s\n", buffer);
    va_end (args);
}

void debug_log_print_array (
        const char* p_tag,
        const char* p_header,
        const unsigned char* p_data,
        const unsigned int length)
{
    // pattern
    char p_hex_arr [] = "0123456789abcdef";

    // hex representation string
    char p_log_hex [128] = { 0 };
    unsigned int p_log_hex_len = 0;

    // ascii representation string
    char p_log_ascii [128] = { 0 };
    unsigned int p_log_ascii_len = 0;

    // write the header
    debug_log_print (p_tag, LOG_DEBUG_ID, NULL, 0,
            "array: %s, length: %d", p_header, length);

    // process all bytes
    for (unsigned int i = 0; i < length; ++ i)
    {
        // do we have a line to show?
        if (p_log_hex_len >= 30)
        {
            // ensure the null in the end
            p_log_hex [p_log_hex_len] = 0;
            p_log_hex_len = 0;

            p_log_ascii [p_log_ascii_len] = 0;
            p_log_ascii_len = 0;

            // write the line
            debug_log_print (p_tag, LOG_DEBUG_ID, NULL, 0,
                    "%s || %s", p_log_hex, p_log_ascii);
        }

        // process byte and make a hex from it
        p_log_hex [p_log_hex_len ++] = p_hex_arr [(p_data [i] & 0xf0) >> 4];
        p_log_hex [p_log_hex_len ++] = p_hex_arr [p_data [i] & 0x0f];
        p_log_hex [p_log_hex_len ++] = ' ';

        // show printable characters, otherwise show a '.'
        p_log_ascii [p_log_ascii_len ++] = p_data [i] >= 0x20 ? p_data [i] : '.';
    }

    // write last piece of data
    if (p_log_hex_len > 0)
    {
        // shift the end of the string to allign the second part
        memset (&p_log_hex [p_log_hex_len], ' ', 30 - p_log_hex_len);
        p_log_hex_len = 30;

        // ensure that our strings null-terminated
        p_log_hex [p_log_hex_len] = 0;
        p_log_ascii [p_log_ascii_len] = 0;

        debug_log_print (p_tag, LOG_DEBUG_ID, NULL, 0,
                "%s || %s ", p_log_hex, p_log_ascii);
    }
}
