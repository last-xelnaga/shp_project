
#ifndef FILE_UTILS_H
#define FILE_UTILS_H


#ifdef __cplusplus
extern "C" {
#endif

int save_to_file (
        const char* file_name,
        const void* data,
        const unsigned int size);

int read_from_file (
        const char* file_name,
        void** data,
        unsigned int* size);

int drop_file (
        const char* file_name);

int drop_old_file (
        const char* file_name,
        unsigned int exp_time);

int does_file_exist (
        const char* file_name);

#ifdef __cplusplus
}
#endif

#endif // ifndef FILE_UTILS_H
