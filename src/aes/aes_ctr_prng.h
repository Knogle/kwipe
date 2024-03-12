#ifndef AES_CTR_RNG_H
#define AES_CTR_RNG_H

#include <stdint.h>
#include <openssl/aes.h>

// Structure to store the state of the AES-CTR random number generator
typedef struct {
    AES_KEY aes_key;                   // AES key for encryption
    unsigned char ivec[AES_BLOCK_SIZE]; // Initialization vector
    unsigned int num;                  // Number of bytes that have been processed
    unsigned char ecount[AES_BLOCK_SIZE]; // Encryption counter block
} aes_ctr_state_t;

// Initializes the AES-CTR random number generator
void init_aes_ctr(aes_ctr_state_t* state, const unsigned char* key);

// Generates a 128-bit random number using AES-CTR and stores it directly in the output buffer
void aes_ctr_prng_genrand_uint128_to_buf(aes_ctr_state_t* state, unsigned char* bufpos);

#endif  // AES_CTR_RNG_H

