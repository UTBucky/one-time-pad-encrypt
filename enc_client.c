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

// Function to send the plaintext and key to the server and get the ciphertext
void encrypt_file(const char *plaintext_file, const char *key_file, const char *server_ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *plaintext = fopen(plaintext_file, "r");
    FILE *key = fopen(key_file, "r");

    if (plaintext == NULL) {
        fprintf(stderr, "Error: Could not open plaintext file %s\n", plaintext_file);
        exit(1);
    }

    if (key == NULL) {
        fprintf(stderr, "Error: Could not open key file %s\n", key_file);
        fclose(plaintext);
        exit(1);
    }

    // Check if the key is longer than or equal to the plaintext length
    fseek(plaintext, 0, SEEK_END);
    long plaintext_len = ftell(plaintext);
    fseek(key, 0, SEEK_END);
    long key_len = ftell(key);

    if (key_len < plaintext_len) {
        fprintf(stderr, "Error: The key is shorter than the plaintext\n");
        fclose(plaintext);
        fclose(key);
        exit(1);
    }

    // Open a socket to connect to the server
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Could not create socket\n");
        fclose(plaintext);
        fclose(key);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Attempt to connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: Could not connect to server on port %d\n", port);
        close(sockfd);
        fclose(plaintext);
        fclose(key);
        exit(2);
    }

    // Send the plaintext to the server
    fseek(plaintext, 0, SEEK_SET);
    char message[MAX_BUFFER_SIZE];
    int i = 0;
    while (fgets(message, sizeof(message), plaintext) != NULL) {
        if (send(sockfd, message, strlen(message), 0) < 0) {
            fprintf(stderr, "Error: Could not send message to server\n");
            close(sockfd);
            fclose(plaintext);
            fclose(key);
            exit(1);
        }
        i++;
    }

    // Send the key to the server
    fseek(key, 0, SEEK_SET);
    char key_buffer[MAX_BUFFER_SIZE];
    i = 0;
    while (fgets(key_buffer, sizeof(key_buffer), key) != NULL) {
        if (send(sockfd, key_buffer, strlen(key_buffer), 0) < 0) {
            fprintf(stderr, "Error: Could not send key to server\n");
            close(sockfd);
            fclose(plaintext);
            fclose(key);
            exit(1);
        }
        i++;
    }

    // Receive the ciphertext from the server
    char ciphertext[MAX_BUFFER_SIZE];
    int recv_len;
    while ((recv_len = recv(sockfd, ciphertext, sizeof(ciphertext) - 1, 0)) > 0) {
        ciphertext[recv_len] = '\0'; // Null-terminate the ciphertext
        printf("%s", ciphertext); // Output the ciphertext to stdout
    }

    // Close the socket and files
    close(sockfd);
    fclose(plaintext);
    fclose(key);
}

// Main function to process command-line arguments and invoke encryption
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <plaintext_file> <key_file> <port>\n", argv[0]);
        exit(1);
    }

    const char *plaintext_file = argv[1];
    const char *key_file = argv[2];
    int port = atoi(argv[3]);

    // Validate the files
    if (!is_valid_file(plaintext_file)) {
        fprintf(stderr, "Error: Plaintext file contains invalid characters or cannot be opened\n");
        exit(1);
    }

    if (!is_valid_file(key_file)) {
        fprintf(stderr, "Error: Key file contains invalid characters or cannot be opened\n");
        exit(1);
    }

    // Connect to enc_server and perform encryption
    encrypt_file(plaintext_file, key_file, "127.0.0.1", port);

    return 0;
}
