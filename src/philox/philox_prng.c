#include <immintrin.h>  // Für AVX2-Instruktionen
#include <assert.h>
#include <string.h>
#include "philox_prng.h"



// Funktion zur Initialisierung des PRNG-Zustands mit einem Schlüssel
void philox_prng_init(philox_state_t* state, unsigned long init_key[], unsigned long key_length) {
    assert(state != NULL && init_key != NULL);

    // Initialisiere den Zähler mit Null
    state->counter = _mm256_setzero_si256();

    // Initialisiere den Schlüssel. Da wir AVX2 verwenden, passen maximal vier 64-Bit-Schlüssel in einen __m256i.
    // Fülle den Schlüsselbereich auf, falls key_length < 4 ist.
    if (key_length >= 4) {
        state->key = _mm256_set_epi64x(init_key[3], init_key[2], init_key[1], init_key[0]);
    } else {
        unsigned long temp_key[4] = {0, 0, 0, 0};  // Fülle mit Nullen auf
        for (unsigned long i = 0; i < key_length; ++i) {
            temp_key[i] = init_key[i];
        }
        state->key = _mm256_set_epi64x(temp_key[3], temp_key[2], temp_key[1], temp_key[0]);
    }
}

// Funktion zur Erzeugung von 512 Bit Zufallszahlen und Kopieren in einen Puffer
void philox_prng_genrand_uint512_to_buf(philox_state_t* state, unsigned char* bufpos) {
    assert(state != NULL && bufpos != NULL);  // Validiere Eingaben

    // Temporärer Puffer für 512 Bit (64 Bytes)
    unsigned char temp_buffer[64];
    memset(temp_buffer, 0, sizeof(temp_buffer));  // Initialisiere temporären Puffer mit Nullen

    // Philox-Kernberechnung:
    __m256i multiplier = _mm256_set1_epi64x(0xD2B74407B1CE6E93);  // Philox-Konstante für Multiplikation
    __m256i increment = _mm256_set1_epi64x(0x9E3779B97F4A7C15);   // Philox-Konstante für Inkrementierung

    // Hauptschleife, die den Zähler und Schlüssel transformiert (Philox-Kern, z.B. 10 Runden)
    for (int i = 0; i < 10; ++i) {
        // Untere 32 Bit multiplizieren und obere 32 Bit für weitere Operationen nutzen
        __m256i lo = _mm256_mul_epu32(state->counter, multiplier);  // Untere 32 Bit multiplizieren
        __m256i hi = _mm256_srli_epi64(state->counter, 32);         // Höhere 32 Bit verschieben

        // Schlüssel und Zähler modifizieren
        state->counter = _mm256_add_epi64(state->counter, increment);
        state->key = _mm256_xor_si256(state->key, hi);  // XOR mit den oberen Bits des Zählers
    }

    // Die Ergebnisse in den temporären Puffer kopieren
    _mm256_storeu_si256((__m256i*)temp_buffer, state->counter);      // Speichere die ersten 256 Bit
    _mm256_storeu_si256((__m256i*)(temp_buffer + 32), state->key);   // Speichere die zweiten 256 Bit

    // Kopiere die generierten Pseudozufallsdaten in den Zielpuffer
    memcpy(bufpos, temp_buffer, sizeof(temp_buffer));
}


