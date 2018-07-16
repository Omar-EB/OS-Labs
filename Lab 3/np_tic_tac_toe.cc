
#include <stdio.h>
#include <stdlib.h> 
#include <signal.h> // sigaction(), sigsuspend(), sig*()
#include <unistd.h> // alarm()
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tic_tac_toe.h"


/* Usage example
 * 
 * First, compile and run this program:
 *     $ gcc np_tic_tac_toe.c -o np_tic_tac_toe
 *     if not already created, create a named pipe (FIFO)
 *     $ mkfifo my_pipe 
 *
 *     In two separe shells run:
 *     $ ./np_tic_tac_toe X 
 *     $ ./np_tic_tac_toe O 
 * 
 *
 */
 
 
int main(int argc, char **argv) {
  char player;
  tic_tac_toe *game = new tic_tac_toe();
  int turn = 0;
  char myfifo[128] = "my_pipe";

  if (argc != 2) {
    printf ("Usage: sig_tic_tac_toe [X|O] \n");
    return (-1);
  }
  player = argv[1][0];
  if (player != 'X' && player != 'O') {
    printf ("Usage: player names must be either X or O");
    return (-2);
  }

  while(true){
    if(player=='X'){
       if(turn%2==0){ // turn is even, X's turn
	game->display_game_board();
	game->get_player_move(player);
    	game->display_game_board();
    	turn++;
	int fd = open(myfifo, O_WRONLY);
	char* ptr = game->convert2string();
	char str[10];
	strcpy(str,ptr);
	write(fd, str, strlen(str));
	close(fd);
	if(game->game_result() != '-'){
	  break;
        }
       } else { //turn is odd, O's turn
          int fd = open(myfifo, O_RDONLY);
	  char str[10];
	  read(fd, str, 10);
	  char* ptr = (char*) malloc(strlen(str));
	  strcpy(ptr,str);
	  game->set_game_state(ptr);
	  close(fd);
	  if(game->game_result() != '-'){
	   break;
          }
	  turn++;
       }
    }
    if(player=='O'){
       if(turn%2==0){ // turn is even, X's turn
          int fd = open(myfifo, O_RDONLY);
	  char str[10];
	  read(fd, str, 10);
	  char* ptr = (char*) malloc(strlen(str));
	  strcpy(ptr,str);
	  game->set_game_state(ptr);
	  close(fd);
	  if(game->game_result() != '-'){
	   break;
          }
	  turn++;
       } else { //turn is odd, O's turn
	game->display_game_board();
	game->get_player_move(player);
    	game->display_game_board();
    	turn++;
	int fd = open(myfifo, O_WRONLY);
	char* ptr = game->convert2string();
	char str[10];
	strcpy(str,ptr);
	write(fd, str, strlen(str));
	close(fd);
	if(game->game_result() != '-'){
	  break;
        }
      }
    }
  }

  game->display_game_board();
  printf ("Game finished, result: %c \n", game->game_result());

  return (0);
}
