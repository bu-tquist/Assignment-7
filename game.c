#include "game.h"
#include <stdlib.h>
#include <stddef.h>

/*
Takes as input a pointer to a connect4 game board, a player (0/1), and a int column.
Places a new piece in `col` on the game board.
*/
int make_move(struct game* g, int player, int col) {
    if(g == NULL || col < 0 || col >= BOARD_COLS){
        return -1;
    }

    char piece = (player == 0) ? 'X' : 'O';

    for(int r = BOARD_ROWS - 1; r >= 0; r--){
        if(g->board.grid[r][col] == '.'){
            g->board.grid[r][col] == piece;
            return r;
        }
    }

    return -1;
}
 
/*
Takes as input a pointer to a connect4 game board.
Returns 0 or 1 if player 0 or 1 has won respectively.
Returns -1 if there is no winner.
*/
int check_win(struct game* g) {
   if(g == NULL){
    return -1;
   }

   int dr[] = {0, 1, 1, -1};
   int dc[] = {1, 0, 1, 1};

    for(int r = 0; r < BOARD_ROWS; r++){
        for(int c = 0; c < BOARD_COLS; c++){
            char current_piece = g->board.grid[r][c];

            if(current_piece == '.'){
                continue;
            }

            for(int d = 0; d < 4; d++){
                int count = 1;

                for(int step = 1; step < 4; step++){
                    int next_r = r + dr[d] * step;
                    int next_c = c + dc[d] * step;

                    if(next_r >= 0 && next_r < BOARD_ROWS && next_c >= 0 && next_c < BOARD_COLS){
                        if(g->board.grid[next_r][next_c] == current_piece){
                            count++;
                        }
                        else {
                            break;
                        }
                    }
                    else{
                        break;
                    }
                }
                if(count == 4){
                    return (current_piece == 'X') ? 0 : 1;
                }
            }

        }
   }

    return -1;
}
 
/*
Initializes an existing game with an empty board.
*/
void new_game(struct game* g, int gameId) {
   if(gameId == NULL){
    return;
   }

   g->game_id = gameId;

   for(int r = 0; r < BOARD_ROWS; r++){
    for(int c = 0; c < BOARD_COLS; c++){
        g->board.grid[r][c] = '.';
    }
   }
}


