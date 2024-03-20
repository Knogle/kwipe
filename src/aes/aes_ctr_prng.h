/*
 * AES CTR PRNG Definitions
 * Author: Fabian Druschke
 * Date: 2024-03-13
 *
 * This header file contains definitions for the AES (Advanced Encryption Standard)
 * implementation in CTR (Counter) mode for pseudorandom number generation, utilizing
 * OpenSSL for cryptographic functions.
 *
 * As the author of this work, I, Fabian Druschke, hereby release this work into the public
 * domain. I dedicate any and all copyright interest in this work to the public domain,
 * making it free to use for anyone for any purpose without any conditions, unless such
 * conditions are required by law.
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

#ifndef AES_CTR_RNG_H
#define AES_CTR_RNG_H

#include <stdint.h>
#include <openssl/aes.h>

// Structure to store the state of the AES-CTR random number generator
typedef struct
{
    AES_KEY aes_key;  // AES key for encryption
    unsigned char ivec[AES_BLOCK_SIZE];  // Initialization vector
    unsigned int num;  // Number of bytes that have been processed
    unsigned char ecount[AES_BLOCK_SIZE];  // Encryption counter block
} aes_ctr_state_t;

// Initializes the AES-CTR random number generator
void init_aes_ctr( aes_ctr_state_t* state, const unsigned char* key );

// Generates a 128-bit random number using AES-CTR and stores it directly in the output buffer
void aes_ctr_prng_genrand_uint128_to_buf( aes_ctr_state_t* state, unsigned char* bufpos );


// For debugging
// void print_aes_ctr_state_to_file(const aes_ctr_state_t* state, const char* filename) {

#endif  // AES_CTR_RNG_H
