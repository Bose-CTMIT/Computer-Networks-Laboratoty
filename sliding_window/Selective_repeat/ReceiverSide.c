#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 8090
#define WINDOW_SIZE 5
#define MAX_SEQ 100

struct packet {
    int no, ack, seq;
    char data[1024];  // Packet data (optional)
};

int main() {
    int ss;
    struct sockaddr_in sadd, cladd;
    socklen_t clen = sizeof(cladd);
    struct packet pk;
    int received[MAX_SEQ] = {0};  // Track received packets
    int wnd;

    ss = socket(AF_INET, SOCK_DGRAM, 0);
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = INADDR_ANY;

    if (bind(ss, (struct sockaddr *)&sadd, sizeof(sadd)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    recvfrom(ss, &wnd, sizeof(wnd), 0, (struct sockaddr *)&cladd, &clen);  // Receive window size
    printf("Received window size: %d\n", wnd);

    int base = 0;  // Base of the receiver window
    printf("Server ready, waiting for packets...\n");

    while (1) {
        recvfrom(ss, &pk, sizeof(pk), 0, (struct sockaddr *)&cladd, &clen);

        if (pk.seq >= base && pk.seq < base + wnd) {
            // If packet is within the receiver window
            if (!received[pk.seq]) {
                printf("Received packet %d with sequence number %d\n", pk.no, pk.seq);
                received[pk.seq] = 1;  // Mark packet as received

                pk.ack = pk.seq;  // Send acknowledgment for this packet
                sendto(ss, &pk.ack, sizeof(pk.ack), 0, (struct sockaddr *)&cladd, clen);
            }

            // Slide the window if packets are received in order
            while (received[base]) {
                printf("Sliding window. Moving base to %d\n", base + 1);
                received[base] = 0;  // Reset the received status for the base packet
                base = (base + 1) % MAX_SEQ;  // Slide the window
            }
        } else {
            // Out of window packet, resend last acknowledgment
            printf("Packet %d out of order, expected window [%d, %d)\n", pk.seq, base, base + wnd);
            int last_ack = base - 1;
            sendto(ss, &last_ack, sizeof(last_ack), 0, (struct sockaddr *)&cladd, clen);
        }
    }

    close(ss);
    return 0;
}
