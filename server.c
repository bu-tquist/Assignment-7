#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "game.h"

#define PORT 8080
#define SAVE_FILE "games.dat"

void save_server_state(struct session games[], int game_counter) {
    int fd = open(SAVE_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        write(fd, &game_counter, sizeof(game_counter));
        write(fd, games, sizeof(struct session) * MAX_GAMES);
        close(fd);
    }
}

void load_server_state(struct session games[], int *game_counter) {
    int fd = open(SAVE_FILE, O_RDONLY);
    if (fd >= 0) {
        read(fd, game_counter, sizeof(*game_counter));
        read(fd, games, sizeof(struct session) * MAX_GAMES);
        close(fd);
    }
}

int main(int argc, char* argv[]) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        return 1;
    }

    printf("Message board server listening on port %d\n", PORT);

    struct session games[MAX_GAMES];
    memset(games, 0, sizeof(games));
    int game_counter = 0;

    if (argc > 1 && strcmp(argv[1], "1") == 0) {
        load_server_state(games, &game_counter);
        printf("[Server] Recovered %d game sessions from disk crash state.\n", game_counter);
    }

    struct packet incoming;
    socklen_t address_len = sizeof(client_addr);

    while(1){
        address_len = sizeof(client_addr);
        ssize_t bytes_rec = recvfrom(sock_fd, &incoming, sizeof(incoming), 0,
                                     (struct sockaddr*)&client_addr, &address_len);

        if(bytes_rec < (ssize_t)sizeof(incoming)){
            continue;
        }

        int cmd = incoming.cmd;
        int pid = incoming.player_id;
        int column = incoming.col;
        int gid = incoming.game_id;

        if(cmd == 1){
            if (game_counter < MAX_GAMES) {
                int new_gid = game_counter++;
                struct session *s = &games[new_gid];
                
                s->p0_connected = 1;
                memcpy(&s->player_0, &client_addr, sizeof(client_addr));
                new_game(&s->state, new_gid);
                s->current_turn = 0;

                struct packet out_pkt;
                memset(&out_pkt, 0, sizeof(out_pkt));
                out_pkt.cmd = 4;
                out_pkt.state = -1;
                out_pkt.game_id = new_gid;
                out_pkt.player_id = 0;
                out_pkt.current_turn = 0;
                out_pkt.ready = 0;
                memcpy(out_pkt.board, s->state.board, sizeof(s->state.board));

                save_server_state(games, game_counter);
                sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0, (struct sockaddr *)&s->player_0, sizeof(s->player_0));
            }
            continue;
        }
        else if(cmd == 2){
            if (gid >= 0 && gid < game_counter) {
                struct session *s = &games[gid];
                if (!s->p1_connected) {
                    s->p1_connected = 1;
                    memcpy(&s->player_1, &client_addr, sizeof(client_addr));

                    struct packet out_pkt;
                    memset(&out_pkt, 0, sizeof(out_pkt));
                    out_pkt.cmd = 4;
                    out_pkt.state = -1;
                    out_pkt.game_id = gid;
                    out_pkt.current_turn = 0;
                    out_pkt.ready = 1;
                    memcpy(out_pkt.board, s->state.board, sizeof(s->state.board));

                    save_server_state(games, game_counter);
                    
                    out_pkt.player_id = 0;
                    sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0, (struct sockaddr *)&s->player_0, sizeof(s->player_0));
                    
                    out_pkt.player_id = 1;
                    sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0, (struct sockaddr *)&s->player_1, sizeof(s->player_1));
                }
            }
            continue;   
        }

        if(cmd == 3){
            if (gid < 0 || gid >= game_counter) continue;
            struct session *s = &games[gid];

            if(!s->p0_connected || !s->p1_connected || pid != s->current_turn){
                continue;
            }
            if(make_move(&s->state, pid, column) == -1){
                continue;
            }

            int gameState = check_win(&s->state);

            if(gameState == -1){
                int full = 1;
                for(int i = 0; i < 42; i++){
                    if(s->state.board[i] == -1){
                        full = 0;
                        break;
                    }
                }
                if(full) gameState = 2;
            }

            if(gameState == -1){
                s->current_turn = (s->current_turn == 0) ? 1 : 0;
            }

            struct packet out_pkt;
            memset(&out_pkt, 0, sizeof(out_pkt));
            out_pkt.cmd = 4;
            out_pkt.state = gameState;
            out_pkt.game_id = gid;
            out_pkt.col = column;
            out_pkt.current_turn = s->current_turn;
            out_pkt.ready = 1;
            memcpy(out_pkt.board, s->state.board, sizeof(s->state.board));

            save_server_state(games, game_counter);

            if(s->p0_connected){
                out_pkt.player_id = 0;
                sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0, (struct sockaddr*)&s->player_0, sizeof(s->player_0));
            }
            if(s->p1_connected){
                out_pkt.player_id = 1;
                sendto(sock_fd, &out_pkt, sizeof(out_pkt), 0, (struct sockaddr*)&s->player_1, sizeof(s->player_1));
            }

            if(gameState != -1){
                memset(s, 0, sizeof(struct session));
                save_server_state(games, game_counter);
            }
        }
    }

    close(sock_fd);
    return 0;
}
