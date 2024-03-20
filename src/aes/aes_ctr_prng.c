/*
 * AES CTR PRNG Implementation
 * Author: Fabian Druschke
 * Date: 2024-03-13
 *
 * This is an AES (Advanced Encryption Standard) implementation in CTR (Counter) mode
 * for pseudorandom number generation, utilizing OpenSSL for cryptographic functions.
 *
 * As the author of this implementation, I, Fabian Druschke, hereby release this work into
 * the public domain. I dedicate any and all copyright interest in this work to the public
 * domain, making it free to use for anyone for any purpose without any conditions, unless
 * such conditions are required by law.
 *
 * This software is provided "as is", without warranty of any kind, express or implied,
 * including but not limited to the warranties of merchantability, fitness for a particular
 * purpose and noninfringement. In no event shall the authors be liable for any claim,
 * damages or other liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings in the software.
 *
 * USAGE OF OPENSSL IN THIS SOFTWARE:
 * This software uses OpenSSL for cryptographic operations. Users are responsible for
 * ensuring compliance with OpenSSL's licensing terms.
 */

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
/*
void print_aes_ctr_state_to_file(const aes_ctr_state_t* state, const char* filename) {
    FILE* file = fopen(filename, "a"); // Öffne die Datei im Append-Modus
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        return;
    }

    fprintf(file, "AES CTR State:\n");
    fprintf(file, "IVec: ");
    for(int i = 0; i < AES_BLOCK_SIZE; i++) {
        fprintf(file, "%02x", state->ivec[i]);
    }
    fprintf(file, "\nNum: %u\n", state->num);
    fprintf(file, "ECount: ");
    for(int i = 0; i < AES_BLOCK_SIZE; i++) {
        fprintf(file, "%02x", state->ecount[i]);
    }
    // Der AES-Schlüssel ist direkt nicht zugänglich für Ausgabe,
    // aus Sicherheitsgründen überspringen wir diesen Teil.
    fprintf(file, "\nAES Key: [Nicht direkt zugänglich]\n\n");

    fclose(file); // Schließe die Datei
}
*/

void aes_ctr_prng_genrand_uint128_to_buf( aes_ctr_state_t* state, unsigned char* bufpos )
{
    // Initialize bufpos directly with random numbers, avoiding the use of a separate output buffer
    CRYPTO_ctr128_encrypt(
        bufpos, bufpos, 16, &state->aes_key, state->ivec, state->ecount, &state->num, (block128_f) AES_encrypt );

    // Ensure that next_state is called correctly without causing errors
    next_state( state );
}
