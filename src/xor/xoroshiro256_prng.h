/*
 * XORoshiro-256 PRNG Definitions
 * Autor: Fabian Druschke
 * Datum: 2024-03-13
 *
 * Diese Header-Datei enthält die Definitionen für die Implementierung des XORoshiro-256
 * Pseudorandom Number Generators (PRNG). XORoshiro-256 ist Teil der XORoshiro-Familie von PRNGs,
 * bekannt für ihre Einfachheit, Effizienz und qualitativ hochwertige Pseudorandom Number Generation,
 * geeignet für eine Vielzahl von Anwendungen, ausgenommen kryptographische Zwecke aufgrund ihrer
 * vorhersehbaren Natur.
 *
 * Als Autor dieser Arbeit gebe ich, Fabian Druschke, dieses Werk hiermit in die Public Domain frei.
 * Ich verzichte auf alle Urheberrechte an diesem Werk und stelle es der Öffentlichkeit frei zur
 * Verfügung, ohne jegliche Bedingungen, sofern solche Bedingungen nicht gesetzlich erforderlich sind.
 *
 * Diese Software wird "wie besehen" bereitgestellt, ohne jegliche ausdrückliche oder implizierte
 * Garantie, einschließlich, aber nicht beschränkt auf die Garantien der Marktgängigkeit, der Eignung
 * für einen bestimmten Zweck und der Nichtverletzung. In keinem Fall sind die Autoren haftbar für
 * irgendwelche Ansprüche, Schäden oder andere Haftungen, ob aus Vertrag, unerlaubter Handlung oder
 * anderweitig, die sich aus oder in Verbindung mit der Software oder der Nutzung oder anderen
 * Geschäften mit der Software ergeben.
 *
 * Hinweis: Diese Implementierung nutzt keine kryptographischen Bibliotheken, da XORoshiro-256 nicht
 * für kryptographische Anwendungen vorgesehen ist. Es ist wichtig, für Anwendungen, die kryptographische
 * Sicherheit erfordern, einen kryptographisch sicheren PRNG zu verwenden.
 */

#ifndef XOROSHIRO256_PRNG_H
#define XOROSHIRO256_PRNG_H

#include <stdint.h>

// Struktur zur Speicherung des Zustands des xoroshiro256** Zufallszahlengenerators
typedef struct xoroshiro256_state_s
{
    uint64_t s[4];
} xoroshiro256_state_t;

// Initialisiert den xoroshiro256** Zufallszahlengenerator mit einem Seed
void xoroshiro256_init( xoroshiro256_state_t* state, uint64_t init_key[], unsigned long key_length );

// Generiert eine 256-Bit-Zufallszahl mit xoroshiro256** und speichert sie im Ausgabepuffer
void xoroshiro256_genrand_uint256_to_buf( xoroshiro256_state_t* state, unsigned char* bufpos );

#endif  // XOROSHIRO256_PRNG_H

