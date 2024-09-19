/*
 * philox_prng.h
 * Header-Datei für die optimierte Philox PRNG Implementierung mit AVX2
 * Autor: [Ihr Name]
 * Datum: [Aktuelles Datum]
 *
 * Dieses Werk ist gemeinfrei. Es kann von jedermann für beliebige Zwecke
 * genutzt werden, ohne jegliche Bedingungen, es sei denn, solche Bedingungen
 * sind gesetzlich vorgeschrieben.
 */

#ifndef PHILOX_PRNG_H
#define PHILOX_PRNG_H

#include <stdint.h>
#include <immintrin.h>  // Für AVX2-Intrinsics

#ifdef __cplusplus
extern "C" {
#endif

// Anzahl der Runden für den Philox 4x32 Algorithmus
#define NUM_ROUNDS 10  // Anzahl der Runden für Philox 4x32


typedef struct philox_state_s {
    __m256i counter_lo;  // Untere 128 Bit des Zählers
    __m256i counter_hi;  // Obere 128 Bit des Zählers
    __m256i key;         // 256-Bit Schlüssel für den PRNG
} philox_state_t;

/* Initialisiert den Philox PRNG Zustand.
   - state: Zeiger auf den Philox PRNG Zustandsstruktur.
   - init_key: Array, das den Seed-Schlüssel enthält.
   - key_length: Länge des Schlüsselarrays. */
void philox_prng_init(philox_state_t* state, unsigned long init_key[], unsigned long key_length);

/* Generiert Pseudorandom-Zahlen und schreibt sie in einen Puffer.
   - state: Zeiger auf den initialisierten Philox PRNG Zustand.
   - bufpos: Zielpuffer, in den die Pseudorandom-Zahlen geschrieben werden.
     Der Puffer sollte mindestens 64 Bytes groß sein. */
void philox_prng_genrand_uint512_to_buf(philox_state_t* state, unsigned char* bufpos);

#ifdef __cplusplus
}
#endif

#endif // PHILOX_PRNG_H

