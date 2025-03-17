#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALPHABET_SIZE 27 // 26 letters + space
#define CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ " // Allowed characters

// Function to generate a random key of a specified length
void generate_key(int keylength) {
    // Seed the random number generator
    srand(time(NULL));

    // Generate and output the key
    for (int i = 0; i < keylength; i++) {
        // Pick a random index from 0 to ALPHABET_SIZE - 1
        int random_index = rand() % ALPHABET_SIZE;

        // Output the corresponding character
        putchar(CHARACTERS[random_index]);
    }

    // Output a newline at the end of the key
    putchar('\n');
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <keylength>\n", argv[0]);
        return 1;
    }

    // Get the key length from the command-line argument
    int keylength = atoi(argv[1]);

    if (keylength <= 0) {
        fprintf(stderr, "Key length must be a positive integer.\n");
        return 1;
    }

    // Generate and print the key
    generate_key(keylength);

    return 0;
}
