#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>

#define PORT 8090
#define WINDOW_SIZE 5
#define TIMEOUT 2  // Timeout in seconds

struct packet {
    int no, ack, seq;
    char data[1024];  // Packet data (optional)
};

int main() {
    int sock, n, wnd;
    struct sockaddr_in sadd;
    socklen_t slen = sizeof(sadd);

    printf("Enter the number of packets: ");
    scanf("%d", &n);
    printf("Enter the window size: ");
    scanf("%d", &wnd);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, &wnd, sizeof(wnd), 0, (struct sockaddr *)&sadd, sizeof(sadd));  // Send window size

    struct packet pk[n];
    int acked[n];  // Array to track acknowledged packets
    memset(acked, 0, sizeof(acked));  // Initialize acknowledgment array to 0

    for (int i = 0; i < n; i++) {
        pk[i].no = i;
        pk[i].seq = i;  // Assign sequence number
        snprintf(pk[i].data, sizeof(pk[i].data), "Packet %d data", i);
    }

    int base = 0, next_seq_num = 0;
    time_t timers[WINDOW_SIZE];  // Timer for each packet

    while (base < n) {
        // Send packets within the window
        while (next_seq_num < base + wnd && next_seq_num < n) {
            if (!acked[next_seq_num]) {  // Send only unacknowledged packets
                printf("Sending packet %d with sequence number %d\n", next_seq_num, pk[next_seq_num].seq);
                sendto(sock, &pk[next_seq_num], sizeof(pk[next_seq_num]), 0, (struct sockaddr *)&sadd, sizeof(sadd));
                timers[next_seq_num % WINDOW_SIZE] = time(NULL);  // Start the timer for the packet
            }
            next_seq_num++;
        }

        // Wait for acknowledgment
        struct timeval tv;
        tv.tv_sec = 1;  // Timeout of 1 second
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int ack;
        if (recvfrom(sock, &ack, sizeof(ack), 0, (struct sockaddr *)&sadd, &slen) > 0) {
            printf("Received ACK for packet %d\n", ack);
            acked[ack] = 1;  // Mark packet as acknowledged

            // Slide the window forward
            while (acked[base] && base < n) {
                base++;  // Move the base only when packets are acknowledged
            }
        }

        // Retransmit packets if they timeout
        for (int i = base; i < next_seq_num; i++) {
            if (!acked[i] && (time(NULL) - timers[i % WINDOW_SIZE]) > TIMEOUT) {
                printf("Retransmitting packet %d\n", pk[i].seq);
                sendto(sock, &pk[i], sizeof(pk[i]), 0, (struct sockaddr *)&sadd, sizeof(sadd));
                timers[i % WINDOW_SIZE] = time(NULL);  // Restart the timer
            }
        }
    }

    printf("All packets sent and acknowledged.\n");
    close(sock);
    return 0;
}
