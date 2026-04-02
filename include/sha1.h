#ifndef SHA1_H
#define SHA1_H

#include <stddef.h>

/**
 * @brief computes a SHA-1 hash over an arbitrary byte buffer
 *
 * pads the input, splits it into 64-byte blocks, and runs each through
 * sha1_process_block(). result is written as 20 raw bytes into hash.
 *
 * @param data  input bytes to hash
 * @param len   length of the input in bytes
 * @param hash  output buffer for the 20-byte binary hash
 */
void sha1(const unsigned char *data, size_t len, unsigned char hash[20]);

/**
 * @brief converts a 20-byte binary SHA-1 hash to a 40-char hex string
 *
 * @param hash    20-byte binary hash from sha1()
 * @param output  output buffer, must be at least 41 bytes (40 hex + null terminator)
 */
void sha1_to_hex(const unsigned char hash[20], char output[41]);

#endif
