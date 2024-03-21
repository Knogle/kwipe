#ifndef SHA_DRBG_PRNG_H
#define SHA_DRBG_PRNG_H

#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>

// State definition for the SHA-512-based PRNG
typedef struct {
    unsigned char seed[64]; // SHA-512 output size for the state
} sha_dbrg_state_t;

// Function prototypes
void sha_dbrg_prng_init(sha_dbrg_state_t* state, unsigned long init_key[], unsigned long key_length);
void sha_dbrg_prng_genrand_uint512_to_buf(sha_dbrg_state_t* state, unsigned char* bufpos);

#endif // SHA_DRBG_PRNG_H
