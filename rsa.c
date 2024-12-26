#include "rsa.h"
#include "numtheory.h"
#include "randstate.h"
#include <stdlib.h>

// Creates an RSA public key. Two large prime numbers p and q, their product
// n, and the public exponent e.
//
// Input parameters:
// p: mpz_t: Prime number to be generated
// q: mpz_t: Prime number to be generated
// n: mpz_t: n = pq
// e: mpz_t: Exponent
// nbits: uint64_t: Minimum number of bits for n
// iters: uint64_t: Number of iterations to be used for primality test
// Returns: void
void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    mpz_t tmp1, tmp2, tmp3, tmp4, lambda;
    mpz_inits(tmp1, tmp2, tmp3, tmp4, lambda, NULL);

    // Use a number in the interval [nbits/4, 3*nbits/4] as bit length for p,
    // and the rest for q.
    uint64_t p_len = nbits / 4 + (random() % nbits) / 2;
    uint64_t q_len = nbits - p_len;

    make_prime(p, p_len, iters);
    make_prime(q, q_len, iters);

    // Set tmp1 to p-1 and tmp2 to q-1
    mpz_sub_ui(tmp1, p, 1);
    mpz_sub_ui(tmp2, q, 1);

    // Multiply tmp1 and tmp2. Save in tmp3
    mpz_mul(tmp3, tmp1, tmp2);

    // Calculate gcd of tmp1 and tmp2. Save in tmp4
    gcd(tmp4, tmp1, tmp2);

    // lambda = lcm(p-1,q-1) = product/gcd
    mpz_fdiv_q(lambda, tmp3, tmp4);

    mpz_urandomb(tmp1, state, nbits);
    gcd(tmp2, tmp1, lambda);

    // Loop till a random number of size around nbits is found that's coprime
    // with lambda. This number is the exponent.
    // while (! mpz_cmp_ui(tmp2, 1) || mpz_even_p(tmp1)) {
    while (mpz_cmp_ui(tmp2, 1)) {
        mpz_urandomb(tmp1, state, nbits);
        gcd(tmp2, tmp1, lambda);
    }
    mpz_set(e, tmp1);
    mpz_mul(n, p, q);

    mpz_clears(tmp1, tmp2, tmp3, tmp4, lambda, NULL);
    return;
}

// Writes a public RSA key to pbfile. n, e, and s are written as hexstrings
// in that order.
//
// Input parameters:
// n, e, s: mpz_t: Constituents of the public key
// username: char[]
// pbfile: FILE *: File pointer to the public key file
void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fprintf(pbfile, "%Zx\n", n);
    gmp_fprintf(pbfile, "%Zx\n", e);
    gmp_fprintf(pbfile, "%Zx\n", s);
    fprintf(pbfile, "%s\n", username);
}

// Reads the public key file pbfile to obtain the public key constituents.
//
// Input parameters:
// n, e, s: mpz_t: Constituents of the public key read as hex strings
// username: char[]
// pbfile: FILE *: File pointer to the public key file
void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx", n);
    gmp_fscanf(pbfile, "%Zx", e);
    gmp_fscanf(pbfile, "%Zx", s);
    fscanf(pbfile, "%s", username);
}

// Given primes p and q and public exponent e, create an RSA private key d.
//
// Input parameters:
// p, q: mpz_t: Primes
// e: mpz_t: Exponent
// d: mpz_t: Generated private key is saved here
// Returns: void
void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t tmp1, tmp2, tmp3, tmp4, lambda;
    mpz_inits(tmp1, tmp2, tmp3, tmp4, lambda, NULL);

    // Set tmp1 to p-1 and tmp2 to q-1
    mpz_sub_ui(tmp1, p, 1);
    mpz_sub_ui(tmp2, q, 1);

    // Multiply tmp1 and tmp2. Save in tmp3
    mpz_mul(tmp3, tmp1, tmp2);

    // Calculate gcd of tmp1 and tmp2. Save in tmp4
    gcd(tmp4, tmp1, tmp2);

    // lambda = lcm(p-1,q-1) = product/gcd
    mpz_fdiv_q(lambda, tmp3, tmp4);

    // Private key = inverse of e modulo lambda(n)
    mod_inverse(d, e, lambda);

    mpz_clears(tmp1, tmp2, tmp3, tmp4, lambda, NULL);
}

