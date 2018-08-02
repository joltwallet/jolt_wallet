#include <stddef.h>

extern void printf(char *, ...);
extern void sqrt(double);
extern int crypto_generichash(unsigned char *, size_t, const unsigned char *, unsigned long long, const unsigned char *, size_t);
extern char *sodium_bin2hex(char * const, const size_t,
                             const unsigned char * const, const size_t);

volatile char *private_key = NULL; // Pointer to Key in Vault

int app_main(int argc, char **argv) {
    /* Entry point for primary Jolt GUI */

    printf("Hello World!\n");

    unsigned char hash[32];
    unsigned char key[32] = { 0 };
    crypto_generichash(hash, sizeof(hash), "meow", 4, 
            key, sizeof(key));
    char hash_hex[65];
    sodium_bin2hex(hash_hex, sizeof(hash_hex), hash, sizeof(hash) );
    printf("Hash result: %s\n", hash_hex);
    return 0;
}

int console(int argc, char **argv) {
    /* Entry point for console commands */
    printf("First Passed in argument was %s.\n", argv[0]);
    return 0;
}

int sign_msg(char *sig, size_t sig_len, char *msg, size_t msg_len, int argc, ...) {
    /* Populates sig buffer of length sig_len for msg of length msg_len.
     * Uses the private key derived by the BIP path specified by argv */
    return 0;
}

char *
