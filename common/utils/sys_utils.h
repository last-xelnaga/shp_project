
#ifndef SYS_UTILS_H
#define SYS_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// Increase scheduling priority and algorithm to try to get 'real time' results.
#define PRIORITY_MAX        0
// Drop scheduling priority back to normal/default.
#define PRIORITY_DEFAULT    1


void set_app_priority (
        const int priority);

#ifdef __cplusplus
}
#endif

#endif // ifndef SYS_UTILS_H
