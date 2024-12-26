#include "randstate.h"
#include <stdlib.h>

// Input Parameters:
// seed: uint64_t: Use seed as the random seed
// Returns: void
void randstate_init(uint64_t seed) {
    srandom(seed);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
    return;
}

// Clears and frees the memory used by state.
//
// Input parameters: None
// Returns: void
void randstate_clear(void) {
    gmp_randclear(state);
    return;
}
