/**
 * More info?
 * a.dotreppe@aspyct.org
 * http://aspyct.org
 * @aspyct (twitter)
 *
 * Hope it helps :)
 */
// $ kill -l
 // 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL       5) SIGTRAP
 // 6) SIGABRT      7) SIGEMT       8) SIGFPE       9) SIGKILL     10) SIGBUS
// 11) SIGSEGV     12) SIGSYS      13) SIGPIPE     14) SIGALRM     15) SIGTERM
// 16) SIGURG      17) SIGSTOP     18) SIGTSTP     19) SIGCONT     20) SIGCHLD
// 21) SIGTTIN     22) SIGTTOU     23) SIGIO       24) SIGXCPU     25) SIGXFSZ
// 26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGPWR      30) SIGUSR1
// 31) SIGUSR2     32) SIGRTMIN    33) SIGRTMIN+1  34) SIGRTMIN+2  35) SIGRTMIN+3
// 36) SIGRTMIN+4  37) SIGRTMIN+5  38) SIGRTMIN+6  39) SIGRTMIN+7  40) SIGRTMIN+8
// 41) SIGRTMIN+9  42) SIGRTMIN+10 43) SIGRTMIN+11 44) SIGRTMIN+12 45) SIGRTMIN+13
// 46) SIGRTMIN+14 47) SIGRTMIN+15 48) SIGRTMIN+16 49) SIGRTMAX-15 50) SIGRTMAX-14
// 51) SIGRTMAX-13 52) SIGRTMAX-12 53) SIGRTMAX-11 54) SIGRTMAX-10 55) SIGRTMAX-9
// 56) SIGRTMAX-8  57) SIGRTMAX-7  58) SIGRTMAX-6  59) SIGRTMAX-5  60) SIGRTMAX-4
// 61) SIGRTMAX-3  62) SIGRTMAX-2  63) SIGRTMAX-1  64) SIGRTMAX


#include <stdio.h>
#include <stdlib.h> 
#include <signal.h> // sigaction(), sigsuspend(), sig*()
#include <unistd.h> // alarm()
#include <fcntl.h>
#include <string.h>

#include "tic_tac_toe.h"

void handle_signal(int signal);
void handle_sigalrm(int signal);
void do_sleep(int seconds);
bool oponent_done=false;
tic_tac_toe *game = new tic_tac_toe();

/* Usage example
 * 
 * First, compile and run this program:
 *     $ gcc signal.c
 *     $ ./a.out
 * 
 * It will print out its pid. Use it from another terminal to send signals
 *     $ kill -HUP <pid>
 *     $ kill -USR1 <pid>
 *     $ kill -ALRM <pid>
 *
 * Exit the process with ^C ( = SIGINT) or SIGKILL, SIGTERM
 */
int main(int argc, char **argv) {
  struct sigaction sa;
  char player;
  FILE *inFile, *outFile;
  char buffer1[128], buffer2[128];
  char x_file_name[] = "xmove.txt";
  char o_file_name[] = "omove.txt";
  char *my_filename, *oponent_filename;
  int my_pid, oponent_pid;
  int size;
  int turn = 0;
  
  // tic_tac_toe *game = new tic_tac_toe();

  if (argc != 2) {
    printf ("Usage: sig_tic_tac_toe [X|O] \n");
    return (-1);
  }
  player = argv[1][0];
  if (player != 'X' && player != 'O') {
    printf ("Usage: player names must be either X or O");
    return (-2);
  }
  if (player == 'X') {
    my_filename = x_file_name;
    oponent_filename = o_file_name;
  }
  else {
    my_filename = o_file_name;
    oponent_filename = x_file_name;
  }

  // Print pid, so that we can send signals from other shells
  my_pid = getpid();
  printf("My pid is: %d\n", my_pid);
  
  outFile = fopen (my_filename, "w");
  fprintf (outFile, "%d", my_pid);
  fclose(outFile);
  
  printf ("trying to open input file:\n .");
  do {
    sleep(1);
    inFile = fopen (oponent_filename, "r");
    printf (".");
  } while (inFile == NULL);
  
  fgets( buffer1, 128, inFile );
  oponent_pid = atoi (buffer1);
  printf (" done. \noponent_pid %d \n", oponent_pid);
  fclose(inFile);

  //char integer_string[5];
  //sprintf(integer_string,"%d", oponent_pid);
  //printf (integer_string);
  //printf("\n");
  //char other_string[21]="kill –s SIGUSR1 ";
 
  //strcat(other_string,integer_string);
  //printf (other_string);
  //printf("\n");


  // Setup the sighub handler
  sa.sa_handler = &handle_signal;
  // Restart the system call, if at all possible
  sa.sa_flags = SA_RESTART;
  // Block every signal during the handler
  sigfillset(&sa.sa_mask);
  // Intercept SIGUSR1 and SIGINT
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
      perror("Error: cannot handle SIGUSR1");
  }
  if (sigaction(SIGINT, &sa, NULL) == -1) {
      perror("Error: cannot handle SIGINT"); // Should not happen
  }

  while(true){
    if(player=='X'){
       if(turn%2==0){ // turn is even, X's turn
	game->display_game_board();
	game->get_player_move(player);
    	game->display_game_board();
    	turn++;
	int fd = open(my_filename, O_WRONLY);
	char* ptr = game->convert2string();
	char str[10];
	strcpy(str,ptr);
	write(fd, str, strlen(str));
	close(fd);
	//system(other_string);
	kill(oponent_pid,SIGUSR1);
	if(game->game_result() != '-'){
	  break;
        }
       } else { //turn is odd, O's turn
	  while(!oponent_done){
	    sleep(1);
	  }
	  oponent_done=false;
          int fd = open(oponent_filename, O_RDONLY);
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
	  while(!oponent_done){
	    sleep(2);
	  }
	  oponent_done=false;
          int fd = open(oponent_filename, O_RDONLY);
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
	int fd = open(my_filename, O_WRONLY);
	char* ptr = game->convert2string();
	char str[10];
	strcpy(str,ptr);
	write(fd, str, strlen(str));
	close(fd);
        //system(other_string);
	kill(oponent_pid,SIGUSR1);
	if(game->game_result() != '-'){
	  break;
        }
      }
    }
  }

  game->display_game_board();
  printf ("Game finished, result: %c \n", game->game_result());
  remove (oponent_filename);
  return (0);

}

void handle_signal(int signal) {
  sigset_t pending;

  if (signal == SIGUSR1){
    oponent_done=true;
    //printf ("received SIGUSR1 signal \n");
  }
  else if (signal == SIGINT) exit(0); // CTRL C exit
  else return;
  //printf("Done handling SIGUSR1\n\n");
}

