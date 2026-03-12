#ifndef SHA1_UTIL_H
#define SHA1_UTIL_H

/**
 * @brief Compute SHA-1 hash of given data.
 *
 * This function computes the SHA-1 digest of an arbitrary byte sequence.
 * The resulting hash is stored as a 20-byte binary value.
 *
 * @param data Pointer to input data buffer.
 * @param len Length of input data in bytes.
 * @param hash Output buffer where the 20-byte SHA-1 digest will be stored.
 *
 * @note The output buffer must have space for at least 20 bytes.
 */
void sha1(const unsigned char *data, size_t len, unsigned char hash[20]);

/**
 * @brief Convert binary SHA-1 hash to hexadecimal string.
 *
 * Git stores SHA-1 values as 40-character hexadecimal strings.
 * This function converts the 20-byte binary SHA-1 digest into
 * a readable hex representation.
 *
 * @param hash 20-byte SHA-1 digest.
 * @param output Output buffer that will contain the 40-character hex string.
 *
 * @note The output buffer must be at least 41 bytes long to include the
 *       null terminator.
 */
void sha1_to_hex(const unsigned char hash[20], char output[41]);

#endif