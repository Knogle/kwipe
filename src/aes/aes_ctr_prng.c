#include "aes_ctr_prng.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>  // For calloc and free
#include <stdbool.h>  // For the bool data type

typedef enum nwipe_log_t_ {
    NWIPE_LOG_NONE = 0,
    NWIPE_LOG_DEBUG,  // Output only when --verbose option used on cmd line.
    NWIPE_LOG_INFO,  // General Info not specifically relevant to the wipe.
    NWIPE_LOG_NOTICE,  // Most logging happens at this level related to wiping.
    NWIPE_LOG_WARNING,  // Things that the user should know about.
    NWIPE_LOG_ERROR,  // Non-fatal errors that result in failure.
    NWIPE_LOG_FATAL,  // Errors that cause the program to exit.
    NWIPE_LOG_SANITY,  // Programming errors.
    NWIPE_LOG_NOTIMESTAMP  // logs the message without the timestamp
} nwipe_log_t;

// External declaration of the nwipe_log function
extern void nwipe_log( nwipe_log_t level, const char* format, ... );

void aes_ctr_prng_init( aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length )
{
    if( state == NULL )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: state is NULL" );
        return;  // Ensure the state is not NULL
    }

    unsigned char key[32];  // Space for a 256-bit key
    memset( key, 0, sizeof( key ) );  // Initially set the key to zero

    memset( state->ivec, 0, AES_BLOCK_SIZE );  // Initialize IV (Initialization Vector) and counter before use
    state->num = 0;
    memset( state->ecount, 0, AES_BLOCK_SIZE );

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if( mdctx == NULL )
    {
        state->is_initialized = false;
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_MD_CTX_new" );
        return;
    }

    if( !EVP_DigestInit_ex( mdctx, EVP_sha256(), NULL ) )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_DigestInit_ex" );
        EVP_MD_CTX_free( mdctx );
        state->is_initialized = false;
        return;
    }

    if( !EVP_DigestUpdate( mdctx, (unsigned char*) init_key, key_length * sizeof( unsigned long ) ) )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_DigestUpdate" );
        EVP_MD_CTX_free( mdctx );
        state->is_initialized = false;
        return;
    }

    if( !EVP_DigestFinal_ex( mdctx, key, NULL ) )
    {  // Generate the final key
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_DigestFinal_ex" );
        EVP_MD_CTX_free( mdctx );
        state->is_initialized = false;
        return;
    }
    EVP_MD_CTX_free( mdctx );

    state->ctx = EVP_CIPHER_CTX_new();
    if( state->ctx == NULL )
    {
        state->is_initialized = false;
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_CIPHER_CTX_new" );
        return;
    }

    if( !EVP_EncryptInit_ex( state->ctx, EVP_aes_256_ctr(), NULL, key, state->ivec ) )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_init: Error at EVP_EncryptInit_ex" );
        EVP_CIPHER_CTX_free( state->ctx );
        state->is_initialized = false;
        return;
    }

    state->is_initialized = true;  // Mark as initialized
    nwipe_log( NWIPE_LOG_DEBUG, "aes_ctr_prng_init: Initialization successful" );
}

void aes_ctr_prng_genrand_uint128_to_buf( aes_ctr_state_t* state, unsigned char* bufpos )
{
    if( state == NULL || !state->is_initialized )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_genrand_uint128_to_buf: State not initialized or NULL" );
        return;  // Ensure the state has been initialized
    }

    unsigned char temp_buffer[16];  // Temporary buffer for 128 bits
    int outlen;

    if( !EVP_EncryptUpdate( state->ctx, temp_buffer, &outlen, temp_buffer, sizeof( temp_buffer ) ) )
    {
        nwipe_log( NWIPE_LOG_ERROR, "aes_ctr_prng_genrand_uint128_to_buf: Error at EVP_EncryptUpdate" );
        return;
    }

    memcpy( bufpos, temp_buffer, 16 );
    // nwipe_log(NWIPE_LOG_DEBUG, "aes_ctr_prng_genrand_uint128_to_buf: Random data successfully generated");
}

void aes_ctr_prng_cleanup( aes_ctr_state_t* state )
{
    if( state != NULL )
    {
        if( state->ctx != NULL )
        {
            EVP_CIPHER_CTX_free( state->ctx );
            state->ctx = NULL;
            nwipe_log( NWIPE_LOG_DEBUG, "aes_ctr_prng_cleanup: Resources successfully released" );
        }

        state->is_initialized = false;  // Reset the initialization status
    }
}
