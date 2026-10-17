#include <arpa/inet.h>
#include <sys/socket.h>


#ifndef COMMON_H
#define COMMON_H

#define BOARD_COLS 7
#define BOARD_ROWS 6
#define MAX_GAMES 50
// Define any structs you may find helpful here.


struct game{
    int game_id;
    int board[42]; //0 == X, 1 == O, -1 == empty
};

struct packet
{
    int cmd;
    int game_id;
    int player_id;
    int col;
    int board[42];
    int state;
    int ready;
    int current_turn;
};

struct session{
    struct game state;
    struct sockaddr_in player_0;
    struct sockaddr_in player_1;
    int current_turn;
    int p0_connected;
    int p1_connected;
};

struct gameDB{
    struct session games[MAX_GAMES];
    int game_counter;
};


#endif