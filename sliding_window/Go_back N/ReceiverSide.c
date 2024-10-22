#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 8090

struct packet{
    int no, ack, seq;
};

int main(){
    int ss;
    struct sockaddr_in sadd, cladd;
    socklen_t clen = sizeof(cladd);  // Proper declaration
    struct packet pk;
    int wnd;

    ss = socket(AF_INET, SOCK_DGRAM, 0);
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = INADDR_ANY;

    if (bind(ss, (struct sockaddr *)&sadd, sizeof(sadd)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    recvfrom(ss, &wnd, sizeof(wnd), 0, (struct sockaddr *)&cladd, &clen);  // Correct recvfrom usage
    printf("Received window size: %d\n", wnd);

    int expected_seq = 0;  // Expected sequence number for the next packet
    while (1) {
        recvfrom(ss, &pk, sizeof(pk), 0, (struct sockaddr *)&cladd, &clen);
        
        if (pk.seq == expected_seq) {
            // Packet received in order
            printf("Received packet %d\n", pk.no);
            pk.ack = pk.seq;
            sendto(ss, &pk.ack, sizeof(pk.ack), 0, (struct sockaddr *)&cladd, clen);  // Correct sendto usage
            expected_seq++;  // Move to the next expected sequence
        } else {
            // Out of order packet, resend last acknowledged packet
            printf("Packet %d out of order, expected %d\n", pk.no, expected_seq);
            int last_ack = expected_seq - 1;
            sendto(ss, &last_ack, sizeof(last_ack), 0, (struct sockaddr *)&cladd, clen);  // Correct sendto usage
        }
    }

    return 0;
}
