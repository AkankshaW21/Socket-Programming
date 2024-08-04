#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define HISTORY_FILE "chat_history_client.txt"

void to_hamming(const char *msg, int *hamming_msg) {
    int i = 0;
    while (msg[i]) {
        int ascii_char = (int)msg[i];
        for (int j = 7; j >= 0; j--) {
            hamming_msg[i * 8 + (7 - j)] = (ascii_char >> j) & 1;
        }
        i++;
    }
    hamming_msg[i * 8] = -1;
}

void from_hamming(int *hamming_msg, char *msg) {
    int i = 0;
    while (hamming_msg[i * 8] != -1) {
        int ascii_char = 0;
        for (int j = 0; j < 8; j++) {
            ascii_char = (ascii_char << 1) | hamming_msg[i * 8 + j];
        }
        msg[i] = (char)ascii_char;
        i++;
    }
    msg[i] = '\0';
}

// Function to log messages to a file
void logMessage(const char *message) {
    FILE *logFile = fopen(HISTORY_FILE, "a"); // 'a' for append
    if (logFile != NULL) {
        time_t currentTime;
        time(&currentTime);
        fprintf(logFile, "[%s] %s\n", asctime(localtime(&currentTime)), message);
        fclose(logFile);
    }
}

int main() {
    int sock;
    struct sockaddr_in server;
    char server_reply[12000]; // Adjusted the buffer size to match Hamming encoding
    int response_hamming[12000]; // Adjusted the buffer size
    char response[1000];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
        return 1;
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to the remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    while (1) {
        // Get message from user
        printf("Client (you): ");
        fgets(response, sizeof(response), stdin);

        // Remove the trailing newline character from the user's input
        response[strcspn(response, "\n")] = '\0';

        // Log the client's message
        logMessage(response);

        // Display the message and its ASCII value
        printf("Client (you): %s\n", response);
        printf("ASCII Value: ");
        int hamming_msg[12000]; // Adjusted the buffer size
        to_hamming(response, hamming_msg);

        for (int i = 0; hamming_msg[i] != -1; i++) {
            printf("%d ", hamming_msg[i]);
        }
        printf("\n");

        // Send the Hamming-encoded response to the server
        if (send(sock, hamming_msg, sizeof(hamming_msg), 0) < 0) {
            puts("Send failed");
            break;
        }

        // Receive message from the server
        if (recv(sock, response_hamming, sizeof(response_hamming), 0) < 0) {
            puts("recv failed");
            break;
        }

        // Convert the received Hamming encoded message to a string
        char server_response[1000];
        from_hamming(response_hamming, server_response);
        logMessage(server_response);

        // Display the server's response and its ASCII value
        printf("Server: %s\n", server_response);
        printf("ASCII Value: ");
        int server_hamming_msg[12000]; // Adjusted the buffer size
        to_hamming(server_response, server_hamming_msg);

        for (int i = 0; server_hamming_msg[i] != -1; i++) {
            printf("%d ", server_hamming_msg[i]);
        }
        printf("\n");
    }
    close(sock);
    return 0;
}

