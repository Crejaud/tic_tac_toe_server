#include "ttt_logic.h"

void unix_error(char *msg) /* unix-style error */
{
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
        exit(0);
}

void posix_error(int code, char *msg) /* posix-style error */
{
        fprintf(stderr, "%s: %s\n", msg, strerror(code));
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

/* Tries to make a move, return 1 if move is made, 0 if invalid */
int make_move(int player, int x, int y) {


        printf("move coords %d %d\n", x, y);
        printf("current board index %d\n", current_board_index );
        for (int i = current_board_index; i < current_board_index+1; i++) {
                board_history[i] = (char **)Malloc(BOARD_HEIGHT * sizeof(char *));
                for (int j = 0; j < BOARD_HEIGHT; j++) {
                        board_history[i][j] = (char *)Malloc(BOARD_WIDTH * sizeof(char));
                        for (int k = 0; k < BOARD_WIDTH; k++) {
                                if(board_history[i-1][j][k] == X_LITERAL) {
                                        board_history[i][j][k] = X_LITERAL;
                                }
                                else if (board_history[i-1][j][k] == O_LITERAL) {
                                        board_history[i][j][k] = O_LITERAL;
                                }
                                else
                                        board_history[i][j][k] = EMPTY;

                        }
                }
        }

        //check if the spot is vacant
        if((board_history[current_board_index][x][y]!=X_LITERAL) && (board_history[current_board_index][x][y]!=O_LITERAL)) {
                //make the move

                if(player == 1) {
                        board_history[current_board_index][x][y] = X_LITERAL;
                }
                else {
                        board_history[current_board_index][x][y] = O_LITERAL;
                }
                //check to see if the player won
                if(check_winner(x,y) == 1) {
                        printf("make_move: player %i won!\n", player);
                        //end game code?
                }
                ++current_board_index; //increment board index
                pthread_cond_broadcast(&current_board_index_cv); //alert spectators
                return 1;
        }
        else {
                printf("make_move: player %i moved on a taken space.\n",player);
                return 0;
        }

}

/* returns 1 if the last move was a winning move */
int check_winner(int x, int y) {
        char** board = board_history[current_board_index];

        //check row for winner
        if((board[x][0] == board[x][1]) && (board[x][0] == board[x][2])) {
            printf("%s\n", "test 1");
                return 1;
        }
        //check col for winner
        if((board[0][y] == board[1][y]) && (board[0][y] == board[2][y])) {
            printf("%s\n", "test 2");

                return 1;
        }
        //check diagonal for winner
        if((board[1][1]=='X' || board[1][1]=='O') && (board[0][0] == board[1][1]) && (board[0][0] == board[2][2])) {
            printf("%s\n", "test 3");

                return 1;
        }
        //check diagonal for winner
        if((board[1][1]=='X' || board[1][1]=='O') && (board[0][2] == board[1][1]) && (board[0][2] == board[2][0])) {
            printf("%s\n", "test 4");

                return 1;
        }

        return 0;
}


/* Process player 1 */
void process_p1(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {

        rio_t rio; /* Rio buffer for calls to buffered rio_readlineb routine */
        char buf[MAXLINE]; /* General I/O buffer */
        Rio_readinitb(&rio, fd);
        int n;

        if ((n = Rio_readlineb_w(&rio, buf, sizeof(JOIN_KEY))) <= 0) {
                printf("process_spec: client issued a bad request (1).\n");
                Close(fd);
                return;
        }

        if (is_buf_join_key(buf) == FALSE) {
                // Incorrect join key
                printf("process_spec: (p1) incorrect join key: %s | size = %d", buf, sizeof(buf));
                Close(fd);
                return;
        }
        // Start processing player 1

        // when making a move, please call pthread_cond_broadcast(&current_board_index_cv) so that the spectators can receive the latest board

        while (1) {
                if ((n = Rio_readlineb_w(&rio, buf, MAXLINE)) <= 0) {
                        printf("process_p1: client issued a bad request (1).\n");
                        close(fd);
                        break;
                }

                //p1 only allowed to go when current_board_index is even
                if(current_board_index%2 != 1) {
                        printf("process_p1: p1 moved out of turn.\n");
                        continue; //skip to next p1 input
                }

                //check for correct format
                if(buf[1]!=' ') {
                        printf("process_p1: p1 sent invalid move format.\n");
                        continue; //skip to next p1 input
                }

                if(atoi(&buf[0])<0 || atoi(&buf[0])>2) {
                  printf("process_p1: p1 sent invalid x coordinate.\n");
                }

                if(atoi(&buf[2])<0 || atoi(&buf[2])>2) {
                  printf("process_p1: p1 sent invalid y coordinate.\n");
                }




                //try to make move
                make_move(1,atoi(&buf[0]),atoi(&buf[2]));


        }

}

/* Process player 2 */
void process_p2(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {
        rio_t rio; /* Rio buffer for calls to buffered rio_readlineb routine */
        char buf[MAXLINE]; /* General I/O buffer */
        int n;
        Rio_readinitb(&rio, fd);

        if ((n = Rio_readlineb_w(&rio, buf, sizeof(JOIN_KEY))) <= 0) {
                printf("process_spec: client issued a bad request (1).\n");
                Close(fd);
                return;
        }

        if (is_buf_join_key(buf) == FALSE) {
                // Incorrect join key
                printf("process_spec: (spec) incorrect join key: %s | size = %d", buf, sizeof(buf));
                Close(fd);
                return;
        }

        // Start processing player 2

        // when making a move, please call pthread_cond_broadcast(&current_board_index_cv) so that the spectators can receive the latest board

        while (1) {
                if ((n = Rio_readlineb_w(&rio, buf, MAXLINE)) <= 0) {
                        printf("process_p2: client issued a bad request (1).\n");
                        close(fd);
                        break;
                }

                //p2 only allowed to go when current_board_index is odd
                if(current_board_index%2 != 0) {
                        printf("process_p2: p2 moved out of turn.\n");
                        continue; //skip to next p2 input
                }

                //check for correct format
                if(buf[1]!=' ') {
                        printf("process_p2: p2 sent invalid move format.\n");
                        continue; //skip to next p2 input
                }
                if(atoi(&buf[0])<0 || atoi(&buf[0])>2) {
                  printf("process_p2: p2 sent invalid x coordinate.\n");
                }

                if(atoi(&buf[2])<0 || atoi(&buf[2])>2) {
                  printf("process_p2: p2 sent invalid y coordinate.\n");
                }

                //try to make move
                make_move(2,atoi(&buf[0]),atoi(&buf[2]));
        }

}

/* Process spectator */
void process_spec(int fd, struct sockaddr_in clientaddr, socklen_t clientlen) {

        rio_t rio; /* Rio buffer for calls to buffered rio_readlineb routine */
        char buf[MAXLINE]; /* General I/O buffer */
        int n;
        Rio_readinitb(&rio, fd);

        if ((n = Rio_readlineb_w(&rio, buf, MAXLINE)) <= 0) {
                printf("process_spec: client issued a bad request (1).\n");
                Close(fd);
                return;
        }


        // local board index
        int board_index = 0;

        while (1) {
                // wait until the current board index changes
                printf("current board index = %d\n", current_board_index);
                //Pthread_mutex_lock(&current_board_index_mtx);
                if (board_index >= current_board_index)
                        pthread_cond_wait(&current_board_index_cv, &current_board_index_mtx);

                // loop until the spectator is up to date
                while (board_history[board_index] != NULL) {
                        char** tic_tac_toe_board = construct_tic_tac_toe_board(board_index);

                        // BOARD_HEIGHT + 2 because a board has tic tac toe board looks like this
                        // O|X|O   row 0
                        // -----   row 1
                        // X| |    row 2
                        // -----   row 3 (BOARD_HEIGHT)
                        // O| |X   row 4
                        // So there are 5 rows total (5 = BOARD_HEIGHT + 2)
                        for (int i = 0; i < BOARD_HEIGHT + 2; i++) {
                                // may have to be strlen(tic_tac_toe_board[i]) + 1 because of '\0' terminator
                                Rio_writen_w(fd, tic_tac_toe_board[i], strlen(tic_tac_toe_board[i]));
                        }
                        // free tic_tac_toe_board
                        for (int i = 0; i < BOARD_HEIGHT+2; i++) {
                                printf("%s", tic_tac_toe_board[i]);
                                free(tic_tac_toe_board[i]);
                        }
                        printf("each row freed correctly\n");
                        free(tic_tac_toe_board);
                        printf("tic_tac_toe_board correctly freed\n");

                        Rio_writen_w(fd, "\n", 1);

                        // increment local board index
                        board_index++;

                        // if board index is now the last possible board, then might as well close now
                        if (board_index == MAX_BOARD_HISTORY - 1) {
                                // this is the last possible board, so close connection because game is over
                                Close(fd);
                                return;
                        }
                }
                //Pthread_mutex_unlock(&current_board_index_mtx);
        }

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

        return arg;
}

/*********************************************************************
 * The Rio package - robust I/O functions
 **********************************************************************/
/*
 * rio_readn - robustly read n bytes (unbuffered)
 */
/* $begin rio_readn */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
        size_t nleft = n;
        ssize_t nread;
        char *bufp = usrbuf;

        while (nleft > 0) {
                if ((nread = read(fd, bufp, nleft)) < 0) {
                        if (errno == EINTR) /* interrupted by sig handler return */
                                nread = 0; /* and call read() again */
                        else
                                return -1; /* errno set by read() */
                } else if (nread == 0)
                        break; /* EOF */
                nleft -= nread;
                bufp += nread;
        }
        return (n - nleft); /* return >= 0 */
}
/* $end rio_readn */

/*
 * rio_writen - robustly write n bytes (unbuffered)
 */
/* $begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
        size_t nleft = n;
        ssize_t nwritten;
        char *bufp = usrbuf;

        while (nleft > 0) {
                if ((nwritten = write(fd, bufp, nleft)) <= 0) {
                        if (errno == EINTR) /* interrupted by sig handler return */
                                nwritten = 0; /* and call write() again */
                        else
                                return -1; /* errorno set by write() */
                }
                nleft -= nwritten;
                bufp += nwritten;
        }
        return n;
}
/* $end rio_writen */

