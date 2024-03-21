#ifndef SHA_DRBG_PRNG_H
#define SHA_DRBG_PRNG_H

#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>

// State definition for the SHA-256-based PRNG
typedef struct {
    unsigned char seed[32]; // SHA-256 output size for the state
} sha_dbrg_state_t;

// Function prototypes
void sha_dbrg_prng_init(sha_dbrg_state_t* state, unsigned long init_key[], unsigned long key_length);
void sha_dbrg_prng_genrand_uint256_to_buf(sha_dbrg_state_t* state, unsigned char* bufpos);

#endif // SHA_DRBG_PRNG_H
