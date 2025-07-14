# One-Time Pad Encryption System (C Socket Programming)

This project implements a basic one-time pad encryption system using client-server architecture in C. It includes components for generating keys, encrypting messages, and decrypting ciphertext—leveraging secure communication over TCP sockets.

## Components

- `keygen.c` – Generates a one-time pad key file of specified length
- `enc_client.c` – Connects to the encryption server and sends plaintext/key
- `enc_server.c` – Receives data from the encryption client and returns encrypted text
- `dec_client.c` – Connects to the decryption server and sends ciphertext/key
- `dec_server.c` – Receives data from the decryption client and returns decrypted plaintext

## Build Instructions

To compile all programs:

```bash
gcc -o keygen keygen.c
gcc -o enc_client enc_client.c
gcc -o enc_server enc_server.c
gcc -o dec_client dec_client.c
gcc -o dec_server dec_server.c
```

## Usage

### 1. Generate a key
```bash
./keygen 1000 > mykey.txt
```

### 2. Start the encryption server
```bash
./enc_server <port>
```

### 3. Encrypt a message
```bash
./enc_client <plaintext_file> <key_file> <port>
```

### 4. Start the decryption server
```bash
./dec_server <port>
```

### 5. Decrypt a message
```bash
./dec_client <ciphertext_file> <key_file> <port>
```

> Ensure that servers are running before launching their corresponding clients.

## 💡 Notes

- All data is passed using standard input/output and TCP sockets.
- The system supports basic character validation (A–Z and space).
