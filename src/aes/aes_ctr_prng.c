#include "aes_ctr_prng.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h> // Für calloc und free
#include <stdbool.h> // Für den bool Datentyp


void aes_ctr_prng_init(aes_ctr_state_t* state, unsigned long init_key[], unsigned long key_length) {
    if (state == NULL) {
        return; // Sicherstellen, dass der Zustand nicht NULL ist
    }

    // Bereite den Schlüsselraum vor
    unsigned char key[32]; // Platz für einen 256-Bit-Schlüssel
    memset(key, 0, sizeof(key)); // Schlüssel initial Null setzen

    // Initialisiere IV (Initialisierungsvektor) und Zähler vor der Verwendung
    memset(state->ivec, 0, AES_BLOCK_SIZE);
    state->num = 0;
    memset(state->ecount, 0, AES_BLOCK_SIZE);

    // Verwende EVP für SHA-256 Hashing, um den Schlüssel aus dem bereitgestellten Seed zu generieren
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        state->is_initialized = false;
        return; // Frühes Beenden, wenn EVP_MD_CTX_new fehlschlägt
    }
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, (unsigned char*)init_key, key_length * sizeof(unsigned long));
    EVP_DigestFinal_ex(mdctx, key, NULL); // Generiere den endgültigen Schlüssel
    EVP_MD_CTX_free(mdctx);

    // Initialisiere den Verschlüsselungskontext mit dem AES-256-CTR-Modus
    state->ctx = EVP_CIPHER_CTX_new();
    if (state->ctx == NULL) {
        state->is_initialized = false;
        return; // Frühes Beenden, wenn EVP_CIPHER_CTX_new fehlschlägt
    }
    EVP_EncryptInit_ex(state->ctx, EVP_aes_256_ctr(), NULL, key, state->ivec);

    state->is_initialized = true; // Markiere als initialisiert
}

void aes_ctr_prng_genrand_uint128_to_buf(aes_ctr_state_t* state, unsigned char* bufpos) {
    if (state == NULL || !state->is_initialized) {
        return; // Sicherstellen, dass der Zustand initialisiert wurde
    }

    unsigned char temp_buffer[16]; // Zwischenpuffer für 128 Bits
    int outlen;

    // Generiere Pseudorandom-Nummern im Zwischenpuffer
    EVP_EncryptUpdate(state->ctx, temp_buffer, &outlen, temp_buffer, sizeof(temp_buffer));

    // Kopiere die generierten zufälligen Daten direkt in den Zielbuffer
    memcpy(bufpos, temp_buffer, 16);
}

void aes_ctr_prng_cleanup(aes_ctr_state_t* state) {
    if (state != NULL) {
        if (state->ctx != NULL) {
            EVP_CIPHER_CTX_free(state->ctx);
            state->ctx = NULL;
        }
        
        state->is_initialized = false; // Zurücksetzen des Initialisierungsstatus
    }
}

