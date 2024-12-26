CC=clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic
GMP=`pkg-config --libs gmp`

all: keygen encrypt decrypt

encrypt: encrypt.o numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o encrypt encrypt.o numtheory.o rsa.o randstate.o ${GMP}

decrypt: decrypt.o numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o decrypt decrypt.o numtheory.o rsa.o randstate.o ${GMP}

keygen: keygen.o numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o keygen keygen.o numtheory.o rsa.o randstate.o ${GMP} 

numtheory: numtheory.o randstate.o numtheory_main.o
	$(CC) $(CFLAGS) -o numtheory numtheory.o randstate.o numtheory_main.o ${GMP}

decrypt.o: decrypt.c
	$(CC) $(CFLAGS) -c decrypt.c

encrypt.o: encrypt.c
	$(CC) $(CFLAGS) -c encrypt.c

keygen.o: keygen.c
	$(CC) $(CFLAGS) -c keygen.c

numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c numtheory.c

numtheory_main.o: numtheory_main.c
	$(CC) $(CFLAGS) -c numtheory_main.c

randstate.o: randstate.c
	$(CC) $(CFLAGS) -c randstate.c

rsa.o: rsa.c
	$(CC) $(CFLAGS) -c rsa.c

clean:
	rm -f *.o decrypt encrypt keygen numtheory

format:
	clang-format -i -style=file *.[c,h]

tst: tst_keygen tst_encrypt tst_decrypt

tst_keygen:
	./keygen -b 1000 -v

tst_encrypt:
	cp /usr/share/dict/words words
	./encrypt -i words -o words.enc

tst_decrypt:
	./decrypt -i words.enc -o words.dec
	diff words words.dec
	rm words words.enc words.dec

tst_valgrind: tst_valgrind_keygen tst_valgrind_encrypt tst_valgrind_decrypt

tst_valgrind_keygen:
	echo "-----------------------------------------------"
	echo "Testing valgrind keygen"
	valgrind ./keygen -v

tst_valgrind_encrypt:
	echo "-----------------------------------------------"
	echo "Testing valgrind encrypt"
	echo "This is a test for valgrind." > msg_file
	echo "Using a small file, since otherwise, it takes a very long time" >> msg_file
	valgrind ./encrypt -i msg_file -o msg.enc

tst_valgrind_decrypt:
	echo "-----------------------------------------------"
	echo "Testing valgrind decrypt"
	valgrind ./decrypt -i msg.enc -o msg.dec
	rm -f msg_file msg.enc msg.dec

