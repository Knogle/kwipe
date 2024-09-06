#include <stdio.h>
#include <emmintrin.h>  // Für SSE2-Operationen
#include "SFMT.h"
#include "SFMT-sse2.h"  // Optimierung für SSE2
#include "SFMT-params19937.h"  // Periode 19937

int main() {
    // Initialisiere den SFMT-Zufallszahlengenerator
    sfmt_t sfmt;
    sfmt_init_gen_rand(&sfmt, 12345);  // Setze einen Seed

    // Generiere 10 Zufallszahlen und gib sie aus
    for (int i = 0; i < 10; ++i) {
        uint32_t random_number = sfmt_genrand_uint32(&sfmt);
        printf("%u\n", random_number);
    }

    return 0;
}
