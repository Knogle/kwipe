#include <immintrin.h>  // Für AVX2-Instrinsics
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "philox_prng.h"

#define NUM_ROUNDS 10  // Anzahl der Runden für Philox 4x32

// Helper-Funktion zur Berechnung der oberen 32 Bits der 32-Bit-Multiplikation
static inline __m256i mulhi_epu32(__m256i a, __m256i b)
{
    __m256i a_lo = _mm256_and_si256(a, _mm256_set1_epi64x(0xFFFFFFFF));
    __m256i b_lo = _mm256_and_si256(b, _mm256_set1_epi64x(0xFFFFFFFF));
    __m256i product_lo = _mm256_mul_epu32(a_lo, b_lo);  // Multipliziert die unteren 32-Bit-Werte

    __m256i a_hi = _mm256_srli_epi64(a, 32);
    __m256i b_hi = _mm256_srli_epi64(b, 32);
    __m256i product_hi = _mm256_mul_epu32(a_hi, b_hi);  // Multipliziert die oberen 32-Bit-Werte

    __m256i high_lo = _mm256_srli_epi64(product_lo, 32);
    __m256i high_hi = _mm256_srli_epi64(product_hi, 32);

    return _mm256_or_si256(high_lo, _mm256_slli_epi64(high_hi, 32));
}

// Initialisierung des PRNG-Zustands
void philox_prng_init(philox_state_t* state, unsigned long init_key[], unsigned long key_length)
{
    assert(state != NULL && init_key != NULL && key_length >= 4);

    uint32_t key_data[8] = {0};  // Temporärer Puffer für den Schlüssel, keine Ausrichtung erforderlich
    for (size_t i = 0; i < 8 && i < key_length; ++i) {
        key_data[i] = (uint32_t)(init_key[i] & 0xFFFFFFFF);
    }
    state->key = _mm256_loadu_si256((__m256i*)key_data);  // Verwende "u"-Variante für unaligned Load

    state->counter_lo = _mm256_setzero_si256();
    state->counter_hi = _mm256_setzero_si256();
}

// Eine Runde des Philox 4x32 PRNG
static inline void philox_round(__m256i* counter_lo, __m256i* counter_hi, __m256i* key)
{
    const __m256i M0 = _mm256_set1_epi32(0xD256D193);
    const __m256i M1 = _mm256_set1_epi32(0x9E3779B9);

    __m256i x0 = *counter_lo;
    __m256i x1 = *counter_hi;

    __m256i lo0 = _mm256_mullo_epi32(x0, M0);
    __m256i hi0 = mulhi_epu32(x0, M0);

    __m256i lo1 = _mm256_mullo_epi32(x1, M1);
    __m256i hi1 = mulhi_epu32(x1, M1);

    __m256i new_x0 = _mm256_xor_si256(hi1, _mm256_add_epi32(_mm256_shuffle_epi32(*counter_lo, _MM_SHUFFLE(0, 0, 3, 2)), *key));
    __m256i new_x1 = lo1;
    __m256i new_x2 = _mm256_xor_si256(hi0, _mm256_add_epi32(_mm256_shuffle_epi32(*counter_hi, _MM_SHUFFLE(0, 0, 1, 0)), *key));
    __m256i new_x3 = lo0;

    *counter_lo = _mm256_blend_epi32(new_x2, new_x3, 0xF0);
    *counter_hi = _mm256_blend_epi32(new_x0, new_x1, 0x0F);

    const __m256i KEY_INCREMENT = _mm256_set1_epi32(0x9E3779B9);
    *key = _mm256_add_epi32(*key, KEY_INCREMENT);
}

// Generiert Pseudorandom-Zahlen und schreibt sie in einen Puffer
void philox_prng_genrand_uint512_to_buf(philox_state_t* state, unsigned char* bufpos)
{
    assert(state != NULL && bufpos != NULL);

    __m256i temp_buffer[2];
    memset(temp_buffer, 0, sizeof(temp_buffer));  // Initialisierung des temporären Puffers

    __m256i counter_lo = state->counter_lo;
    __m256i counter_hi = state->counter_hi;
    __m256i key = state->key;

    // Philox-Runden ausführen
    for (int i = 0; i < NUM_ROUNDS; ++i)
    {
        philox_round(&counter_lo, &counter_hi, &key);
    }

    // Temporären Puffer in den Ausgabepuffer schreiben, ohne dass eine Ausrichtung notwendig ist
    _mm256_storeu_si256((__m256i*)bufpos, counter_lo);         // Unaligned Store
    _mm256_storeu_si256((__m256i*)(bufpos + 32), counter_hi);  // Unaligned Store

    // Zähler inkrementieren
    __m256i one = _mm256_set1_epi64x(1);
    state->counter_lo = _mm256_add_epi64(state->counter_lo, one);

    // Überlaufbehandlung
    __m256i zero = _mm256_setzero_si256();
    __m256i mask = _mm256_cmpeq_epi64(state->counter_lo, zero);
    state->counter_hi = _mm256_add_epi64(state->counter_hi, mask);
}

