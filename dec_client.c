#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 1024

// Function to check if a file contains only valid characters (letters)
int is_valid_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 0; // Could not open the file
    }

    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (!isalpha(ch)) {
            fclose(file);
            return 0; // Invalid character found
        }
    }

    fclose(file);
    return 1; // File contains only valid characters
}

// Function to connect to dec_server and handle decryption
int connect_to_server(const char *hostname, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Failed to create socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(hostname);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error: Connection failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Function to send ciphertext and key to dec_server and receive the decrypted message
void decrypt_message(int sockfd, const char *ciphertext, const char *key) {
    // Send the ciphertext
    if (send(sockfd, ciphertext, strlen(ciphertext), 0) < 0) {
        perror("Error: Failed to send ciphertext");
        close(sockfd);
        exit(1);
    }

    // Send the key
    if (send(sockfd, key, strlen(key), 0) < 0) {
        perror("Error: Failed to send key");
        close(sockfd);
        exit(1);
    }

    // Receive the decrypted plaintext from the server
    char decrypted[MAX_BUFFER_SIZE];
    int recv_len = recv(sockfd, decrypted, sizeof(decrypted) - 1, 0);
    if (recv_len < 0) {
        perror("Error: Failed to receive decrypted data");
        close(sockfd);
        exit(1);
    }
    decrypted[recv_len] = '\0'; // Null-terminate the decrypted message

    // Output the decrypted message to stdout
    printf("Decrypted message: %s\n", decrypted);

    close(sockfd);
}

// Main function for dec_client
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <plaintext file> <key file> <port>\n", argv[0]);
        exit(1);
    }

    const char *plaintext_file = argv[1];
    const char *key_file = argv[2];
    int port = atoi(argv[3]);

    // Check if the files contain valid characters
    if (!is_valid_file(plaintext_file)) {
        fprintf(stderr, "Error: Invalid plaintext file (only alphabetic characters allowed)\n");
        exit(1);
    }

    if (!is_valid_file(key_file)) {
        fprintf(stderr, "Error: Invalid key file (only alphabetic characters allowed)\n");
        exit(1);
    }

    // Open the plaintext file and key file
    FILE *file = fopen(plaintext_file, "r");
    if (!file) {
        perror("Error: Failed to open plaintext file");
        exit(1);
    }

    char ciphertext[MAX_BUFFER_SIZE];
    size_t len = fread(ciphertext, 1, sizeof(ciphertext) - 1, file);
    if (len == 0) {
        fprintf(stderr, "Error: Plaintext file is empty\n");
        fclose(file);
        exit(1);
    }
    ciphertext[len] = '\0'; // Null-terminate the ciphertext
    fclose(file);

    file = fopen(key_file, "r");
    if (!file) {
        perror("Error: Failed to open key file");
        exit(1);
    }

    char key[MAX_BUFFER_SIZE];
    len = fread(key, 1, sizeof(key) - 1, file);
    if (len == 0) {
        fprintf(stderr, "Error: Key file is empty\n");
        fclose(file);
        exit(1);
    }
    key[len] = '\0'; // Null-terminate the key
    fclose(file);

    // Check that the key is long enough
    if (strlen(key) < strlen(ciphertext)) {
        fprintf(stderr, "Error: Key is shorter than the ciphertext\n");
        exit(1);
    }

    // Connect to dec_server
    int sockfd = connect_to_server("127.0.0.1", port);
    if (sockfd < 0) {
        fprintf(stderr, "Error: Failed to connect to server on port %d\n", port);
        exit(2); // Set exit code to 2 for connection failure
    }

    // Send the ciphertext and key to the server and receive the decrypted message
    decrypt_message(sockfd, ciphertext, key);

    return 0;
}