/*
 * rio_read - This is a wrapper for the Unix read() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    read() if the internal buffer is empty.
 */
/* $begin rio_read */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
        int cnt;

        while (rp->rio_cnt <= 0) { /* refill if buf is empty */
                rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
                if (rp->rio_cnt < 0) {
                        if (errno != EINTR) /* interrupted by sig handler return */
                                return -1;
                } else if (rp->rio_cnt == 0) /* EOF */
                        return 0;
                else
                        rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
        }

        /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
        cnt = n;
        if (rp->rio_cnt < n)
                cnt = rp->rio_cnt;
        memcpy(usrbuf, rp->rio_bufptr, cnt);
        rp->rio_bufptr += cnt;
        rp->rio_cnt -= cnt;
        return cnt;
}
/* $end rio_read */

/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
/* $begin rio_readinitb */
void rio_readinitb(rio_t *rp, int fd) {
        rp->rio_fd = fd;
        rp->rio_cnt = 0;
        rp->rio_bufptr = rp->rio_buf;
}
/* $end rio_readinitb */

/*
 * rio_readnb - Robustly read n bytes (buffered)
 */
/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
        size_t nleft = n;
        ssize_t nread;
        char *bufp = usrbuf;

        while (nleft > 0) {
                if ((nread = rio_read(rp, bufp, nleft)) < 0) {
                        if (errno == EINTR) /* interrupted by sig handler return */
                                nread = 0; /* call read() again */
                        else
                                return -1; /* errno set by read() */
                } else if (nread == 0)
                        break; /* EOF */
                nleft -= nread;
                bufp += nread;
        }
        return (n - nleft); /* return >= 0 */
}
/* $end rio_readnb */

