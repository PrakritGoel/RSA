#include "numtheory.h"
#include "randstate.h"

gmp_randstate_t state;

int main() {
    mpz_t a, b, d, out;

    mpz_inits(a, b, d, out, NULL);
    randstate_init(0);

    printf("Testing gcd\n");
    mpz_set_ui(a, 240);
    mpz_set_ui(b, 104);
    gcd(out, a, b);
    gmp_printf("GCD of %Zd and %Zd = %Zd\n\n", a, b, out);

    printf("Testing pow_mod\n");
    mpz_set_ui(a, 241);
    mpz_set_ui(d, 3000);
    pow_mod(out, a, b, d);
    gmp_printf("%Zd ^ %Zd (mod %Zd) = %Zd\n", a, b, d, out);
    printf("Can be verified at the site "
           "https://www.mtholyoke.edu/courses/quenell/s2003/ma139/js/powermod.html\n\n");

    printf("Testing is_prime\n");
    mpz_set_ui(d, 999331);
    if (is_prime(d, 50)) {
        gmp_printf("%Zd is prime\n\n", d);
    } else {
        printf("Number is not prime\n\n");
    }

    // Testing modular inverse
    printf("Testing modular inverse. The answer should be 533\n");
    mpz_set_ui(a, 197);
    mpz_set_ui(b, 3000);
    mod_inverse(out, a, b);
    gmp_printf("Mod inverse of %Zd and %Zd = %Zd\n", a, b, out);
    printf("Can be verified at the site https://planetcalc.com/3311/\n\n");

    printf("Testing mod inverse again, since keygen is always giving 1 for private key\n");
    mpz_set_ui(a, 62);
    mpz_set_ui(b, 180);
    mod_inverse(out, a, b);
    gmp_printf("Mod inverse of %Zd and %Zd = %Zd\n\n", a, b, out);

    printf("Testing make_prime\n");
    make_prime(out, 130, 50);
    gmp_printf("Prime number of approx 130 bits = (%ld bits) %Zd\n", mpz_sizeinbase(out, 2), out);

    mpz_clears(a, b, d, out, NULL);
    return 0;
}
