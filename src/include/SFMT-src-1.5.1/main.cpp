#include <iostream>
#include <emmintrin.h>  // Für SSE2-Operationen
extern "C" {
    #include "SFMT.h"  // Die C-Header für SFMT
}

int main() {
    sfmt_t sfmt;
    sfmt_init_gen_rand(&sfmt, 12345);  // Seed setzen

    // Generiere und gib 10 Zufallszahlen aus
    for (int i = 0; i < 10; ++i) {
        uint32_t random_number = sfmt_genrand_uint32(&sfmt);
        std::cout << random_number << std::endl;
    }

    return 0;
}
