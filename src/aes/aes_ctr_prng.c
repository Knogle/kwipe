
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

#include "aes_ctr_prng.h"
#include <openssl/rand.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/modes.h>
#include <openssl/sha.h>  // Include for SHA256

void aes_ctr_prng_init(aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length) {
    unsigned char key[32];  // Size for 256 bits
    unsigned char iv_seed[32];  // Zusätzlicher Seed für IV

    // Verwende SHA256, um den Schlüssel zu generieren
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, (unsigned char*) init_key, key_length * sizeof(unsigned long));  // Füge den init_key hinzu
    SHA256_Final(key, &sha256);  // Generiere den finalen Schlüssel

    // Optional: Erzeuge einen deterministischen IV aus dem Seed, falls gewünscht
    SHA256_Init(&sha256);
    // Optional: Füge eine Konstante oder eine Variation hinzu, um sicherzustellen, dass der IV anders ist als der Hauptkey
    SHA256_Update(&sha256, (unsigned char*) init_key, key_length * sizeof(unsigned long));
    SHA256_Final(iv_seed, &sha256);  // Generiere IV Seed

    // Nutze die ersten 16 Bytes des IV Seeds als IV (für AES-256)
    memcpy(state->ivec, iv_seed, AES_BLOCK_SIZE);

    AES_set_encrypt_key(key, 256, &state->aes_key);  // Nutze den 256-Bit-Schlüssel
    state->num = 0;
    memset(state->ecount, 0, AES_BLOCK_SIZE);
}

static void next_state(aes_ctr_state_t* state) {
    for (int i = 0; i < AES_BLOCK_SIZE; ++i) {
        if (++state->ivec[i])
            break;
    }
}

void aes_ctr_prng_genrand_uint128_to_buf(aes_ctr_state_t* state, unsigned char* bufpos) {
    // Setze ivec, ecount, und num zu Beginn jeder Operation zurück
    memset(state->ivec, 0, AES_BLOCK_SIZE);  // Setze den IV auf Nullen
    memset(state->ecount, 0, AES_BLOCK_SIZE);  // Setze den ecount auf Nullen
    state->num = 0;  // Setze num auf 0

    // Generiere direkt Zufallszahlen in bufpos, ohne einen separaten Ausgabepuffer zu verwenden
    CRYPTO_ctr128_encrypt(
        bufpos, bufpos, 16, &state->aes_key, state->ivec, state->ecount, &state->num, (block128_f)AES_encrypt);

    // Stelle sicher, dass next_state korrekt aufgerufen wird, ohne Fehler zu verursachen
    next_state(state);
}

