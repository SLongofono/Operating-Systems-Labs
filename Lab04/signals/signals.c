#include <stdio.h>     /* standard I/O functions                         */
#include <stdlib.h>    /* exit                                           */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

/* first, define the Ctrl-C counter, initialize it with zero. */

// Use globals since we need cheap flags for signal handling
int ctrl_c_count = 0;
int got_response = 0;
#define CTRL_C_THRESHOLD  5

/* the Ctrl-C signal handler "SIGINT"*/
void catch_int(int sig_num)
{

  // According to the manpages, we shouldn't be calling printf here because it
  // is not among the functions deemed safe for use within singal handlers.
  // Apparently there are many opportunities for race conditions, and only
  // certain functions have been cleared by the POSIX standard.

  /* increase count, and check if threshold was reached */
  ctrl_c_count++;
  if (ctrl_c_count >= CTRL_C_THRESHOLD) {
    char answer[30];

    /* prompt the user to tell us if to really
     * exit or not */
    printf("\nReally exit? [Y/n]: ");
    fflush(stdout);

    // Trigger a 10 second timeout
    alarm(10);
    fgets(answer, sizeof(answer), stdin);

    // Signal the alarm handler that all is well
    got_response = 1;

    if (answer[0] == 'n' || answer[0] == 'N') {
      printf("\nContinuing\n");
      fflush(stdout);
      /* 
       * Reset Ctrl-C counter
       */
      ctrl_c_count = 0;
    }
    else {
      printf("\nExiting...\n");
      fflush(stdout);
      exit(0);
    }
  }
}

/* the alarm signal handler SIGALRM */
void catch_alrm(int sig_num){
	if(!got_response){
		printf("Timed out, exiting...\n");
		exit(0);
	}
}


/* the Ctrl-Z signal handler SIGTSTP*/
void catch_tstp(int sig_num)
{

  // According to the manpages, we shouldn't be calling printf here because it
  // is not among the functions deemed safe for use within singal handlers.
  // Apparently there are many opportunities for race conditions, and only
  // certain functions have been cleared by the POSIX standard.

  /* print the current Ctrl-C counter */
  printf("\n\nSo far, '%d' Ctrl-C presses were counted\n\n", ctrl_c_count);
  fflush(stdout);
}

int main(int argc, char* argv[])
{
  struct sigaction sa;

  sigset_t mask_set;  /* used to set a signal masking set. */
  // Signal masks describe the behavior of the code within the specific
  // handler to which they are assigned.


  // Signals are masked backwards from everything else, ever(1 is blocked, 0 is passed)
  // So where we normally would have 1111 & 0010 = 0010, we instead get the
  // counter-intuitive 1111 & 0010 = 1101
  //
  // Thus when we fill the mask set, we are blocking everything
  // Thus when we delete an entry, we are allowing it to be passed

  // Initialize our mask type to include no signals
  sigfillset(&mask_set);



  // Add in the first signal we want to account for to the mask for the first
  // handler.  Since alarm is used within the handler, we need to allow it.
  sigdelset(&mask_set, SIGALRM);
  sa.sa_handler = &catch_int;

  // Register the new handler
  sigaction(SIGINT, &sa, NULL);


  // Add in the second signal we want to account for in the second handler
  sigfillset(&mask_set);
  sa.sa_handler = &catch_tstp;

  // Register the new handler
  sigaction(SIGTSTP, &sa, NULL);

  // Add in the third signal we want to account for.
  sigfillset(&mask_set);
  sa.sa_handler = &catch_alrm;

  // Register new handler
  sigaction(SIGALRM, &sa, NULL);


/*
 * The following is an example of the simpler signal mechanism, which also
 * allows signal handlers to be assigned and manipulated.
 */

/*
if(SIG_ERR == signal(SIGINT, &catch_int)){
	printf("Failed to assign handler for SIGINT\n");	
	return(-1);
}

// Handle terminal stop (ctrl-z)
if(SIG_ERR == signal(SIGTSTP, &catch_tstp)){
	printf("Failed to assign handler for SIGTSTP\n");	
	return (-1);
}


// Handle alarm signal (SIGALRM)
//if(SIG_ERR == signal(SIGALRM, &catch_alrm)){
//	printf("Failed to assign handler for SIGALRM\n");
//	return(-1);
//}
//
*/

while(1){
	//do nothing until a signal is received	
	pause();
}
  return 0;
}

