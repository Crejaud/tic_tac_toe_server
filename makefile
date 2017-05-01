CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread

OBJS = ttt_server.o ttt_logic.o

all: ttt_server

ttt_server: $(OBJS)

ttt_logic.o: ttt_logic.c
	$(CC) $(CFLAGS) -c ttt_logic.c

ttt_server.o: ttt_server.c
	$(CC) $(CFLAGS) -c ttt_server.c
clean:
	rm -f *~ *.o ttt_server
