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

    int piece = (player == 0) ? 0 : 1;

    for(int r = BOARD_ROWS - 1; r >= 0; r--){
        
        int index = r * BOARD_COLS + col;

        if(g->board[index] == -1){
            g->board[index] = player;
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
            int current_index = r * BOARD_COLS + c;
            int current_player = g->board[current_index];

            if(current_player == -1){
                continue;
            }

            for(int d = 0; d < 4; d++){
                int count = 1;

                for(int step = 1; step < 4; step++){
                    int next_r = r + dr[d] * step;
                    int next_c = c + dc[d] * step;

                    if(next_r >= 0 && next_r < BOARD_ROWS && next_c >= 0 && next_c < BOARD_COLS){
                        int next_index =  next_r * BOARD_COLS + next_c;
                        if(g->board[next_index] == current_player){
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
                    return current_player;
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
   if(g == NULL){
    return;
   }

   g->game_id = gameId;

   for(int i = 0; i < 42; i++){
        g->board[i] = -1;
   }
}