/*
 * rio_readlineb - robustly read a text line (buffered)
 */
/* $begin rio_readlineb */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
        int n, rc;
        char c, *bufp = usrbuf;

        for (n = 1; n < maxlen; n++) {
                if ((rc = rio_read(rp, &c, 1)) == 1) {
                        *bufp++ = c;
                        if (c == '\n')
                                break;
                } else if (rc == 0) {
                        if (n == 1)
                                return 0; /* EOF, no data read */
                        else
                                break; /* EOF, some data was read */
                } else
                        return -1; /* error */
        }
        *bufp = 0;
        return n;
}
/* $end rio_readlineb */

/**********************************
* Wrappers for robust I/O routines
**********************************/
ssize_t Rio_readn(int fd, void *ptr, size_t nbytes) {
        ssize_t n;

        if ((n = rio_readn(fd, ptr, nbytes)) < 0)
                unix_error("Rio_readn error");
        return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n) {
        if (rio_writen(fd, usrbuf, n) != n)
                unix_error("Rio_writen error");
}

void Rio_readinitb(rio_t *rp, int fd) {
        rio_readinitb(rp, fd);
}

ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
        ssize_t rc;

        if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
                unix_error("Rio_readnb error");
        return rc;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
        ssize_t rc;

        if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
                unix_error("Rio_readlineb error");
        return rc;
}

