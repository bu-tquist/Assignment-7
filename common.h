#include <arpa/inet.h>
#include <sys/socket.h>


#ifndef COMMON_H
#define COMMON_H

#define BOARD_COLS 7
#define BOARD_ROWS 6
// Define any structs you may find helpful here.

struct board{
    char grid[BOARD_ROWS][BOARD_COLS];
};

struct game{
    int game_id;
    struct board board;
};

#endif