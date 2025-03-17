#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Function to perform encryption
void encrypt_message(char *message, char *key, char *ciphertext) {
    int msg_len = strlen(message);
    for (int i = 0; i < msg_len; i++) {
        if (isalpha(message[i]) && isalpha(key[i])) {
            // Convert letters to uppercase if they are not already
            char m = toupper(message[i]);
            char k = toupper(key[i]);
            // Encrypt using Vigenère cipher (mod 26)
            ciphertext[i] = ((m - 'A') + (k - 'A')) % 26 + 'A';
        }
    }
    ciphertext[msg_len] = '\0'; // Null terminate the ciphertext
}

// Function to handle client communication
void handle_client(int client_sock) {
    char message[1024], key[1024], ciphertext[1024];
    
    // Receive plaintext from the client
    int msg_len = recv(client_sock, message, sizeof(message), 0);
    if (msg_len <= 0) {
        perror("Error receiving message");
        close(client_sock);
        return;
    }

    // Receive key from the client
    int key_len = recv(client_sock, key, sizeof(key), 0);
    if (key_len <= 0) {
        perror("Error receiving key");
        close(client_sock);
        return;
    }

    // Perform encryption
    encrypt_message(message, key, ciphertext);

    // Send back to the client
    send(client_sock, ciphertext, strlen(ciphertext), 0);

    close(client_sock);
}

// Main function to set up the server and handle incoming connections
int main(int argc, char *argv[]) {
    int server_sock, client_sock, port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Check if port number is provided as a command-line argument
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    // Create server socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Set up the server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }

    // Listen for incoming client connections (backlog of 5)
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("enc_server listening on port %d...\n", port);

    // Accept and handle multiple client connections
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Handle client communication
            close(server_sock); // Close the server socket in the child process
            handle_client(client_sock);
            exit(0); // Child process exits after handling client
        } else if (pid < 0) {
            perror("Fork failed");
            close(client_sock);
        } else {
            // Parent process: Close the client socket and continue listening
            close(client_sock);
        }
    }

    // Close the server socket (never reached in this case)
    close(server_sock);
    return 0;
}
