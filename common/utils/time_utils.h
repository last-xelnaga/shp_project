
#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

//
void sleep_milliseconds (
        const unsigned int millis);

const char* get_time_str (
        void);

void set_curr_time (
        const unsigned long now);

#ifdef __cplusplus
}
#endif

#endif // ifndef TIME_UTILS_H
