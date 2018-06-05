
//https://en.wikipedia.org/wiki/SHA-2

#include "sha256.h"
#include <string.h>


#define ROL(x, k) ((x << k) | (x >> (32 - k)))
#define ROR(x, k) ((x >> k) | (x << (32 - k)))

// first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19
static const unsigned int h [8] =
{
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311
static const unsigned int k [64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


static void do_loop (
        unsigned int* w,
        unsigned int* sqrt_arr)
{
    unsigned char* wc = (unsigned char*)w;

    unsigned int a [sizeof (h)];
    memcpy (a, sqrt_arr, sizeof (h));

    for (unsigned int i = 0; i < 16; ++ i)
        w [i] = (wc [i * 4] << 24) | (wc [i * 4 + 1] << 16) | (wc [i * 4 + 2] << 8) | (wc [i * 4 + 3]);

    // Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array
    for (unsigned int i = 16; i < 64; ++ i)
    {
        unsigned int s0 = ROR (w [ i - 15], 7) ^ ROR (w [i - 15], 18) ^ (w [i - 15] >> 3);
        unsigned int s1 = ROR (w [i - 2], 17) ^ ROR (w [i - 2], 19) ^ (w [i - 2] >> 10);
        w [i] = w [i - 16] + s0 + w [i - 7] + s1;
    }

    // Compression function main loop:
    for (unsigned int i = 0; i < 64; ++ i)
    {
        unsigned int s1 = ROR (a [4], 6) ^ ROR (a [4], 11) ^ ROR (a [4], 25);
        unsigned int ch = (a [4] & a [5]) ^ ((~a [4]) & a [6]);
        unsigned int temp1 = a[7] + s1 + ch + k [i] + w [i];
        unsigned int s0 = ROR (a [0], 2) ^ ROR (a [0], 13) ^ ROR (a [0], 22);
        unsigned int maj = (a [0] & a [1]) ^ (a [0] & a [2]) ^ (a [1] & a [2]);
        unsigned int temp2 = s0 + maj;

        a [7] = a [6];
        a [6] = a [5];
        a [5] = a [4];
        a [4] = a [3] + temp1;
        a [3] = a [2];
        a [2] = a [1];
        a [1] = a [0];
        a [0] = temp1 + temp2;
    }

    // Add the compressed chunk to the current hash value
    for (unsigned int i = 0; i < sizeof (h); ++ i)
        sqrt_arr [i] = sqrt_arr [i] + a [i];
}

void sha256 (
        const unsigned char* p_src,
        const unsigned int i_src_len,
        unsigned char* p_res)
{
    unsigned int w [64];
    int stopBitDone = 0, footerDone = 0;
    unsigned char* wc = (unsigned char*)w;

    unsigned int sqrt_arr [sizeof (h)];
    memcpy (sqrt_arr, h, sizeof (h));

    // Process the message in successive 512-bit chunks
    for (unsigned int offs = 0; offs < i_src_len; offs += 64)
    {
        unsigned int len = 64;
        if (offs + len > i_src_len)
        {
            len = i_src_len - offs;
            memset (wc + len, 0, sizeof (w) - len);
        }

        memcpy (w, p_src + offs, len);
        if (len < 64)
        {
            wc [len] = 0x80;
            len ++;
            stopBitDone = 1;
            if (len <= 56)
            {
                // Should be 8 bytes of length info, but we don't support that much data
                wc [60] = ((i_src_len * 8) >> 24) & 0xff;
                wc [61] = ((i_src_len * 8) >> 16) & 0xff;
                wc [62] = ((i_src_len * 8) >> 8)  & 0xff;
                wc [63] =  (i_src_len * 8)        & 0xff;
                footerDone = 1;
            }
        }

        do_loop (w, sqrt_arr);
    }

    if (footerDone == 0)
    {
        memset (w, 0, sizeof (w) - 4);
        if (stopBitDone == 0)
            wc [0] = 0x80;

        wc [60] = ((i_src_len * 8) >> 24) & 0xff;
        wc [61] = ((i_src_len * 8) >> 16) & 0xff;
        wc [62] = ((i_src_len * 8) >> 8)  & 0xff;
        wc [63] =  (i_src_len * 8)        & 0xff;

        do_loop (w, sqrt_arr);
    }

    for (unsigned int i = 0; i < sizeof (h); ++ i)
    {
        p_res [i * 4]     = (sqrt_arr [i] >> 24) & 0xff;
        p_res [i * 4 + 1] = (sqrt_arr [i] >> 16) & 0xff;
        p_res [i * 4 + 2] = (sqrt_arr [i] >> 8)  & 0xff;
        p_res [i * 4 + 3] = (sqrt_arr [i])       & 0xff;
    }
}
