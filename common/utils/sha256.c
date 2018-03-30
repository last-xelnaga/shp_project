
#include <string.h>
#include "sha256.h"

//#include "secd_sha256.hpp"

#define ROL(x,k) ((x<<k)|(x>>(32-k)))
#define ROR(x,k) ((x>>k)|(x<<(32-k)))

static const unsigned int cubeRoot [64] =
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

static void ExpandIt (
        unsigned int *w)
{
    int i;
    unsigned int s0, s1;
    for (i = 16; i < 64; i ++)
    {
        s0 = ROR(w[i-15],7) ^ ROR(w[i-15],18) ^ (w[i-15]>>3);
        s1 = ROR(w[i-2],17) ^ ROR(w[i-2],19) ^ (w[i-2]>>10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
}

static void MangleIt (
        const unsigned int* w,
        unsigned int* h)
{
    unsigned int s0, s1, maj, t2, t1, ch;
    unsigned int a [8];

    memcpy(a, h, sizeof(a));

    // main loop:
    for (unsigned int i = 0; i < 64; i ++)
    {
        s0 = ROR(a[0],2) ^ ROR(a[0],13) ^ ROR(a[0],22);
        maj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
        t2 = s0 + maj;
        s1 = ROR(a[4],6) ^ ROR(a[4],11) ^ ROR(a[4],25);
        ch = (a[4] & a[5]) ^ ((~a[4]) & a[6]);
        t1 = a[7] + s1 + ch + cubeRoot[i] + w[i];

        a[7] = a[6];
        a[6] = a[5];
        a[5] = a[4];
        a[4] = a[3] + t1;
        a[3] = a[2];
        a[2] = a[1];
        a[1] = a[0];
        a[0] = t1 + t2;
    }

    for (unsigned int k = 0; k < 8; k ++)
    {
        h[k] = h[k] + a[k];
    }
}

static void FixEndian (
        unsigned int *w)
{
    unsigned char* wc = (unsigned char*)w;
    for (unsigned int i = 0; i < 16; i ++)
    {
        w[i] = (wc[i*4]<<24) | (wc[i*4+1]<<16) | (wc[i*4+2]<<8) | (wc[i*4+3]);
    }
}

int sha256 (
        const unsigned char* p_src,
        const unsigned int i_src_len,
        unsigned char* p_res)
{
    int result = 0;

    //FAIL_IF_PARAM_NULL(pSrc);
    //FAIL_IF_PARAM_ZERO(i_src_len);
    //FAIL_IF_PARAM_NULL(pRes);

    unsigned int h [8] =
    {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    if (result == 0)
    {
        unsigned int w [64];
        int stopBitDone = 0, footerDone = 0;
        unsigned char* wc = (unsigned char*)w;

        // Process message in 512-bit chunks
        for (unsigned int offs = 0; offs < i_src_len; offs += 64)
        {
            unsigned int len = 64;
            if (offs + len > i_src_len)
            {
                len = i_src_len - offs;
                memset(wc + len, 0, sizeof(w) - len);
            }
            memcpy(w, p_src + offs, len);
            if (len < 64)
            {
                wc[len] = 0x80;
                len ++;
                stopBitDone = 1;
                if (len <= 56)
                {
                    // Should be 8 bytes of length info, but we don't support that much data
                    wc[60] = ((i_src_len * 8) >> 24) & 0xff;
                    wc[61] = ((i_src_len * 8) >> 16) & 0xff;
                    wc[62] = ((i_src_len * 8) >> 8) & 0xff;
                    wc[63] = (i_src_len * 8) & 0xff;
                    footerDone = 1;
                }
            }
            FixEndian(w);
            ExpandIt(w);
            MangleIt(w,h);
        }

        if (footerDone == 0)
        {
            memset(w, 0, sizeof(w) - 4);
            if (stopBitDone == 0)
            {
                wc[0] = 0x80;
            }
            wc[60] = ((i_src_len*8) >> 24) & 0xff;
            wc[61] = ((i_src_len*8) >> 16) & 0xff;
            wc[62] = ((i_src_len*8) >> 8) & 0xff;
            wc[63] = (i_src_len*8) & 0xff;
            FixEndian(w);
            ExpandIt(w);
            MangleIt(w,h);
        }
    }

    if (result == 0)
    {
        for (unsigned int k = 0; k < 8; k ++)
        {
            p_res [k*4] = (h[k]>>24) & 0xff;
            p_res [k*4+1] = (h[k]>>16) & 0xff;
            p_res [k*4+2] = (h[k]>>8) & 0xff;
            p_res [k*4+3] = (h[k]) & 0xff;
        }
    }

    return result;
}