/*
 * Rio_readn_w - A wrapper function for rio_readn (csapp.c) that
 * prints a warning message when a read fails instead of terminating
 * the process.
 */
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes) {
        ssize_t n;

        if ((n = rio_readn(fd, ptr, nbytes)) < 0) {
                printf("Warning: rio_readn failed\n");
                return 0;
        }
        return n;
}

/*
 * Rio_readlineb_w - A wrapper for rio_readlineb (csapp.c) that
 * prints a warning when a read fails instead of terminating
 * the process.
 */
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen) {
        ssize_t rc;

        if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
                printf("Warning: rio_readlineb failed\n");
                return 0;
        }
        return rc;
}

/*
 * Rio_writen_w - A wrapper function for rio_writen (csapp.c) that
 * prints a warning when a write fails, instead of terminating the
 * process.
 */
void Rio_writen_w(int fd, void *usrbuf, size_t n) {
        if (rio_writen(fd, usrbuf, n) != n) {
                printf("Warning: rio_writen failed.\n");
        }
}

/* Takes a board index and constructs a printable tic tac toe board from board_history */
char** construct_tic_tac_toe_board(int board_index) {
        if (board_history[board_index] == NULL) {
                return NULL;
        }

        char** board_to_be_printed = (char **) calloc(BOARD_HEIGHT+2, (BOARD_HEIGHT+2) * sizeof(char*));

        for (int i = 0; i < BOARD_HEIGHT+2; i++) {
                char* board_row = (char *) calloc(BOARD_WIDTH+3, (BOARD_WIDTH+3) * sizeof(char));
                if (i % 2 == 1) {
                        for (int j = 0; j < BOARD_WIDTH+2; j++) {
                                board_row[j] = '-';
                        }
                }
                else {
                        for (int j = 0; j < BOARD_WIDTH+2; j++) {
                                if (j % 2 == 1) {
                                        board_row[j] = '|';
                                }
                                else {
                                        int actual_row = i/2;
                                        int actual_col = j/2;
                                        if (board_history[board_index][actual_row][actual_col] == X) {
                                                board_row[j] = 'X';
                                        }
                                        else if (board_history[board_index][actual_row][actual_col] == O) {
                                                board_row[j] = 'O';
                                        }
                                        else {
                                                board_row[j] = ' ';
                                        }
                                }
                        }
                }

                board_row[BOARD_WIDTH+2] = '\n';
                board_to_be_printed[i] = board_row;

                printf("row: %s", board_to_be_printed[i]);
        }

        return board_to_be_printed;
}

void Close(int fd) {
        int rc;

        if ((rc = close(fd)) < 0)
                unix_error("Close error");
}

int is_buf_join_key(char *buf) {
        if (buf[0] == JOIN_KEY[0] && buf[1] == JOIN_KEY[1] && buf[2] == JOIN_KEY[2] && buf[3] == JOIN_KEY[3])
                return TRUE;

        return FALSE;
}
