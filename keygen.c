#include "numtheory.h"
#include "rsa.h"
#include "randstate.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

gmp_randstate_t state;

// Usage Function
// Input parameters:
// exec_name: char *: Name of the program
// Returns: void
void usage(char *exec_name) {
    printf("USAGE: %s [-b <num_bits>][-i <num_iters>][-n <pub_key_file>][-d <priv_key_file>][-s "
           "<seed>][-vh]\n",
        exec_name);
    printf("-b <num_bits>: Minimum number of bits needed for public modulus n\n");
    printf("-i <num_iters>: Number of Miller-Rabin iterations for testing primes\n");
    printf("-n <pub_key_file>: File containing the public key. Default is rsa.pub\n");
    printf("-d <pub_key_file>: File containing the private key. Default is rsa.priv\n");
    printf("-s <seed>: Seed for random state initialization\n");
    printf("-v: Turn on verbose mode\n");
    printf("-h: Print this message\n");
    return;
}

// The main function
//
// Input parameters:
// argc: int: Number of input arguments
// argv: char **: The input arguments
// Returns: int: 0 in case of success, non-zero for failure
int main(int argc, char **argv) {
    int opt;
    uint64_t nbits = 256;
    uint32_t mr_iters = 50;
    char *pbfile = "rsa.pub";
    char *pvfile = "rsa.priv";
    FILE *pbfp, *pvfp;
    time_t seed = time(NULL);
    bool verbose = false;
    char *user_name;
    mpz_t d, e, m, n, p, q, s, u;

    // Parse the input options.
    while ((opt = getopt(argc, argv, "b:vi:n:d:s:h")) != -1) {
        switch (opt) {
        case ('b'): nbits = strtoul(optarg, NULL, 10); break;
        case ('i'): mr_iters = strtoul(optarg, NULL, 10); break;
        case ('n'): pbfile = optarg; break;
        case ('d'): pvfile = optarg; break;
        case ('s'): seed = strtoul(optarg, NULL, 10); break;
        case ('v'): verbose = true; break;
        case ('h'): usage(argv[0]); return 0;
        default: usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    // Open the public key file for writing
    if ((pbfp = fopen(pbfile, "w")) == NULL) {
        printf("The public key file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    }

    // Open the private key file for writing
    if ((pvfp = fopen(pvfile, "w")) == NULL) {
        printf("The private key file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    } else {
        // Ensure that private key file permissions are set to 0600.
        int fd = fileno(pvfp);
        fchmod(fd, S_IRUSR | S_IWUSR);
    }

    randstate_init(seed);
    mpz_inits(d, e, m, n, p, q, s, u, NULL);
    rsa_make_pub(p, q, n, e, nbits, mr_iters);
    rsa_make_priv(d, e, p, q);

    user_name = getenv("USER");
    mpz_set_str(u, user_name, 62);

    // Compute the signature of the user name
    rsa_sign(s, u, d, n);

    // Write the public and private keys
    rsa_write_pub(n, e, s, user_name, pbfp);
    rsa_write_priv(n, d, pvfp);

    if (verbose == true) {
        printf("user = %s\n", user_name);
        gmp_printf("s (%ld bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("p (%ld bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q (%ld bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n (%ld bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%ld bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
        gmp_printf("d (%ld bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    fclose(pbfp);
    fclose(pvfp);

    // Clear all mpz_t variables
    randstate_clear();
    mpz_clears(d, e, m, n, p, q, s, u, NULL);

    return 0;
}
