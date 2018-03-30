
#ifndef SHA256_H
#define SHA256_H

#define SHA256_RESULT_LEN               32

#ifdef __cplusplus
extern "C" {
#endif

int sha256 (
        const unsigned char* p_src,
        const unsigned int i_src_len,
        unsigned char* p_res);

#ifdef __cplusplus
}
#endif

#endif // SHA256_H
