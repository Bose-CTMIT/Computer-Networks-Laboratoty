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
    int n, wnd;
    struct sockaddr_in sadd;
    socklen_t slen = sizeof(sadd);

    printf("Enter the size of file (this file will be broken into packets of size 1): ");
    scanf("%d", &n);
    printf("Enter the window size: ");
    scanf("%d", &wnd);

    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORT);
    sadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sock, &wnd, sizeof(wnd), 0, (struct sockaddr *)&sadd, sizeof(sadd));

    struct packet pk[n];
    for(int i = 0; i < n; i++){
        pk[i].no = i;
        pk[i].ack = 0;
        pk[i].seq = i;  // Assign sequence numbers in order
    }

    int base = 0, next_seq_num = 0, ack = 0;

    while(base < n){
        
        while(next_seq_num < base + wnd && next_seq_num < n){
            printf("Sending packet %d\n", next_seq_num);
            sendto(sock, &pk[next_seq_num], sizeof(pk[next_seq_num]), 0, (struct sockaddr *)&sadd, sizeof(sadd));
            next_seq_num++;
        }

    
        recvfrom(sock, &ack, sizeof(ack), 0, (struct sockaddr *)&sadd, &slen);
        printf("Received ACK for packet %d\n", ack);

        if(ack >= base){
            base = ack + 1;  // Move the window
        } else {
            // If ACK is out of order or lost, retransmit all unacknowledged packets
            printf("ACK %d received, retransmitting from packet %d\n", ack, base);
            next_seq_num = base;
        }
    }

    printf("All packets sent successfully.\n");
    close(sock);
}
