#include "ttt_logic.h"

void unix_error(char *msg) /* unix-style error */
{
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(0);
}

/****************************
 * Sockets interface wrappers
 ****************************/

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  int rc;

  if ((rc = accept(s, addr, addrlen)) < 0)
    unix_error("Accept error");
  return rc;
}

/***************************************************
 * Wrappers for dynamic storage allocation functions
 ***************************************************/

void *Malloc(size_t size) {
  void *p;

  if ((p = malloc(size)) == NULL)
    unix_error("Malloc error");
  return p;
}

/************************************************
 * Wrappers for Pthreads thread control functions
 ************************************************/

void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void *(*routine)(void *), void *argp) {
  int rc;

  if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
    posix_error(rc, "Pthread_create error");
}

void Pthread_cancel(pthread_t tid) {
  int rc;

  if ((rc = pthread_cancel(tid)) != 0)
    posix_error(rc, "Pthread_cancel error");
}

void Pthread_join(pthread_t tid, void **thread_return) {
  int rc;

  if ((rc = pthread_join(tid, thread_return)) != 0)
    posix_error(rc, "Pthread_join error");
}

void Pthread_detach(pthread_t tid) {
  int rc;

  if ((rc = pthread_detach(tid)) != 0)
    posix_error(rc, "Pthread_detach error");
}


/*******************************
 * Wrappers for Posix mutex
 *******************************/

void Pthread_mutex_init(pthread_mutex_t *mtx, const pthread_mutexattr_t *attr) {
  if (pthread_mutex_init(mtx, NULL) < 0)
    unix_error("Pthread_mutex_init error");
}

void Pthread_mutex_lock(pthread_mutex_t *mtx) {
  if (pthread_mutex_lock(mtx) < 0)
    unix_error("Pthread_mutex_lock error");
}

void Pthread_mutex_unlock(pthread_mutex_t *mtx) {
  if (pthread_mutex_unlock(mtx) < 0)
    unix_error("Pthread_mutex_unlock error");
}

/*
 * open_listenfd - open and return a listening socket on port
 *     Returns -1 and sets errno on Unix error.
 */
int open_listenfd(int port) {
  int listenfd, optval = 1;
  struct sockaddr_in serveraddr;

  /* Create a socket descriptor */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;

  /* Eliminates "Address already in use" error from bind. */
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                 sizeof(int)) < 0)
    return -1;

  /* Listenfd will be an endpoint for all requests to port
     on any IP address for this host */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)port);
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    return -1;

  /* Make it a listening socket ready to accept connection requests */
  if (listen(listenfd, LISTENQ) < 0)
    return -1;
  return listenfd;
}

/* Wrapper for server helper routines */
int Open_listenfd(int port) {
  int rc;

  if ((rc = open_listenfd(port)) < 0)
    unix_error("Open_listenfd error");
  return rc;
}

/* Process player 1 */
void process_p1(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {

}

/* Process player 2 */
void process_p2(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {

}

/* Process spectator */
void process_spec(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {

}

/* Thread function for calling process_p1 */
void process_p1_thread_func(void *args) {
  request_arg *targ = args;
  process_p1(targ->connfd, targ->clientaddr, targ->clientlen);
  free(targ);
  pthread_exit(NULL);
}
/* Thread function for calling process_p2 */
void process_p2_thread_func(void *args) {
  request_arg *targ = args;
  process_p2(targ->connfd, targ->clientaddr, targ->clientlen);
  free(targ);
  pthread_exit(NULL);
}
/* Thread function for calling process_spec */
void process_spec_thread_func(void *args) {
  request_arg *targ = args;
  process_spec(targ->connfd, targ->clientaddr, targ->clientlen);
  free(targ);
  spec_count--;
  pthread_exit(NULL);
}

/* Clear the board history */
void clear_board_history() {
  for (int i = 0; i < MAX_BOARD_HISTORY; i++) {
    if (board_history[i] != NULL) {
      for (int j = 0; j < BOARD_HEIGHT; j++) {
        free(board_history[i][j]);
      }

      free(board_history[i]);
    }
    board_history[i] = NULL;
  }
}

request_arg* construct_request_arg(int connfd, struct sockaddr_in clientaddr, socklen_t clientlen) {
  request_arg *arg = (request_arg *) Malloc(sizeof(request_arg));
  arg->connfd = connfd;
  arg->clientaddr = clientaddr;
  arg->clientlen = clientlen;

  return targ;
}
