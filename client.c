#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096


// prints the 7x6 flattened board array in the correct format
void print_board(int board[]) {
 
    printf("\n");
    for (int col = 0; col < 7; col++)
        printf(" %d", col);
    printf("\n");
 
    for (int row = 0; row < 6; row++) {
        printf("|");
        for (int col = 0; col < 7; col++) {
            int cell = board[row * 7 + col];
            printf("%c|", cell == 0 ? 'X' : cell == 1 ? 'O' : '.');
        }
        printf("\n");
    }
    printf("\n");
}


int main() {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);


    printf("Connect 4\n");
    printf("  1) Create a new game\n");
    printf("  2) Join a game\n");
    printf("> ");
    int choice = 0;
    scanf("%d", &choice);

    int assigned_play =  (choice == 1) ? 0 : 1;

    int opp_play = (assigned_play == 0) ? 1 : 0;

    struct packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd = choice;
    pkt.player_id = assigned_play;

    sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    socklen_t addr_len = sizeof(server_addr);
    int current_turn = 0;

    while(1){
        if(assigned_play == current_turn){
            int col = -1;
            while(col < 0 || col > 6){
                printf("Input Move Row:");
                if(scanf("%d", &col) != 1){
                    while(getchar() != '\n');
                    continue;
                }
            }

            pkt.cmd = 3;
            pkt.player_id = assigned_play;
            pkt.col = col;

            sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            printf("Current Game\n");
            printf("--------------------");
            printf("GameID : %d", pkt.game_id);
        }
        struct packet game_pkt;
        ssize_t bytes_rec = recvfrom(sock_fd, &game_pkt, sizeof(game_pkt), 0
        ,(struct sockaddr*)&server_addr, &addr_len);

        print_board(game_pkt.board);

        if(game_pkt.state == assigned_play){
            printf("YOU WIN!");
            break;
        }
        else if(game_pkt.state == opp_play){
            printf("YOU LOSE!");
            break;
        }
        else if(game_pkt.state == 2){
            printf("TIE");
            break;
        }

        current_turn = (current_turn == 0) ? 1 : 0;
        printf("waiting for other player...");
    }

    close(sock_fd);

    return 0;
}