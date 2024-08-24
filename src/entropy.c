#include "entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <immintrin.h>  // Für RDSEED Intrinsics
#include <openssl/rand.h>  // OpenSSL für die zweite Entropiequelle

#define NWIPE_KNOB_ENTROPY "/dev/urandom"
#define N 64  // Number of bits in a uint64_t

static int nwipe_entropy_fd = -1;

typedef enum {
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

extern void nwipe_log(nwipe_log_t level, const char* format, ...);

// Shannon entropy calculation
double shannon_entropy(uint64_t num) {
    int bit_count[2] = {0, 0};

    for (int i = 0; i < N; i++) {
        bit_count[(num >> i) & 1]++;
    }

    double p0 = (double)bit_count[0] / N;
    double p1 = (double)bit_count[1] / N;

    if (p0 == 0.0 || p1 == 0.0) {
        return 0.0;
    }

    return -p0 * log2(p0) - p1 * log2(p1);
}

// Bit frequency test
double bit_frequency_test(uint64_t num) {
    int count_ones = 0;

    for (int i = 0; i < N; i++) {
        if ((num >> i) & 1) {
            count_ones++;
        }
    }

    return (double)count_ones / N;
}

// Runs test
int runs_test(uint64_t num) {
    int runs = 1;
    int prev_bit = num & 1;

    for (int i = 1; i < N; i++) {
        int current_bit = (num >> i) & 1;
        if (current_bit != prev_bit) {
            runs++;
            prev_bit = current_bit;
        }
    }

    return runs;
}

// Auto-correlation test
double auto_correlation_test(uint64_t num) {
    int matches = 0;

    for (int i = 0; i < N - 1; i++) {
        if (((num >> i) & 1) == ((num >> (i + 1)) & 1)) {
            matches++;
        }
    }

    return (double)matches / (N - 1);
}

// Entropy check function
int nwipe_check_entropy(uint64_t num) {
    double entropy = shannon_entropy(num);
    if (entropy == 0.0) {
        nwipe_log(NWIPE_LOG_FATAL, "Entropy calculation failed. All bits are identical.");
        return 0;
    }

    double frequency = bit_frequency_test(num);
    int runs = runs_test(num);
    double correlation = auto_correlation_test(num);

    nwipe_log(NWIPE_LOG_INFO, "Shannon Entropy: %f", entropy);
    nwipe_log(NWIPE_LOG_INFO, "Bit Frequency (proportion of 1s): %f", frequency);
    nwipe_log(NWIPE_LOG_INFO, "Number of Runs: %d", runs);
    nwipe_log(NWIPE_LOG_INFO, "Auto-correlation: %f", correlation);

    if (entropy > 0.9 && frequency > 0.3 && frequency < 0.7 && runs > 10 && runs < 54 && correlation < 0.7) {
        nwipe_log(NWIPE_LOG_INFO, "Entropy check passed. Sufficient randomness detected. Entropy: %f", entropy);
        return 1;
    } else {
        nwipe_log(NWIPE_LOG_ERROR, "Entropy check failed. Insufficient randomness. Entropy: %f", entropy);
        return 0;
    }
}

// /dev/urandom Initialisierungsfunktion
int nwipe_entropy_dev_urandom_init(void) {
    nwipe_entropy_fd = open(NWIPE_KNOB_ENTROPY, O_RDONLY);

    if (nwipe_entropy_fd < 0) {
        nwipe_perror(errno, __FUNCTION__, "open");
        nwipe_log(NWIPE_LOG_FATAL, "Unable to open entropy source %s.", NWIPE_KNOB_ENTROPY);
        return ENTROPY_INIT_FAILURE;
    }

    nwipe_log(NWIPE_LOG_NOTICE, "Opened entropy source '%s'.", NWIPE_KNOB_ENTROPY);

    uint64_t entropy_sample;
    if (read(nwipe_entropy_fd, &entropy_sample, sizeof(entropy_sample)) != sizeof(entropy_sample)) {
        nwipe_log(NWIPE_LOG_FATAL, "Failed to read sufficient entropy from the source.");
        close(nwipe_entropy_fd);
        nwipe_entropy_fd = -1;
        return ENTROPY_INIT_FAILURE;
    }

    if (nwipe_check_entropy(entropy_sample) == 0) {
        nwipe_log(NWIPE_LOG_FATAL, "Entropy validation failed. Insufficient randomness detected.");
        close(nwipe_entropy_fd);
        nwipe_entropy_fd = -1;
        return ENTROPY_VALIDATION_FAILURE;
    } else {
        nwipe_log(NWIPE_LOG_INFO, "Entropy validation passed. Sufficient randomness detected.");
    }

    return ENTROPY_INIT_SUCCESS;
}

// /dev/urandom Lesefunktion
uint64_t nwipe_entropy_dev_urandom_read(void) {
    uint64_t value = 0;
    if (nwipe_entropy_fd == -1) {
        fprintf(stderr, "Entropy source not initialized\n");
        return 0;
    }

    ssize_t result = read(nwipe_entropy_fd, &value, sizeof(value));
    if (result != sizeof(value)) {
        perror("Failed to read from entropy source");
        return 0;
    }

    return value;
}

// OpenSSL Initialisierungsfunktion
int nwipe_entropy_openssl_init(void) {
    if (RAND_poll() == 0) {
        nwipe_log(NWIPE_LOG_FATAL, "OpenSSL PRNG initialization failed.");
        return ENTROPY_INIT_FAILURE;
    }

    nwipe_log(NWIPE_LOG_NOTICE, "Initialized OpenSSL PRNG.");

    uint64_t entropy_sample;
    if (RAND_bytes((unsigned char*)&entropy_sample, sizeof(entropy_sample)) != 1) {
        nwipe_log(NWIPE_LOG_FATAL, "Failed to read sufficient entropy from OpenSSL PRNG.");
        return ENTROPY_INIT_FAILURE;
    }

    if (nwipe_check_entropy(entropy_sample) == 0) {
        nwipe_log(NWIPE_LOG_FATAL, "Entropy validation failed. Insufficient randomness detected.");
        return ENTROPY_VALIDATION_FAILURE;
    } else {
        nwipe_log(NWIPE_LOG_INFO, "Entropy validation passed. Sufficient randomness detected.");
    }

    return ENTROPY_INIT_SUCCESS;
}

// OpenSSL Lesefunktion
uint64_t nwipe_entropy_openssl_read(void) {
    uint64_t value = 0;
    if (RAND_bytes((unsigned char*)&value, sizeof(value)) != 1) {
        perror("Failed to read from OpenSSL entropy source");
        return 0;
    }

    return value;
}
/*
// RDSEED Initialisierungsfunktion
int nwipe_entropy_rdseed_init(void) {
    unsigned int eax, ebx, ecx, edx;
    eax = 7;
    ecx = 0;

    __asm__ volatile("cpuid"
                     : "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(eax), "c"(ecx));

    if (!(ecx & (1 << 18))) {
        nwipe_log(NWIPE_LOG_FATAL, "RDSEED not supported on this CPU.");
        return ENTROPY_INIT_FAILURE;
    }

    nwipe_log(NWIPE_LOG_NOTICE, "RDSEED supported and available.");

    uint64_t entropy_sample;
    if (nwipe_entropy_rdseed_read() == 0) {
        nwipe_log(NWIPE_LOG_FATAL, "Failed to read sufficient entropy from RDSEED.");
        return ENTROPY_INIT_FAILURE;
    }

    if (nwipe_check_entropy(entropy_sample) == 0) {
        nwipe_log(NWIPE_LOG_FATAL, "Entropy validation failed. Insufficient randomness detected.");
        return ENTROPY_VALIDATION_FAILURE;
    } else {
        nwipe_log(NWIPE_LOG_INFO, "Entropy validation passed. Sufficient randomness detected.");
    }

    return ENTROPY_INIT_SUCCESS;
}

// RDSEED Lesefunktion
uint64_t nwipe_entropy_rdseed_read(void) {
    uint64_t value = 0;
    int success = 0;

    for (int i = 0; i < 10; i++) {
        success = _rdseed64_step(&value);
        if (success) {
            return value;
        }
    }

    nwipe_log(NWIPE_LOG_FATAL, "RDSEED failed to generate a random value after multiple attempts.");
    return 0;
}
*/
// Instanz der Struktur für /dev/urandom
nwipe_entropy_algorithm_t nwipe_entropy_dev_urandom = {
    "Linux Kernel (/dev/urandom)",
    nwipe_entropy_dev_urandom_init,
    nwipe_entropy_dev_urandom_read
};

// Instanz der Struktur für OpenSSL
nwipe_entropy_algorithm_t nwipe_entropy_openssl = {
    "OpenSSL PRNG",
    nwipe_entropy_openssl_init,
    nwipe_entropy_openssl_read
};

/*
// Instanz der Struktur für RDSEED
nwipe_entropy_algorithm_t nwipe_entropy_rdseed = {
    "x86 RDSEED",
    nwipe_entropy_rdseed_init,
    nwipe_entropy_rdseed_read
};

*/
