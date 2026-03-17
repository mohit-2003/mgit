#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/sha1.h"

#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

void sha1(const unsigned char *data, size_t len, unsigned char hash[20])
{
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    uint64_t bitlen = len * 8;

    unsigned char chunk[64];
    size_t i, j;

    for (i = 0; i < len; i += 64)
    {
        memset(chunk, 0, 64);
        size_t chunk_len = (len - i < 64) ? len - i : 64;
        memcpy(chunk, data + i, chunk_len);

        if (chunk_len < 64)
        {
            chunk[chunk_len] = 0x80;

            if (chunk_len <= 55)
            {
                for (j = 0; j < 8; j++)
                    chunk[56 + j] = (bitlen >> ((7 - j) * 8)) & 0xff;
            }
        }

        uint32_t w[80];

        for (j = 0; j < 16; j++)
        {
            w[j] =
                (chunk[j * 4] << 24) |
                (chunk[j * 4 + 1] << 16) |
                (chunk[j * 4 + 2] << 8) |
                (chunk[j * 4 + 3]);
        }

        for (j = 16; j < 80; j++)
            w[j] = ROTLEFT(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);

        uint32_t a = h0;
        uint32_t b = h1;
        uint32_t c = h2;
        uint32_t d = h3;
        uint32_t e = h4;

        for (j = 0; j < 80; j++)
        {
            uint32_t f, k;

            if (j < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (j < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (j < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }

            uint32_t temp = ROTLEFT(a, 5) + f + e + k + w[j];
            e = d;
            d = c;
            c = ROTLEFT(b, 30);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    uint32_t hashes[5] = {h0, h1, h2, h3, h4};

    for (i = 0; i < 5; i++)
    {
        hash[i * 4] = (hashes[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (hashes[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (hashes[i] >> 8) & 0xff;
        hash[i * 4 + 3] = hashes[i] & 0xff;
    }
}

void sha1_to_hex(const unsigned char hash[20], char output[41])
{
    for (int i = 0; i < 20; i++)
        sprintf(output + (i * 2), "%02x", hash[i]);

    output[40] = '\0';
}