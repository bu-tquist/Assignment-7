#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define PORT 8080

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
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Connect 4\n");
    printf("  1) Create a new game\n");
    printf("  2) Join a game\n");
    printf("> ");
    int choice = 0;
    if (scanf("%d", &choice) != 1) return 1;

    int assigned_play = (choice == 1) ? 0 : 1;
    if(assigned_play == 0){
        printf("\n");
    }
    int opp_play = (assigned_play == 0) ? 1 : 0;

    struct packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd = choice;
    pkt.player_id = assigned_play;

    if(choice == 2){
        printf("Game ID? >\n");
        if (scanf("%d", &pkt.game_id) != 1) {
            pkt.game_id = 0;
        }
    }

    sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    socklen_t addr_len = sizeof(server_addr);
    int authoritative_turn = 0; 
    int is_game_ready = 0;     

    while(1){
        struct packet game_pkt;
        ssize_t bytes_rec = recvfrom(sock_fd, &game_pkt, sizeof(game_pkt), 0,
                                     (struct sockaddr*)&server_addr, &addr_len);

        if (bytes_rec < (ssize_t)sizeof(game_pkt)) {
            continue;
        }

        authoritative_turn = game_pkt.current_turn;
        is_game_ready = game_pkt.ready;

        printf("Current Game\n");
        printf("---------------------\n");
        printf("GameID: %d\n", game_pkt.game_id);

        print_board(game_pkt.board);

        if(game_pkt.state == assigned_play){
            printf("YOU WIN!\n");
            break;
        }
        else if(game_pkt.state == opp_play){
            printf("YOU LOSE!\n");
            break;
        }
        else if(game_pkt.state == 2){
            printf("TIE\n");
            break;
        }

        if (!is_game_ready || assigned_play != authoritative_turn) {
            printf("waiting for other player...\n\n");
        } 
        else if (is_game_ready && assigned_play == authoritative_turn) {
            int col = -1;
            while(col < 0 || col > 6){
                printf("\nInput Move Row: > \n");
                if(scanf("%d", &col) != 1){
                    if (feof(stdin)) {
                        close(sock_fd);
                        return 0;
                    }
                    while(getchar() != '\n');
                    continue;
                }
            }

            pkt.cmd = 3;
            pkt.game_id = game_pkt.game_id;
            pkt.player_id = assigned_play;
            pkt.col = col;

            sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }
    }

    close(sock_fd);
    return 0;
}
