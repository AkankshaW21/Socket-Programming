// Receiver Side (Server)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h> // Add time.h for logging

#define HISTORY_FILE "chat_history_server.txt"

// Function to convert a string to ASCII values
void to_ascii(const char *msg, int *ascii_msg) {
    int i = 0;
    while (msg[i]) {
        ascii_msg[i] = (int) msg[i];
        i++;
    }
    ascii_msg[i] = -1;
}

// Function to apply Hamming code to ASCII values
void hamming_code(int *ascii_msg, int *hamming_msg) {
    int i = 0, j = 0, k = 0;
    while (ascii_msg[i] != -1) {
        int bin[8];
        for (j = 0; j < 8; j++) {
            bin[j] = ascii_msg[i] % 2;
            ascii_msg[i] /= 2;
        }
        for (j = 7; j >= 0; j--) {
            hamming_msg[k++] = bin[j];
        }
        i++;
    }
    hamming_msg[k] = -1;
}


void to_char(int *hamming_msg, char *msg) {
    int i = 0, j = 0;
    while (hamming_msg[i] != -1) {
        int num = 0;
        for (j = 0; j < 8; j++) {
            num = num * 2 + hamming_msg[i + j];
        }
        msg[i / 8] = (char) num;
        i += 8;
    }
    msg[i / 8] = '\0';
}

// Function to log messages to a file
void logMessage(const char* message) {
    FILE* logFile = fopen(HISTORY_FILE, "a"); // 'a' for append
    if (logFile != NULL) {
        time_t currentTime;
        time(&currentTime);
        fprintf(logFile, "[%s] %s\n", asctime(localtime(&currentTime)), message);
        fclose(logFile);
    }
}

// Function to retrieve chat history
void retrieveHistory() {
    FILE* logFile = fopen(HISTORY_FILE, "r");
    if (logFile != NULL) {
        char line[1000];
        while (fgets(line, sizeof(line), logFile) != NULL) {
            printf("%s", line);
        }
        fclose(logFile);
    }
}

int main() {
    int sock;
    struct sockaddr_in server, client;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Bind
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Listen
    listen(sock, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    int sin_size = sizeof(struct sockaddr_in);
    int new_socket = accept(sock, (struct sockaddr *)&client, &sin_size);
    if (new_socket < 0) {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    int hamming_msg[12000];
    int ascii_msg[1000];
    char msg[1000];
    char response[1000];
    int response_hamming[12000]; // Add a new array for Hamming code

    while (1) { // Continue the conversation until a specific condition is met
        // Receive message from client
        if (recv(new_socket, hamming_msg, sizeof(hamming_msg), 0) < 0) {
            puts("recv failed");
            break; // End the conversation on recv failure
        }

        // Convert message to ASCII
        to_char(hamming_msg, msg);

        // Log received message
        logMessage(msg);

        // Display received message and its ASCII value
        printf("Client: %s\n", msg);
        printf("ASCII Value: ");
        for (int i = 0; i < strlen(msg); i++) {
            printf("%d ", (int) msg[i]);
        }
        printf("\n");

        // Send a response
        printf("Server (you): ");
        fgets(response, sizeof(response), stdin);

        // Remove the trailing newline character from the user's input
        response[strcspn(response, "\n")] = 0;

        // Log the response
        logMessage(response);

        // Display the response and its ASCII value
        printf("Server (you): %s\n", response);
        printf("ASCII Value: ");
        for (int i = 0; i < strlen(response); i++) {
            printf("%d ", (int) response[i]);
        }
        printf("\n");

        // Apply Hamming code to the response
        to_ascii(response, ascii_msg);
        hamming_code(ascii_msg, response_hamming);

        // Send the response to the client
        if (send(new_socket, response_hamming, sizeof(response_hamming), 0) < 0) {
            puts("Send failed");
            break; // End the conversation on send failure
        }
    }

    close(new_socket);
    close(sock);
    return 0;
}
