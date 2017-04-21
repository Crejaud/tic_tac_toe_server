#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 0
#define FALSE 1

#define EMPTY 0
#define X 1
#define O 2

#define MAX_BOARD_HISTORY 10
#define BOARD_WIDTH 3
#define BOARD_HEIGHT 3

#define MAX_LINE 1024

// Our mutex
pthread_mutex_t mtx;

// The history of the board
char*** board_history;

// To join the server, this is the key that needs to be requested
char* JOIN_KEY = "join";

// Number of spectators
int spec_count = 0;

// Current board index
int current_board_index = 0;
pthread_cond_t current_board_index_cv;

/* This struct is for sending the necessary arguements to a thread to handle a socket */
typedef struct {
        int connfd;                     /* the sock fd for the client */
        struct sockaddr_in clientaddr;  /* the sockaddr_in for the client */
        socklen_t clientlen;            /* the client address length */
} request_arg;

/* Persistent state for the robust I/O (Rio) package */
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
        int rio_fd;            /* descriptor for this internal buf */
        int rio_cnt;           /* unread bytes in internal buf */
        char *rio_bufptr;      /* next unread byte in internal buf */
        char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;
/* $end rio_t */

void unix_error(char *msg);

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* Pthreads thread control wrappers */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void * (*routine)(void *), void *argp);
void Pthread_join(pthread_t tid, void **thread_return);
void Pthread_cancel(pthread_t tid);
void Pthread_detach(pthread_t tid);

/* POSIX mutex wrappers */
void Pthread_mutex_init(pthread_mutex_t *mtx, const pthread_mutexattr_t *attr);
void Pthread_mutex_lock(pthread_mutex_t *mtx);
void Pthread_mutex_unlock(pthread_mutex_t *mtx);

/* Dynamic storage allocation wrappers */
void *Malloc(size_t size);

int open_listenfd(int portno);
/* Wrapper for open_listenfd */
int Open_listenfd(int portno);

void process_p1(int fd, struct sockaddr_in clientaddr, socklen_t clientlen);
void process_p2(int fd, struct sockaddr_in clientaddr, socklen_t clientlen);
void process_spec(int fd, struct sockaddr_in clientaddr, socklen_t clientlen);

void process_p1_thread_func(void *args);
void process_p2_thread_func(void *args);
void process_spec_thread_func(void *args);

void clear_board_history();

request_arg* construct_request_arg(int connfd, struct sockaddr_in clientaddr, socklen_t clientlen);

/* Rio (Robust I/O) package */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Wrappers for Rio package */
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);
void Rio_writen(int fd, void *usrbuf, size_t n);
void Rio_readinitb(rio_t *rp, int fd);
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
