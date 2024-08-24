#ifndef ENTROPY_H
#define ENTROPY_H

#include <stdint.h>  // Für die Definition von uint64_t

// Rückgabecodes für Initialisierungsfunktionen
#define ENTROPY_INIT_SUCCESS 1
#define ENTROPY_INIT_FAILURE 0
#define ENTROPY_VALIDATION_FAILURE -1

// Prototypen der Entropieüberprüfungsfunktionen
double shannon_entropy(uint64_t num);
double bit_frequency_test(uint64_t num);
int runs_test(uint64_t num);
double auto_correlation_test(uint64_t num);

// Funktion zur Überprüfung der Entropie eines Zahl anhand verschiedener statistischer Methoden
int nwipe_check_entropy(uint64_t num);

// Prototypen der Entropiefunktionen für /dev/urandom
int nwipe_entropy_dev_urandom_init(void);
uint64_t nwipe_entropy_dev_urandom_read(void);

// Prototypen der Entropiefunktionen für OpenSSL
int nwipe_entropy_openssl_init(void);
uint64_t nwipe_entropy_openssl_read(void);

// Prototypen der Entropiefunktionen für RDSEED
int nwipe_entropy_rdseed_init(void);
uint64_t nwipe_entropy_rdseed_read(void);

// Strukturdefinition für PRNG
typedef struct {
    const char *name;
    int (*init)(void);
    uint64_t (*read)(void);
} nwipe_entropy_algorithm_t;

// Externe Deklarationen der Strukturinstanzen für die verschiedenen Entropiequellen
extern nwipe_entropy_algorithm_t nwipe_entropy_dev_urandom;
extern nwipe_entropy_algorithm_t nwipe_entropy_openssl;
extern nwipe_entropy_algorithm_t nwipe_entropy_rdseed;

#endif  // ENTROPY_H

