#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ALPHABET_SIZE 26
#define CHARSET_SIZE 27  // 26 letters + 1 space character
#define PORT 57171
#define MAX_CLIENTS 5

// Encrypt the plaintext using modulo 27
void encrypt(char *plaintext, char *key, char *ciphertext) {
    for (int i = 0; plaintext[i] != '\0'; i++) {
        int pt_char = (plaintext[i] == ' ') ? 26 : plaintext[i] - 'A';  // ' ' -> 26, 'A' -> 0
        int key_char = (key[i] == ' ') ? 26 : key[i] - 'A';  // ' ' -> 26, 'A' -> 0
        int enc_char = (pt_char + key_char) % CHARSET_SIZE;
        ciphertext[i] = (enc_char == 26) ? ' ' : enc_char + 'A';  // Modulo 27 wrap around
    }
}

void handle_client(int client_socket) {
    char plaintext[1024], key[1024], ciphertext[1024];
    int bytes_received;

    // Receive plaintext
    if ((bytes_received = recv(client_socket, plaintext, sizeof(plaintext) - 1, 0)) < 0) {
        perror("recv failed");
        close(client_socket);
        return;
    }
    plaintext[bytes_received] = '\0';  // Null-terminate string

    // Receive key
    if ((bytes_received = recv(client_socket, key, sizeof(key) - 1, 0)) < 0) {
        perror("recv failed");
        close(client_socket);
        return;
    }
    key[bytes_received] = '\0';  // Null-terminate string

    // Encrypt the message
    encrypt(plaintext, key, ciphertext);

    // Send the ciphertext back to the client
    send(client_socket, ciphertext, strlen(ciphertext), 0);

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Encryption server started on port %d\n", PORT);

    // Accept connections and handle them
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) != -1) {
        printf("Client connected\n");

        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(0);  // Close child process
        } else {
            close(client_socket);  // Parent process closes the client socket
        }
    }

    close(server_socket);
    return 0;
}
