#ifndef BASE64_H
#define BASE64_H


#ifdef __cplusplus
extern "C" {
#endif

char* base64_encode (
        const unsigned char* data,
        unsigned int input_length,
        unsigned int* output_length);

unsigned char* base64_decode (
        const char* data,
        unsigned int input_length,
        unsigned int* output_length);

void base64_cleanup (
        void);

// replace + with -, / with _ and drop = characters
void convert_to_base64url (
        char* data,
        unsigned int* input_length);

#ifdef __cplusplus
}
#endif

#endif // BASE64_H
