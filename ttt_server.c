#include "ttt_logic.h"

int spec_count = 0;
int current_board_index = 1;
int winning_player = 0;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
    exit(0);
  }

  // START SERVER CODE HERE
  int listenfd;        /* The server's listening descriptor */
  int port;            /* The port the server is listening on */
  socklen_t clientlen; /* Size in bytes of the client socket address */
  struct sockaddr_in clientaddr; /* Client address struct */
  int is_player_1_taken =
      FALSE; /* Player 1 boolean to check if player 1 is taken */
  int is_player_2_taken =
      FALSE;     /* Player 2 boolean to check if player 1 is taken */
  int p1_fd;     /* Player 1 socket fd */
  int p2_fd;     /* Player 2 socket fd */
  int spec_fd;   /* Spectator socket fd */
  pthread_t tid; /* Pthread used for processing spectators, p1, and p2 */

  // Initialize board_history

  // there can be at most 10 different boards in tic tac toe
  board_history = (char ***)Malloc(MAX_BOARD_HISTORY * sizeof(char **));
  for (int i = 0; i < MAX_BOARD_HISTORY; i++) {
    board_history[i] = NULL;
  }

  // Ensure every board is set to NULL
  clear_board_history();

  for (int i = 0; i < 1; i++) {
    board_history[i] = (char **)Malloc(BOARD_HEIGHT * sizeof(char *));
    for (int j = 0; j < BOARD_HEIGHT; j++) {
      board_history[i][j] = (char *)Malloc(BOARD_WIDTH * sizeof(char));
      for (int k = 0; k < BOARD_WIDTH; k++) {
        board_history[i][j][k] = EMPTY;
      }
    }
  }

  /* Create a server file descriptor */
  port = atoi(argv[1]);
  listenfd = Open_listenfd(port);

  // initialize player 1 to not be taken
  is_player_1_taken = FALSE;
  p1_fd = 0;
  // initialize player 2 to not be taken
  is_player_2_taken = FALSE;
  p2_fd = 0;

  // Initialize mutex
  Pthread_mutex_init(&mtx, NULL);
  Pthread_mutex_init(&current_board_index_mtx, NULL);

  // Initialize cv
  pthread_cond_init(&current_board_index_cv, NULL);

  printf("starting to listen\n");

  // start listening for anything
  for (;;) {
    clientlen = sizeof(clientaddr);
    printf("listening\n");

    if (is_player_1_taken == FALSE) {
      p1_fd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      printf("Accepting player 1\n");
      is_player_1_taken = TRUE;

      // construct argument to
      request_arg *targ = construct_request_arg(p1_fd, clientaddr, clientlen);

      // Create thread and call process_request
      Pthread_create(&tid, NULL, process_p1_thread_func, targ);
    } else if (is_player_2_taken == FALSE) {
      p2_fd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      printf("Accepting player 2\n");
      is_player_2_taken = TRUE;

      // construct argument to
      request_arg *targ = construct_request_arg(p2_fd, clientaddr, clientlen);

      // Create thread and call process_request
      Pthread_create(&tid, NULL, process_p2_thread_func, targ);
    } else {
      // is a spectator!
      spec_fd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
      printf("Accepting spectator\n");
      spec_count++;

      // construct argument to
      request_arg *targ = construct_request_arg(spec_fd, clientaddr, clientlen);

      // Create thread and call process_request
      Pthread_create(&tid, NULL, process_spec_thread_func, targ);
    }
  }
}
