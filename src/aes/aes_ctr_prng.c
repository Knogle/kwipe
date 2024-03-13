#include "aes_ctr_prng.h"
#include <openssl/rand.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/modes.h>
#include <openssl/sha.h>  // Include for SHA256

void aes_ctr_prng_init( aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length )
{
    unsigned char key[32];  // Größe für 256 Bits

    SHA256_CTX sha256;
    SHA256_Init( &sha256 );
    SHA256_Update(
        &sha256, (unsigned char*) init_key, key_length * sizeof( unsigned long ) );  // Füge den init_key hinzu

    // Optional: Ein Salt hinzufügen, um die Eindeutigkeit zu erhöhen
    // const unsigned char salt[] = "optional salt value";
    // SHA256_Update(&sha256, salt, sizeof(salt));

    SHA256_Final( key, &sha256 );  // Generiere den finalen Schlüssel

    AES_set_encrypt_key( key, 256, &state->aes_key );  // Verwende den 256-Bit-Schlüssel
    memset( state->ivec, 0, AES_BLOCK_SIZE );  // Initialisiere das IV mit Nullen
    state->num = 0;
    memset( state->ecount, 0, AES_BLOCK_SIZE );

    // Drucke den finalen Schlüssel in Hexadezimal
    printf( "Final Key: " );
    for( int i = 0; i < sizeof( key ); i++ )
    {
        printf( "%02x", key[i] );
    }
    printf( "\n" );
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
