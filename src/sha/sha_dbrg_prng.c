#include "sha_dbrg_prng.h"
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>

void sha_dbrg_prng_init(sha_dbrg_state_t* state, unsigned long init_key[], unsigned long key_length) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md = EVP_sha512();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);

    // Process the initial key
    EVP_DigestUpdate(mdctx, init_key, key_length * sizeof(unsigned long));
    // Finalize the digest, forming the initial seed state
    EVP_DigestFinal_ex(mdctx, state->seed, NULL);

    EVP_MD_CTX_free(mdctx);
}

static void next_state(sha_dbrg_state_t* state) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md = EVP_sha512();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);

    // Update the state by re-hashing
    EVP_DigestUpdate(mdctx, state->seed, sizeof(state->seed));
    EVP_DigestFinal_ex(mdctx, state->seed, NULL);

    EVP_MD_CTX_free(mdctx);
}

void sha_dbrg_prng_genrand_uint512_to_buf(sha_dbrg_state_t* state, unsigned char* bufpos) {
    // Generate random data based on the current state
    EVP_MD_CTX* mdctx;
    const EVP_MD* md = EVP_sha512();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, state->seed, sizeof(state->seed));
    EVP_DigestFinal_ex(mdctx, bufpos, NULL);

    EVP_MD_CTX_free(mdctx);

    // Update the state for the next generation
    next_state(state);
}
