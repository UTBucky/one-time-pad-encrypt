#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALPHABET_SIZE 26
#define CHARSET_SIZE (ALPHABET_SIZE + 1)  // 26 letters + 1 for newline

// Function to generate the key of a specified length
void generate_key(int keylength) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";  // 27 characters (26 letters + newline)
    
    for (int i = 0; i < keylength; ++i) {
        int random_index = rand() % CHARSET_SIZE;  // Randomly pick an index in the charset
        putchar(charset[random_index]);  // Output the character to stdout
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <keylength>\n", argv[0]);
        return 1;
    }

    int keylength = atoi(argv[1]);  // Convert the argument to an integer

    if (keylength <= 0) {
        fprintf(stderr, "Error: Invalid key length. The key length must be a positive integer.\n");
        return 1;
    }

    // Seed the random number generator using the current time
    srand(time(NULL));

    // Generate the key of the specified length
    generate_key(keylength);

    return 0;
}
