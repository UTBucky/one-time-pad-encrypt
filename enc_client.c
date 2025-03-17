#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

void encrypt_and_send(char *plaintext, char *key, const char *server_ip, int server_port) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sock);
        exit(2);  // Exit with code 2 for connection failure
    }

    // Send plaintext to the server
    send(sock, plaintext, strlen(plaintext), 0);

    // Send key to the server
    send(sock, key, strlen(key), 0);

    // Receive the ciphertext from the server
    bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("recv failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    // Output ciphertext
    printf("Ciphertext: %s\n", buffer);

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <plaintext_file> <key_file> <port>\n", argv[0]);
        return 1;
    }

    FILE *plaintext_file = fopen(argv[1], "r");
    FILE *key_file = fopen(argv[2], "r");
    char plaintext[MAX_BUFFER_SIZE], key[MAX_BUFFER_SIZE];
    int port = atoi(argv[3]);

    if (!plaintext_file || !key_file) {
        fprintf(stderr, "Error opening files\n");
        return 1;
    }

    // Read plaintext from file
    fgets(plaintext, MAX_BUFFER_SIZE, plaintext_file);
    fclose(plaintext_file);

    // Read key from file
    fgets(key, MAX_BUFFER_SIZE, key_file);
    fclose(key_file);

    // Remove trailing newlines
    plaintext[strcspn(plaintext, "\n")] = 0;
    key[strcspn(key, "\n")] = 0;

    // Encrypt and send to server
    encrypt_and_send(plaintext, key, "127.0.0.1", port);

    return 0;
}
