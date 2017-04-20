#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define TRUE 0
#define FALSE 1

// Our mutex
pthread_mutex_t mtx;

/* This struct is for sending the necessary arguements to a thread to handle a socket */
typedef struct {
        int connfd;                     /* the sock fd for the client */
        struct sockaddr_in clientaddr;  /* the sockaddr_in for the client */
        socklen_t clientlen;            /* the client address length */
} request_arg;

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
