#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 1024
#define ALPHABET_SIZE 27 // 26 letters + space

// Map characters to numbers
int char_to_num(char c) {
    if (c == ' ') return 26; // Space maps to 26
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return -1; // Invalid character
}

// Map numbers back to characters
char num_to_char(int num) {
    if (num == 26) return ' '; // Space is 26
    if (num >= 0 && num <= 25) return 'A' + num;
    return '\0'; // Invalid number
}

// Function to perform decryption using modulo 27 arithmetic
void decrypt_message(char *ciphertext, char *key, char *plaintext) {
    int i = 0;
    while (ciphertext[i] != '\0' && key[i] != '\0') {
        int cipher_num = char_to_num(ciphertext[i]);
        int key_num = char_to_num(key[i]);

        if (cipher_num == -1 || key_num == -1) {
            fprintf(stderr, "Invalid character in ciphertext or key.\n");
            return;
        }

        // Decrypting using modulo 27
        int plain_num = (cipher_num - key_num + ALPHABET_SIZE) % ALPHABET_SIZE;
        plaintext[i] = num_to_char(plain_num);

        i++;
    }
    plaintext[i] = '\0'; // Null terminate the decrypted message
}

// Function to handle client communication and decryption
void handle_client(int client_sock) {
    char ciphertext[MAX_BUFFER_SIZE], key[MAX_BUFFER_SIZE], plaintext[MAX_BUFFER_SIZE];
    int bytes_received;

    // Receive ciphertext from the client
    bytes_received = recv(client_sock, ciphertext, MAX_BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("recv failed (ciphertext)");
        close(client_sock);
        return;
    }
    ciphertext[bytes_received] = '\0'; // Null terminate the received ciphertext

    // Receive key from the client
    bytes_received = recv(client_sock, key, MAX_BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("recv failed (key)");
        close(client_sock);
        return;
    }
    key[bytes_received] = '\0'; // Null terminate the received key

    // Perform decryption
    decrypt_message(ciphertext, key, plaintext);

    // Send decrypted plaintext back to the client
    send(client_sock, plaintext, strlen(plaintext), 0);

    // Close the client socket
    close(client_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int server_sock, client_sock, port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    port = atoi(argv[1]);

    // Create server socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_sock);
        return 1;
    }

    // Listen for incoming connections (max 5 connections in the queue)
    if (listen(server_sock, 5) == -1) {
        perror("Listen failed");
        close(server_sock);
        return 1;
    }

    printf("dec_server listening on port %d...\n", port);

    // Accept client connections and handle them
    while (1) {
        // Accept incoming connection
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected, handling decryption...\n");

        // Handle client communication in a separate function
        handle_client(client_sock);
    }

    // Close the server socket (never reached in this infinite loop)
    close(server_sock);

    return 0;
}