// Writes a private RSA key to pvfile. n and d are written as hexstrings
// in that order.
//
// Input parameters:
// n, d: mpz_t: Constituents of the private key
// pvfile: FILE *: File pointer to the private key file
// Returns: void
void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", n);
    gmp_fprintf(pvfile, "%Zx\n", d);
}

// Reads a private RSA key from pvfile.
//
// Input parameters:
// n, d: mpz_t: Components of the private key stored as hex and read in that order
// pvfile: FILE *: File pointer of the file to be read
// Returns: void
void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx", n);
    gmp_fscanf(pvfile, "%Zx", d);
}

// Performs RSA encryption, computing ciphertext c by encrypting message
// m using public exponent e and modulus n.
//
// Input parameters:
// c: mpz_t: Generated ciphertext
// m: mpz_t: Original plaintext message
// e: mpz_t: Public exponent
// n: mpz_t: Modulus
// Returns: void
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    pow_mod(c, m, e, n);
}

// Encrypts the contents of infile, writing the encrypted contents to outfile.
//
// Input parameters:
// infile: FILE *: Input file to be encrypted
// outfile: FILE *: Encrypted output file
// n: mpz_t: Modulus
// e: mpz_t: Exponent
// Returns: void
void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    size_t j;
    mpz_t m, c;
    mpz_inits(m, c, NULL);
    uint64_t k;
    uint8_t *buf;

    // Calculate the block size k = floor(log_2(n)-1/8)
    k = (mpz_sizeinbase(n, 2) - 1) / 8;

    // Allocate array to hold k bytes. Typecast it to uint8_t *.
    buf = (uint8_t *) calloc(k, 1);

    // Set the 0th byte of the block to 0xFF
    buf[0] = 0xFF;

    while (!feof(infile)) {
        // Read at most k-1 bytes in the buffer, starting from position 1
        j = fread(buf + 1, 1, k - 1, infile);

        // Import to an mpz_t variable, encrypt, and write to outfile as hex
        mpz_import(m, j + 1, 1, 1, 1, 0, buf);
        rsa_encrypt(c, m, e, n);
        gmp_fprintf(outfile, "%Zx\n", c);
    }

    mpz_clears(m, c, NULL);
    free(buf);
}

// Performs RSA decryption, computing message m by decrypting ciphertext c
//
// Input parameters:
// m: mpz_t: Decrypted message
// c: mpz_t: Ciphertext to be decrypted.
// d: mpz_t: Private key
// n: mpz_t: Public modulus
// Returns: void
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    pow_mod(m, c, d, n);
}

// Decrypts the contents of infile, writing the decrypted contents to outfile.
//
// Input parameters:
// infile: FILE *: Input file containing the ciphertext
// outfile: FILE *: Output file that will contain the plain text
// n: mpz_t: Modulus
// d: mpz_t: Private key
// Returns: void
void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    uint64_t k = 8;
    uint8_t *buf;
    uint64_t j;
    mpz_t c, m;
    mpz_inits(c, m, NULL);

    // Calculate the block size k = floor(log_2(n)-1/8)
    k = (mpz_sizeinbase(n, 2) - 1) / 8;
    // Allocate array to hold k bytes. Typecast it to uint8_t *.
    buf = (uint8_t *) calloc(k, 1);

    // Set the 0th byte of the block to 0xFF
    buf[0] = 0xFF;

    // Scan in a hexstring to a variable c (for ciphertex)
    gmp_fscanf(infile, "%Zx", c);

    while (!feof(infile)) {
        rsa_decrypt(m, c, d, n);
        mpz_export(buf, &j, 1, 1, 1, 0, m);
        fwrite(buf + 1, 1, j - 1, outfile);
        gmp_fscanf(infile, "%Zx", c);
    }

    mpz_clears(c, m, NULL);
    free(buf);
}

// Performs RSA signing
//
// Input parameters:
// s: mpz_t: Signature that's produced
// m: mpz_t: Message to be signed
// d: mpz_t: Private key
// n: mpz_t: Public modulus
// Returns: void
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    pow_mod(s, m, d, n);
}

// Performs RSA verification
//
// Input parameters:
// m: mpz_t: Expected message
// s: mpz_t: Signature to be verified
// e: mpz_t: Exponent
// n: mpz_t: Modulus
// Returns: bool: True if signature is verified. False otherwise
bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    mpz_t t;
    mpz_init(t);

    pow_mod(t, s, e, n);

    if (mpz_cmp(t, m) == 0) {
        mpz_clear(t);
        return true;
    } else {
        mpz_clear(t);
        return false;
    }
}
