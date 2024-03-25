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
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>

/* Initializes the AES CTR pseudorandom number generator state.
   - state: Pointer to the AES CTR PRNG state structure.
   - init_key: Array containing the seed for key generation.
   - key_length: Length of the seed array. */
void aes_ctr_prng_init( aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length )
{
    unsigned char key[32];  // Space for a 256-bit key

    // Initialize IV (initialization vector) and counter before use
    memset( state->ivec, 0, AES_BLOCK_SIZE );
    state->num = 0;
    memset( state->ecount, 0, AES_BLOCK_SIZE );

    // Use EVP for SHA-256 hashing to generate the key from the provided seed
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex( mdctx, EVP_sha256(), NULL );
    EVP_DigestUpdate( mdctx, (unsigned char*) init_key, key_length * sizeof( unsigned long ) );

    // Optional: Add a salt to increase uniqueness
    // const unsigned char salt[] = "optional salt value";
    // EVP_DigestUpdate(mdctx, salt, sizeof(salt));

    EVP_DigestFinal_ex( mdctx, key, NULL );  // Generate the final key
    EVP_MD_CTX_free( mdctx );

    state->ctx = EVP_CIPHER_CTX_new();
    // Initialize the encryption context with AES-256-CTR mode
    EVP_EncryptInit_ex( state->ctx, EVP_aes_256_ctr(), NULL, key, state->ivec );
}

/* Generates pseudorandom numbers and writes them to a buffer.
   - state: Pointer to the initialized AES CTR PRNG state.
   - bufpos: Target buffer where the pseudorandom numbers will be written. */
void aes_ctr_prng_genrand_uint128_to_buf(aes_ctr_state_t* state, unsigned char* bufpos) {
    unsigned char temp_buffer[16]; // Intermediate buffer for 128 bits
    int outlen;

    // Generate pseudorandom numbers in the intermediate buffer
    EVP_EncryptUpdate(state->ctx, temp_buffer, &outlen, temp_buffer, sizeof(temp_buffer));

    // Directly copy the generated random data to the destination buffer
    memcpy(bufpos, temp_buffer, 16);
}


