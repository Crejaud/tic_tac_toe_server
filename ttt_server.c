#include "ttt_logic.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
    exit(0);
  }

  // START SERVER CODE HERE
  int listenfd;                   /* The server's listening descriptor */
  int port;                       /* The port the server is listening on */
  socklent_t clientlen;           /* Size in bytes of the client socket address */
  struct sockaddr_in clientaddr;  /* Client address struct */
  int is_player_1_taken = FALSE;  /* Player 1 boolean to check if player 1 is taken */
  int is_player_2_taken = FALSE;  /* Player 2 boolean to check if player 1 is taken */
  int p1_fd;                      /* Player 1 socket fd */
  int p2_fd;                      /* Player 2 socket fd */
  int spec_fd;                    /* Spectator socket fd */
  pthread_t tid;                  /* Pthread used for processing spectators, p1, and p2 */

  // Initialize board_history

  // there can be at most 10 different boards in tic tac toe
  board_history = (char ***) Malloc(MAX_BOARD_HISTORY * sizeof(char**));

  // Ensure every board is set to NULL
  clear_board_history();

  /* Create a server file descriptor */
  port = atoi(argv[1]);
  listen_fd = Open_listenfd(port);

  // initialize player 1 to not be taken
  is_player_1_taken = FALSE;
  p1_fd = 0;
  // initialize player 2 to not be taken
  is_player_2_taken = FALSE;
  p2_fd = 0;

  // Initialize mutex
  Pthread_mutex_init(&mtx, NULL);
  // Initialize cv
  pthread_cond_init(&current_board_index_cv, NULL);

  // start listening for anything
  for (;;) {
      clientlen = sizeof(clientaddr);

      if (is_player_1_taken == FALSE) {
        p1_fd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        is_player_1_taken = TRUE;

        // construct argument to
        request_arg *targ =  = construct_request_arg(p1_fd, clientaddr, clientlen);

        // Create thread and call process_request
        Pthread_create(&tid, NULL, process_p1_thread_func, targ);
      }
      else if (is_player_2_taken == FALSE) {
        p2_fd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        is_player_2_taken = TRUE;

        // construct argument to
        request_arg *targ = construct_request_arg(p2_fd, clientaddr, clientlen);

        // Create thread and call process_request
        Pthread_create(&tid, NULL, process_p2_thread_func, targ);
      }
      else {
        // is a spectator!
        spec_fd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);

        // Increment the number of spectators
        spec_count++;

        // construct argument to
        request_arg *targ =  = construct_request_arg(spec_fd, clientaddr, clientlen);

        // Create thread and call process_request
        Pthread_create(&tid, NULL, process_spec_thread_func, targ);
      }
  }

  // never reaches this.
  Close(listenfd);
  exit(0);
}
