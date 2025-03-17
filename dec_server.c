#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 5

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

// Function to decrypt ciphertext using a key (Vigenère Cipher-like decryption)
void decrypt_data(int client_sock) {
    char plaintext[MAX_BUFFER_SIZE], key[MAX_BUFFER_SIZE];
    int recv_len;

    // Receive ciphertext from the client
    recv_len = recv(client_sock, plaintext, sizeof(plaintext) - 1, 0);
    if (recv_len < 0) {
        fprintf(stderr, "Error: Failed to receive ciphertext\n");
        close(client_sock);
        return;
    }
    plaintext[recv_len] = '\0'; // Null-terminate the ciphertext

    // Receive key from the client
    recv_len = recv(client_sock, key, sizeof(key) - 1, 0);
    if (recv_len < 0) {
        fprintf(stderr, "Error: Failed to receive key\n");
        close(client_sock);
        return;
    }
    key[recv_len] = '\0'; // Null-terminate the key

    // Decrypt the ciphertext using the key
    int i;
    char decrypted[MAX_BUFFER_SIZE];
    for (i = 0; i < strlen(plaintext); i++) {
        // Decrypt each character
        int cipher_char = plaintext[i] - 'A'; // Convert ciphertext to index (0-25)
        int key_char = key[i % strlen(key)] - 'A'; // Convert key to index (0-25)
        int decrypted_char = (cipher_char - key_char + 26) % 26; // Subtract key char and mod 26
        decrypted[i] = decrypted_char + 'A'; // Convert back to a character
    }
    decrypted[i] = '\0'; // Null-terminate the decrypted message

    // Send the decrypted plaintext back to the client
    if (send(client_sock, decrypted, strlen(decrypted), 0) < 0) {
        fprintf(stderr, "Error: Failed to send decrypted data\n");
    }

    close(client_sock);
}

// Function to handle client connection
void handle_client(int client_sock) {
    // Decrypt the data
    decrypt_data(client_sock);
}

// Main server function to handle incoming connections
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a TCP socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error: Failed to create socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the address and port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error: Failed to bind socket");
        close(server_sock);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_sock, MAX_CONNECTIONS) < 0) {
        perror("Error: Failed to listen on socket");
        close(server_sock);
        exit(1);
    }

    printf("dec_server is listening on port %s...\n", argv[1]);

    while (1) {
        // Accept incoming client connection
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("Error: Failed to accept client connection");
            continue;
        }

        // Handle the client connection in a child process
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error: Failed to fork process");
            close(client_sock);
            continue;
        }

        if (pid == 0) { // Child process
            close(server_sock); // Close server socket in the child process
            handle_client(client_sock); // Decrypt data
            exit(0);
        } else { // Parent process
            close(client_sock); // Close client socket in the parent process
        }
    }

    close(server_sock);
    return 0;
}
