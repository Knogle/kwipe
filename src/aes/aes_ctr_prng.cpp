#include "aes_ctr_prng.h"
#include <wmmintrin.h>  // For AES-NI functions
#include <cstring>
#include <cstdlib>
#include <memory>  // For std::unique_ptr

#define AES_BLOCK_SIZE 16  // Define AES block size as 16 bytes (128 bits)

// Extern "C" ensures C++ name mangling is disabled so that C code can link to these functions
extern "C" {

// Enum for logging levels, facilitating detailed control over how much information is logged.
typedef enum nwipe_log_t_ {
    NWIPE_LOG_NONE = 0,
    NWIPE_LOG_DEBUG,
    NWIPE_LOG_INFO,
    NWIPE_LOG_NOTICE,
    NWIPE_LOG_WARNING,
    NWIPE_LOG_ERROR,
    NWIPE_LOG_FATAL,
    NWIPE_LOG_SANITY,
    NWIPE_LOG_NOTIMESTAMP
} nwipe_log_t;

// Logging function prototype for diagnostics and insights.
void nwipe_log( nwipe_log_t level, const char* format, ... );
}

// AesCtrState manages the state required by the AES CTR pseudorandom number generator.
class AesCtrState {
  public:
    uint8_t ivec[AES_BLOCK_SIZE];  // Initialization vector for CTR mode.
    uint8_t key[32];               // AES-256 key (256 bits).
    __m128i key_schedule[15];      // AES-256 key schedule.
    uint32_t num;                  // Counter for the current block.

    // Constructor to initialize the IV and key schedule.
    AesCtrState()
        : num( 0 ) {
        std::memset( ivec, 0, AES_BLOCK_SIZE );
        std::memset( key, 0, 32 );
        nwipe_log( NWIPE_LOG_NOTICE, "AES CTR PRNG state initialized." );
    }
};

extern "C" {
// Function to create a new AES-CTR PRNG state, enabling use of C++ objects from C code.
void* create_aes_ctr_state() {
    nwipe_log( NWIPE_LOG_DEBUG, "Creating AES CTR PRNG state." );
    return new AesCtrState();  // Allocate a new AesCtrState object and return as void*.
}

// Corresponding function to delete a previously created AES-CTR PRNG state.
void delete_aes_ctr_state( void* state ) {
    nwipe_log( NWIPE_LOG_DEBUG, "Deleting AES CTR PRNG state." );
    delete static_cast<AesCtrState*>( state );  // Cast the void* back to AesCtrState* and delete.
}

// AES-NI helper functions for key expansion and encryption
static inline void aes_key_expansion(const uint8_t* key, __m128i* key_schedule) {
    __m128i temp1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key));
    __m128i temp2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key + 16));
    key_schedule[0] = temp1;
    key_schedule[1] = temp2;

    // First round with constant 0x01
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x01);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[2] = temp2;

    // Second round with constant 0x02
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x02);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[3] = temp2;

    // Third round with constant 0x04
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x04);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[4] = temp2;

    // Fourth round with constant 0x08
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x08);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[5] = temp2;

    // Fifth round with constant 0x10
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x10);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[6] = temp2;

    // Sixth round with constant 0x20
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x20);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[7] = temp2;

    // Seventh round with constant 0x40
    temp1 = _mm_aeskeygenassist_si128(temp2, 0x40);
    temp1 = _mm_shuffle_epi32(temp1, 0xFF);
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, _mm_slli_si128(temp2, 4));
    temp2 = _mm_xor_si128(temp2, temp1);
    key_schedule[8] = temp2;

    // Continue for remaining rounds with predefined constants.
    // Round 8, 9, 10, etc. can follow a similar pattern if needed for your AES implementation.
}


// Increment the AES-CTR counter
static inline void increment_counter(uint8_t* counter) {
    for (int i = AES_BLOCK_SIZE - 1; i >= 0; --i) {
        if (++counter[i]) break;
    }
}

// AES-CTR encryption function (encrypts one block)
static inline void aes_encrypt_block(const uint8_t* input, uint8_t* output, __m128i* key_schedule) {
    __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input));

    // Apply the key schedule for AES-256
    block = _mm_xor_si128(block, key_schedule[0]);
    for (int i = 1; i < 14; ++i) {
        block = _mm_aesenc_si128(block, key_schedule[i]);
    }
    block = _mm_aesenclast_si128(block, key_schedule[14]);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(output), block);
}

// Initializes the AES-CTR PRNG with a seed and sets up the AES key schedule.
void aes_ctr_prng_init(aes_ctr_state_t* state, unsigned long* init_key, unsigned long key_length) {
    auto* cppState = reinterpret_cast<AesCtrState*>(state);

    // Convert the seed to a 256-bit key
    std::memcpy(cppState->key, init_key, std::min<size_t>(key_length * sizeof(unsigned long), 32));

    // Expand the key into the key schedule for AES-256
    aes_key_expansion(cppState->key, cppState->key_schedule);

    nwipe_log(NWIPE_LOG_INFO, "AES CTR PRNG initialized with provided seed.");
}

// Fills a buffer with pseudorandom data generated using AES-CTR mode.
void aes_ctr_prng_genrand_uint256_to_buf(aes_ctr_state_t* state, unsigned char* bufpos) {
    auto* cppState = reinterpret_cast<AesCtrState*>(state);
    uint8_t keystream[AES_BLOCK_SIZE];  // Buffer for the keystream (16 bytes = 128 bits)

    // Encrypt the counter to produce the keystream
    aes_encrypt_block(cppState->ivec, keystream, cppState->key_schedule);

    // Copy the keystream into the output buffer
    std::memcpy(bufpos, keystream, AES_BLOCK_SIZE);

    // Increment the counter (CTR mode)
    increment_counter(cppState->ivec);
}

// General cleanup function for AES CTR PRNG
void aes_ctr_prng_general_cleanup(aes_ctr_state_t* state) {
    // No manual cleanup is needed; C++ will handle destruction when delete_aes_ctr_state is called
}

}  // extern "C"

