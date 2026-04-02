#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../include/sha1.h"

#define ROT32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/**
 * @brief core SHA-1 compression — processes one 64-byte block and updates the hash state
 *
 * expands the 16 input words into an 80-word schedule, then runs the
 * 80-round mixing loop across four round functions with different constants.
 *
 * @param block  64-byte input block to process
 * @param h0..h4 the five hash state words, updated in place after each block
 *
 * @note this is the inner loop — called once per 64-byte chunk of padded input
 */
static void sha1_process_block(const unsigned char block[64],
                               uint32_t *h0, uint32_t *h1, uint32_t *h2,
                               uint32_t *h3, uint32_t *h4)
{
    uint32_t w[80];
    int j;

    /* Build message schedule from the 16 input words */
    for (j = 0; j < 16; j++)
    {
        w[j] = ((uint32_t)block[j * 4] << 24) | ((uint32_t)block[j * 4 + 1] << 16) | ((uint32_t)block[j * 4 + 2] << 8) | ((uint32_t)block[j * 4 + 3]);
    }
    for (j = 16; j < 80; j++)
        w[j] = ROT32(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);

    uint32_t a = *h0, b = *h1, c = *h2, d = *h3, e = *h4;

    for (j = 0; j < 80; j++)
    {
        uint32_t f, k;

        if (j < 20)
        {
            f = (b & c) | (~b & d);
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

        uint32_t tmp = ROT32(a, 5) + f + e + k + w[j];
        e = d;
        d = c;
        c = ROT32(b, 30);
        b = a;
        a = tmp;
    }

    *h0 += a;
    *h1 += b;
    *h2 += c;
    *h3 += d;
    *h4 += e;
}

void sha1(const unsigned char *data, size_t len, unsigned char hash[20])
{
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    /*
     * SHA-1 pads the message to a multiple of 512 bits (64 bytes):
     *   - append 0x80 byte
     *   - append 0x00 bytes
     *   - append original bit-length as 8-byte big-endian at the very end
     *
     * We build the padded message in a separate buffer so we always
     * handle multi-block inputs correctly. The padded length is:
     *   ceil((len + 9) / 64) * 64
     */
    uint64_t bit_len = (uint64_t)len * 8;
    size_t padded_len = ((len + 9 + 63) / 64) * 64;

    unsigned char *msg = (unsigned char *)calloc(padded_len, 1);
    if (!msg)
        return;

    memcpy(msg, data, len); /* copy original data         */
    msg[len] = 0x80;        /* append the 1-bit           */

    /* Write 64-bit big-endian bit-length into the last 8 bytes */
    for (int i = 0; i < 8; i++)
        msg[padded_len - 1 - i] = (unsigned char)(bit_len >> (i * 8));

    /* Process every 64-byte block */
    for (size_t offset = 0; offset < padded_len; offset += 64)
        sha1_process_block(msg + offset, &h0, &h1, &h2, &h3, &h4);

    free(msg);

    /* Pack the five 32-bit words into 20 bytes, big-endian */
    uint32_t hh[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; i++)
    {
        hash[i * 4] = (hh[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (hh[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (hh[i] >> 8) & 0xff;
        hash[i * 4 + 3] = hh[i] & 0xff;
    }
}

void sha1_to_hex(const unsigned char hash[20], char output[41])
{
    for (int i = 0; i < 20; i++)
        sprintf(output + i * 2, "%02x", (unsigned char)hash[i]);
    output[40] = '\0';
}