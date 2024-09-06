#include <stdio.h>
#include "../include/SFMT-src-1.5.1/SFMT.h"  // SFMT-Header-Datei
#include <stdint.h> // Für uint32_t

// Definition des Zustands für den Zufallszahlengenerator
typedef struct {
    sfmt_t sfmt_state;  // Der SFMT-Zustand
    int initf;  // Flag zur Überprüfung, ob der Generator initialisiert wurde
} twister_state_t;

/* Initialisiert state mit einem Seed */
void init_genrand(twister_state_t* state, unsigned long s) {
    sfmt_init_gen_rand(&state->sfmt_state, s);  // Initialisiert den SFMT-Zustand
    state->initf = 1;  // Markiert als initialisiert
}

/* Initialisiert state mit einem Array von Seed-Werten */
void twister_init(twister_state_t* state, unsigned long init_key[], unsigned long key_length) {
    uint32_t key32[key_length];  // Erstelle ein Array von uint32_t

    // Konvertiere unsigned long in uint32_t
    for (unsigned long i = 0; i < key_length; ++i) {
        key32[i] = (uint32_t)init_key[i];
    }

    // Initialisiere SFMT mit dem konvertierten Array
    sfmt_init_by_array(&state->sfmt_state, key32, key_length);
    state->initf = 1;  // Markiert als initialisiert
}

/* Generiert eine Zufallszahl auf dem Intervall [0, 0xffffffff] */
unsigned long twister_genrand_int32(twister_state_t* state) {
    // Überprüfen, ob der Generator initialisiert wurde
    if (state->initf == 0) {
        init_genrand(state, 5489UL);  // Standard-Seed, wenn nicht initialisiert
    }

    return sfmt_genrand_uint32(&state->sfmt_state);  // Generiert eine 32-Bit Zufallszahl
}
