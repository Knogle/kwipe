/*
 * XORoshiro-256 PRNG Implementation
 * Autor: Fabian Druschke
 * Datum: 2024-03-13
 *
 * Dies ist eine Implementierung des XORoshiro-256 (XOR/rotate/shift/rotate) Pseudorandom Number Generators
 * (PRNG), entwickelt für schnelle und effiziente Erzeugung von qualitativ hochwertigen Pseudorandom Numbers.
 * XORoshiro-256 ist Teil der XORoshiro-Familie von PRNGs, bekannt für ihre Einfachheit und hervorragenden
 * statistischen Eigenschaften für eine Vielzahl von Anwendungen, obwohl sie aufgrund ihrer Vorhersehbarkeit
 * nicht für kryptographische Zwecke geeignet sind.
 *
 * Als Autor dieser Implementierung gebe ich, Fabian Druschke, dieses Werk hiermit in die Public Domain frei.
 * Ich verzichte auf alle Urheberrechte an diesem Werk und stelle es der Öffentlichkeit frei zur Verfügung,
 * ohne jegliche Bedingungen, sofern solche Bedingungen nicht gesetzlich erforderlich sind.
 *
 * Diese Software wird "wie besehen" bereitgestellt, ohne jegliche ausdrückliche oder implizierte Garantie,
 * einschließlich, aber nicht beschränkt auf die Garantien der Marktgängigkeit, der Eignung für einen
 * bestimmten Zweck und der Nichtverletzung. In keinem Fall sind die Autoren haftbar für irgendwelche
 * Ansprüche, Schäden oder andere Haftungen, ob aus Vertrag, unerlaubter Handlung oder anderweitig, die
 * sich aus oder in Verbindung mit der Software oder der Nutzung oder anderen Geschäften mit der Software
 * ergeben.
 *
 * Hinweis: Diese Implementierung nutzt keine kryptographischen Bibliotheken, da XORoshiro-256 nicht für
 * kryptographische Anwendungen vorgesehen ist. Es ist wichtig, für Anwendungen, die kryptographische
 * Sicherheit erfordern, einen kryptographisch sicheren PRNG zu verwenden.
 */

#include "xoroshiro256_prng.h"
#include <stdint.h>
#include <string.h>

// Rotationsfunktion
static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

// Initialisierungsfunktion
void xoroshiro256_init(xoroshiro256_state_t* state, uint64_t init_key[], unsigned long key_length)
{
    // Initialisierungslogik; stellt sicher, dass 256 Bit korrekt gesetzt sind
    for (int i = 0; i < 4; i++)
    {
        if (i < key_length)
        {
            state->s[i] = init_key[i];
        }
        else
        {
            // Fallback für unzureichende Seeds; bessere Strategien können hier eingesetzt werden
            state->s[i] = 0x9E3779B97F4A7C15ULL * (state->s[i - 1] ^ (state->s[i - 1] >> 27)) + i;
        }
    }
}

// PRNG-Funktion zur Generierung eines 256-Bit-Zufallswerts
void xoroshiro256_genrand_uint256_to_buf(xoroshiro256_state_t* state, unsigned char* bufpos)
{
    uint64_t results[4];
    for (int i = 0; i < 4; i++)
    {
        // Generieren der Zufallszahl gemäß xoroshiro256**
        const uint64_t result_starstar = rotl(state->s[1] * 5, 7) * 9;
        results[i] = result_starstar;

        // Aktualisierung des Zustands
        const uint64_t t = state->s[1] << 17;

        state->s[2] ^= state->s[0];
        state->s[3] ^= state->s[1];
        state->s[1] ^= state->s[2];
        state->s[0] ^= state->s[3];

        state->s[2] ^= t;
        state->s[3] = rotl(state->s[3], 45);
    }

    // Kopieren der generierten Zufallszahlen in den Puffer
    memcpy(bufpos, results, 32);  // Kopiert die 256-Bit-Zufallszahlen in 'bufpos'
}

