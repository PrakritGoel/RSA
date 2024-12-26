#include "numtheory.h"
#include "randstate.h"

// Calculates the gcd of a and b using Euler's recursive algorithm
//
// Input parameters:
// d: mpz_t: The gcd is stored here
// a, b: mpz_t
// Returns: void
void gcd(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t t, a1, b1;
    mpz_inits(t, a1, b1, NULL);

    mpz_set(a1, a);
    mpz_set(b1, b);

    while (mpz_cmp_ui(b1, 0)) {
        mpz_set(t, b1);
        mpz_mod(b1, a1, b1);
        mpz_set(a1, t);
    }

    mpz_set(d, a1);
    mpz_clears(t, a1, b1, NULL);
    return;
}

// Computes the inverse i of a modulo n
// Input parameters:
//
// i, a, n: mpz_t
// Returns: void
void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    mpz_t r1, r2, t1, t2, q, tmp1, tmp2;
    mpz_inits(r1, r2, t1, t2, q, tmp1, tmp2, NULL);

    mpz_set(r1, n);
    mpz_set(r2, a);
    mpz_set_ui(t1, 0);
    mpz_set_ui(t2, 1);

    while (mpz_cmp_ui(r2, 0)) {
        mpz_fdiv_q(q, r1, r2);
        mpz_mul(tmp1, q, r2);
        mpz_sub(tmp2, r1, tmp1);
        mpz_set(r1, r2);
        mpz_set(r2, tmp2);

        mpz_mul(tmp1, q, t2);
        mpz_sub(tmp2, t1, tmp1);
        mpz_set(t1, t2);
        mpz_set(t2, tmp2);
    }

    if (mpz_cmp_ui(r1, 1) > 0) {
        mpz_clears(r1, r2, t1, t2, q, tmp1, tmp2, NULL);
        mpz_set_ui(i, 1);
        return;
    }

    if (mpz_cmp_ui(t1, 0) < 0) {
        mpz_add(t1, t1, n);
    }

    mpz_set(i, t1);
    mpz_clears(r1, r2, t1, t2, q, tmp1, tmp2, NULL);
    return;
}

// Performs fast modular exponentiation, computing base raised to the
// exponent power modulo modulus, and storing the computed result in out.
//
// Input parameters:
// out: mpz_t: Stores the results
// base: mpz_t:
// base: exponent:
// base: modulus:
//
// Returns: void
void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_t e, v, p, rop;
    mpz_inits(e, v, p, rop, NULL);
    mpz_set_ui(v, 1);
    mpz_set(p, base);
    mpz_set(e, exponent);

    while (mpz_cmp_ui(e, 0) > 0) {
        if (mpz_odd_p(e)) {
            mpz_mul(rop, v, p);
            mpz_mod(v, rop, modulus);
        }
        mpz_mul(rop, p, p);
        mpz_mod(p, rop, modulus);
        mpz_fdiv_q_ui(e, e, 2);
    }
    mpz_set(out, v);
    mpz_clears(e, v, p, rop, NULL);
    return;
}

// Conducts the Miller-Rabin primality test to indicate whether or not n
// is prime using iters number of Miller-Rabin iterations.
//
// Input parameters:
// n: mpz_t: Number to check for primality
// iters: uint64_t: Number of Miller-Rabin iterations
// Returns: bool: True if prime. False otherwise
bool is_prime(mpz_t n, uint64_t iters) {
    mpz_t y, a, r, n_minus_1, two_mpz;
    uint64_t s;

    mpz_inits(y, a, r, n_minus_1, two_mpz, NULL);

    // Set n_minus_1 to n-1
    mpz_sub_ui(n_minus_1, n, 1);

    // Set an mpz_t variable to value 2. To be used in pow_mod
    mpz_set_ui(two_mpz, 2);

    // First step of the algo requires us to identify an r such that
    // we write (n-1 = 2^s r), where r is odd.
    mpz_fdiv_q_ui(r, n_minus_1, 2);
    s = 1;

    while (!mpz_odd_p(r)) {
        s++;
        mpz_fdiv_q_ui(r, r, 2);
    }

    for (uint64_t i = 0; i < iters; i++) {
        // choose random a ∈ {2,3,...,n − 2}

        // To achieve this, we call mpz_urandomm(). However, this generates
        // random numbers from 0,...,n-1. Consequently, we first initialize
        // a temporary variable to n-4, generate the random number, and
        // add 2 to it.
        mpz_sub_ui(a, n, 4);
        mpz_urandomm(a, state, a);
        mpz_add_ui(a, a, 2);

        pow_mod(y, a, r, n);

        // If y != 1 and y != n-1
        if (mpz_cmp_ui(y, 1) && mpz_cmp(y, n_minus_1)) {
            uint64_t j = 1;

            // While j < s and y != n-1
            while (j < s && mpz_cmp(y, n_minus_1)) {
                pow_mod(y, y, two_mpz, n);

                // if y == 1
                if (!mpz_cmp_ui(y, 1)) {
                    mpz_clears(y, a, r, n_minus_1, two_mpz, NULL);
                    return false;
                }
                j++;
            }

            // if y != n-1
            if (mpz_cmp(y, n_minus_1)) {
                mpz_clears(y, a, r, n_minus_1, two_mpz, NULL);
                return false;
            }
        }
    }
    mpz_clears(y, a, r, n_minus_1, two_mpz, NULL);
    return true;
}

// Generates a mersenne prime (of the form (2^n)-1 where n >= bits
// It uses is_prime to check for primality with the given number
// of iterations.
//
// Input parameters:
// p: mpz_t: Prime number is stored here
// bits: uint64_t: Minimum number of bits in the generated number
// iters: uint64_t: Number of iterations to validate primarily
// Returns: void
/*
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t two_mpz;
    mpz_init(two_mpz);

    // Create an mpz_t variable with value 2 and raise it to the desired power
    mpz_set_ui(two_mpz, 2);
    mpz_pow_ui(p, two_mpz, bits-1);

    // Subtract 1 from p to generate a potential mersenne prime number
    mpz_sub_ui(p, p, 1);

    // Keep incrementing the bits till we get a prime number
    while (! is_prime(p, iters)) {
        bits++;
        mpz_pow_ui(p, two_mpz, bits);
        mpz_sub_ui(p, p, 1);
    }

    mpz_clear(two_mpz);
    return;
}
*/
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t two_mpz;
    mpz_init(two_mpz);

    // Create an mpz_t variable with value 2 and raise it to the desired power
    mpz_set_ui(two_mpz, 2);
    mpz_pow_ui(p, two_mpz, bits + 1);

    // Subtract 1 from p to generate a potential mersenne prime number
    mpz_add_ui(p, p, 1);

    // Keep incrementing the bits till we get a prime number
    while (!is_prime(p, iters)) {
        /*
        bits++;
        mpz_pow_ui(p, two_mpz, bits);
        */
        mpz_add_ui(p, p, 2);
    }

    mpz_clear(two_mpz);
    return;
}
