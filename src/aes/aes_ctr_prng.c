#include "aes_ctr_prng.h"
#include <openssl/rand.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/modes.h>
#include <openssl/sha.h>  // Include for SHA256

void aes_ctr_prng_init( aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length )
{
    unsigned char key[32];  // Size for 256 bits

    SHA256_CTX sha256;
    SHA256_Init( &sha256 );
    SHA256_Update( &sha256, (unsigned char*) init_key, key_length * sizeof( unsigned long ) );  // Add the init_key

    // Optional: Add a salt to increase uniqueness
    // const unsigned char salt[] = "optional salt value";
    // SHA256_Update(&sha256, salt, sizeof(salt));

    SHA256_Final( key, &sha256 );  // Generate the final key

    AES_set_encrypt_key( key, 256, &state->aes_key );  // Use the 256-bit key
    memset( state->ivec, 0, AES_BLOCK_SIZE );  // Initialize the IV with zeros
    state->num = 0;
    memset( state->ecount, 0, AES_BLOCK_SIZE );
}

static void next_state( aes_ctr_state_t* state )
{
    for( int i = 0; i < AES_BLOCK_SIZE; ++i )
    {
        if( ++state->ivec[i] )
            break;
    }
}

void aes_ctr_prng_genrand_uint128_to_buf( aes_ctr_state_t* state, unsigned char* bufpos )
{
    // Initialize bufpos directly with random numbers, avoiding the use of a separate output buffer
    CRYPTO_ctr128_encrypt(
        bufpos, bufpos, 16, &state->aes_key, state->ivec, state->ecount, &state->num, (block128_f) AES_encrypt );

    // Ensure that next_state is called correctly without causing errors
    next_state( state );
}
