#ifndef FILE_H
#define FILE_H


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

void drop_file (
        const char* file_name);

void drop_old_file (
        const char* file_name,
        unsigned int exp_time);

int does_file_exist (
        const char* file_name);

#ifdef __cplusplus
}
#endif

#endif // FILE_H
