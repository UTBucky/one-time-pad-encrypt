#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

// Function to connect to dec_server and send ciphertext and key for decryption
void decrypt_and_receive(char *ciphertext, char *key, const char *server_ip, int server_port) {
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

    // Send ciphertext to the server
    send(sock, ciphertext, strlen(ciphertext), 0);

    // Send key to the server
    send(sock, key, strlen(key), 0);

    // Receive the decrypted plaintext from the server
    bytes_received = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("recv failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    // Output the decrypted plaintext
    printf("Decrypted plaintext: %s\n", buffer);

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ciphertext_file> <key_file> <port>\n", argv[0]);
        return 1;
    }

    FILE *ciphertext_file = fopen(argv[1], "r");
    FILE *key_file = fopen(argv[2], "r");
    char ciphertext[MAX_BUFFER_SIZE], key[MAX_BUFFER_SIZE];
    int port = atoi(argv[3]);

    if (!ciphertext_file || !key_file) {
        fprintf(stderr, "Error opening files\n");
        return 1;
    }

    // Read ciphertext from file
    fgets(ciphertext, MAX_BUFFER_SIZE, ciphertext_file);
    fclose(ciphertext_file);

    // Read key from file
    fgets(key, MAX_BUFFER_SIZE, key_file);
    fclose(key_file);

    // Remove trailing newlines
    ciphertext[strcspn(ciphertext, "\n")] = 0;
    key[strcspn(key, "\n")] = 0;

    // Decrypt and receive the plaintext from the server
    decrypt_and_receive(ciphertext, key, "127.0.0.1", port);

    return 0;
}
