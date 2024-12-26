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
    printf("-n <pub_key_file>: File containing the public key. Default is rsa.pub\n");
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
    char *pub_key_file = "rsa.pub";
    FILE *ifp, *ofp, *pkfp;
    bool verbose = false;
    mpz_t m, n, e, s;
    char user_name[100];

    // Parse the input options.
    while ((opt = getopt(argc, argv, "vn:i:o:h")) != -1) {
        switch (opt) {
        case ('i'): infile = optarg; break;
        case ('o'): outfile = optarg; break;
        case ('n'): pub_key_file = optarg; break;
        case ('v'): verbose = true; break;
        case ('h'): usage(argv[0]); return 0;
        default: usage(argv[0]); exit(EXIT_FAILURE);
        }
    }

    if ((pkfp = fopen(pub_key_file, "r")) == NULL) {
        printf("The public key file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    }

    mpz_inits(m, n, e, s, NULL);
    // mpz_init(m);
    rsa_read_pub(n, e, s, user_name, pkfp);
    fclose(pkfp);

    if (verbose == true) {
        printf("user = %s\n", user_name);
        gmp_printf("s (%ld bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("n (%ld bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%ld bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
    }

    // Convert the username to an mpz_t variable
    // I'm using the same logic as was used for signing. This doesn't
    // work though. I generated the key through the given keygen program,
    // so it's not a keygen issue. Given that the generated m is much
    // larger than n (I used -b 6 for testing, and got n = 145), I
    // think the below logic is incorrect. The conversion itself is fine
    // though, as can be seen by the printf output.
    mpz_set_str(m, user_name, 62);

    // Verify the signature using rsa_verify()
    if (!rsa_verify(m, s, e, n)) {
        mpz_clears(m, n, e, s, NULL);
        printf("Signature could not be verified. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    if (infile == NULL) {
        ifp = stdin;
    } else if ((ifp = fopen(infile, "r")) == NULL) {
        mpz_clears(m, n, e, s, NULL);
        printf("The input file is invalid. Please provide a valid input file\n");
        exit(EXIT_FAILURE);
    }

    if (outfile == NULL) {
        ofp = stdout;
    } else if ((ofp = fopen(outfile, "w")) == NULL) {
        mpz_clears(m, n, e, s, NULL);
        printf("The output file is invalid. Please provide a valid output file\n");
        exit(EXIT_FAILURE);
    }

    rsa_encrypt_file(ifp, ofp, n, e);

    // Clear any mpz_t variables
    mpz_clears(m, n, e, s, NULL);

    if (infile != NULL) {
        fclose(ifp);
    }
    if (outfile != NULL) {
        fclose(ofp);
    }

    return 0;
}
