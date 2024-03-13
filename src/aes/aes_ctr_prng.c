#include "aes_ctr_prng.h"
#include <openssl/rand.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/modes.h>
#include <openssl/sha.h>  // Include for SHA256

void aes_ctr_prng_init( aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length )
{
    unsigned char temp_key[32];  // Temporary storage for the random key
    unsigned char key[32];  // Size for 256 bits

    // Generate a strong, random key using RAND_bytes
    if( RAND_bytes( temp_key, sizeof( temp_key ) ) != 1 )
    {
        // Error handling: RAND_bytes failed to generate a key
        // Insert abort or error handling here
    }

    // Use SHA256 to generate the final key from the random key and init_key
    SHA256_CTX sha256;
    SHA256_Init( &sha256 );
    SHA256_Update( &sha256, temp_key, sizeof( temp_key ) );  // Add the random key
    SHA256_Update( &sha256, (unsigned char*) init_key, key_length * sizeof( unsigned long ) );  // Add the init_key
    SHA256_Final( key, &sha256 );  // Generate the final key

    AES_set_encrypt_key( key, 256, &state->aes_key );  // Use the 256-bit key
    memset( state->ivec, 0, AES_BLOCK_SIZE );  // Initialize the IV with zeros (or use RAND_bytes for the IV)
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
    // Generate random bytes directly into bufpos using RAND_bytes
    if( RAND_bytes( bufpos, 16 ) != 1 )
    {
        // Error handling: RAND_bytes failed to generate random bytes
        // Insert abort or error handling here
    }
    // No need to call next_state since RAND_bytes is used for random generation
}
