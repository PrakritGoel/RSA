#include "numtheory.h"
#include "rsa.h"

#include <stdlib.h>
#include <unistd.h>

gmp_randstate_t state;

// Usage Function
// Input parameters:
// exec_name: char *: Name of the program
// Returns: void
void usage(char *exec_name) {
    printf("USAGE: %s [-i <input_file>][-o <output_file>][-n <priv_key_file>][-vh]\n", exec_name);
    printf("-i <input_file>: Input file to decrypt. Default is stdin\n");
    printf("-o <output_file>: Output file to decrypt. Default is stdout\n");
    printf("-n <priv_key_file>: File containing the private key. Default is rsa.priv\n");
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
    char *infile = NULL;
    char *outfile = NULL;
    char *priv_key_file = "rsa.priv";
    FILE *ifp, *ofp, *pkfp;
    bool verbose = false;
    mpz_t n, d;

    // Parse the input options.
    while ((opt = getopt(argc, argv, "vn:i:o:h")) != -1) {
        switch (opt) {
        case ('n'): priv_key_file = optarg; break;
        case ('i'): infile = optarg; break;
        case ('o'): outfile = optarg; break;
        case ('v'): verbose = true; break;
        case ('h'): usage(argv[0]); return 0;
        default: usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    if ((pkfp = fopen(priv_key_file, "r")) == NULL) {
        printf("The private key file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    }
    mpz_inits(n, d, NULL);
    rsa_read_priv(n, d, pkfp);
    fclose(pkfp);

    if (verbose == true) {
        gmp_printf("n (%ld bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("d (%ld bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    if (infile == NULL) {
        ifp = stdin;
    } else if ((ifp = fopen(infile, "r")) == NULL) {
        mpz_clears(n, d, NULL);
        printf("The input file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    }

    if (outfile == NULL) {
        ofp = stdout;
    } else if ((ofp = fopen(outfile, "w")) == NULL) {
        mpz_clears(n, d, NULL);
        printf("The output file is invalid. Please provide a valid output file\n");
        exit(EXIT_FAILURE);
    }
    rsa_decrypt_file(ifp, ofp, n, d);

    if (infile != NULL) {
        fclose(ifp);
    }
    if (outfile != NULL) {
        fclose(ofp);
    }
    mpz_clears(n, d, NULL);

    return 0;
}
