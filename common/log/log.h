#ifndef LOG_H
#define LOG_H

#include <string.h>


// DEBUG_TAG defined in Makefile
#define QUOTE(name)                 #name
#define STR(macro)                  QUOTE(macro)
#define DEBUG_TAG_NAME              STR(DEBUG_TAG)

// short form of the file name
#define FLE     strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

// debug levels
#define LOG_ERROR                   1  // LOG_ERROR
#define LOG_INFO                    2  // LOG_INFO

// prototypes for the Infos
#define DEBUG_LOG_INFO(...)                         \
    debug_log_print (DEBUG_TAG_NAME, LOG_INFO, FLE, \
            __LINE__, __VA_ARGS__);

// prototypes for the Errors
#define DEBUG_LOG_ERROR(...)                        \
    debug_log_print (DEBUG_TAG_NAME, LOG_ERROR, FLE,\
            __LINE__, __VA_ARGS__);

// prototype for print array function
#define SECD_DEBUG_LOG_PRINT_ARRAY(__header, __data, __length)\
    debug_log_print_array (DEBUG_TAG_NAME, __header, __data, __length)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function will store a provided string into the buffer
 * for the debug messages.
 *
 * @param p_tag      [in]  pointer to the tag string.
 * @param prio       [in]  required priority of the log message.
 * @param p_file_name[in]  file name. see FLE macro.
 * @param line       [in]  line of the code. will be used __LINE__.
 * @param p_text     [in]  pointer to the log message.
 */
void debug_log_print (
        const char* p_tag,
        const int prio,
        const char* p_file_name,
        const unsigned int line,
        const char* p_text,
        ...);

/*
 * Function will show data buffer with header.
 *
 * @param p_tag      [in]  pointer to the tag string.
 * @param p_header   [in]  header message before data.
 * @param p_data     [in]  data buffer that should be shown.
 * @param length     [in]  length of the data buffer.
 */
void debug_log_print_array (
        const char* p_tag,
        const char* p_header,
        const unsigned char* p_data,
        const unsigned int length);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
