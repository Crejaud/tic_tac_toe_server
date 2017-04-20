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
  pthread_exit(NULL);
}