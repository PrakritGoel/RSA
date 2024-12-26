# Assignment 5 - Public Key Cryptography

In this program, we need to implement the RSA public key encryption algorithm. RSA is an asymmetric key algorithm that uses two mathematically related keys to encrypt/decrypt messages. The public key, as the name suggests, is public, while the private key is known only to the user. This algorithm resolves the key exchange problem, since the communication key can be encrypted using the public key of the recipient. Anyone else will not be able to decrypt and use the key, since the private key is required to decrypt the message.

The algorithm is assumed to be secure, since it is based on a problem that is considered to be hard to solve, namely factorization of large numbers. The program generates large prime numbers (potentially hundreds or thousands of bits long), and generates public and private keys by performing mathematical operations on them.

To achieve this, we use the gnu libgmp library. The standard data types in C don’t help here, since the long data type is only 64 bits long. The mpz_t data type in this library allows us to specify positive integers of arbitrary size.

The following are the user command-line options for running keygen:

-b <num_bits>: Minimum number of bits needed for public modulus n
-i <num_iters>: Number of Miller-Rabin iterations for testing primes
-n <pub_key_file>: File containing the public key (default is rsa.pub)
-d <pub_key_file>: File containing the private key (default is rsa.priv)
-s <seed>: Seed for random state initialization
-v: Turn on verbose mode
-h: Print this message

The following are the user command-line options for running encrypt or decrypt:

-i <input_file>: Input file to decrypt (default is stdin)
-o <output_file>: Output file to decrypt (default is stdout)
-n <pub_key_file>: File containing the public key (default is rsa.pub)
-v: Turn on verbose mode
-h: Print this message


## Building

Run the following to build the `keygen`, `encrypt`, and `decrypt` programs:

```
$ make all
```


## Running

```
$ ./keygen [-b <num_bits>][-i <num_iters>][-n <pub_key_file>][-d <priv_key_file>][-s <seed>][-vh]
```

```
$ ./encrypt [-i <input_file>][-o <output_file>][-n <priv_key_file>][-vh]
```

```
$ ./decrypt [-i <input_file>][-o <output_file>][-n <priv_key_file>][-vh]
```


## Testing

I have also included a file numtheory_main.c that contains some tests for most of the fuctions implemented in numtheory.c. The Makefile target numtheory builds this executable which can be run to conduct some sanity testing for the numtheory functions.

The following commands can be used to build and test numtheory:
```
$ make numtheory
$ ./numtheory
```

I have also added targets in the Makefile to test the three executables and also to check for memory leaks. The 'tst' target uses the `keygen` executable to generate keys of bit length of approximately 1000. It then encrypts the file /usr/share/dict/words using the `encrypt` executable. The encrypted file is decrypted using the `decrypt` executable. The decrypted file is compared to the original file. If the two files are the same, then we know the program is working.

The 'tst_valgrind' target runs the valgrind command on the three executables. I detected no memory leaks when this target was last invoked.

```
$ make tst
$ make tst_valgrind
```

Finally, scan-build reported no false positives nor any other bugs.

