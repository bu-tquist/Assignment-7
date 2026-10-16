#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "game.h"
#include "save.h"

#define PORT 8080


int main(int argc, char* argv[]) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Message board server listening on port %d\n", PORT);

   struct session s;
   memset(&s, 0, sizeof(s));

   struct packet incoming;
   socklen_t address_len = sizeof(client_addr);

   while(1){
     ssize_t bytes_rec = recvfrom(sock_fd, &incoming, sizeof(incoming), 0
    ,(struct sockaddr*)&client_addr, &address_len);

    int cmd =  incoming.cmd;
    int pid =  incoming.player_id;
    int column =  incoming.col;

    if(cmd == 1){
        if(!s.p0_connected){
            s.p0_connected = 1;
            s.player_0 = client_addr;
            new_game(&s.state, 0);
            s.current_turn = 0;
        }
        continue;
    }
    else if(cmd == 2){
         if(!s.p1_connected){
            s.p1_connected = 1;
            s.player_1 = client_addr;

            struct packet update;
            memset(&update , 0, sizeof(update));
            update.cmd = 4;
            update.state = -1;
            memcpy(update.board, s.state.board, sizeof(s.state.board));

            sendto(sock_fd, &update, sizeof(update), 0, (struct sockaddr *)&s.player_0, sizeof(s.player_0));
            sendto(sock_fd, &update, sizeof(update), 0, (struct sockaddr *)&s.player_1, sizeof(s.player_1));
        }

     continue;   
    }


    if(cmd == 3){
        make_move(&s.state, pid, column);

        int gameState = check_win(&s.state);

        if(gameState == -1){
            int full = 1;
            for(int i = 0; i < 42; i++){
                if(s.state.board[i] == -1){
                    full = 0;
                    break;
                }
            }
            if(full) gameState = 2;
        }

        struct packet out_pkt;
        memset(&out_pkt, 0, sizeof(out_pkt));
        out_pkt.cmd = 4;
        out_pkt.state = gameState;

        memcpy(out_pkt.board, s.state.board, sizeof(s.state.board));

        if(gameState == -1){
            s.current_turn = (s.current_turn == 0) ? 1 : 0;
        }

        if(s.p0_connected){
            sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0,
            (struct sockaddr*)&s.player_0, sizeof(s.player_0) );
        }
         if(s.p1_connected){
            sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0,
            (struct sockaddr*)&s.player_1, sizeof(s.player_1) );
        }

        if(gameState != -1){
            memset(&s, 0, sizeof(s));
        }
    }
   }

    close(sock_fd);

    return 0;
}