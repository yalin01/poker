# Variable
CC = gcc
DEBUG = -g
CFLAGS = -Wall -std=c11 $(DEBUG) -c
LFLAGS = -Wall -std=c11 $(DEBUG)
DEBUG_FLAG = -DDEBUG

GTKINC  = `PKG_CONFIG_PATH=/usr/share/pkgconfig pkg-config --cflags gtk+-2.0 gdk-pixbuf-2.0`
GTKLIBS = `PKG_CONFIG_PATH=/usr/share/pkgconfig pkg-config --libs gtk+-2.0 gdk-pixbuf-2.0`

INC	= $(GTKINC)
LIBS	= $(GTKLIBS)

#default target
all: poker_client poker_server poker_client_term poker_bot poker_comms poker_comms_server poker_server_test poker_client_term_test poker_client_test

# testing
test: poker_server_test poker_client_term_test poker_client_test poker_comms poker_comms_server
	./bin/poker_server_test
	./bin/poker_client_term_test
	./bin/poker_client_test

only_test: poker_server_test poker_client_term_test poker_client_test poker_comms poker_comms_server

test_server: only_test
	./bin/poker_server_test

test_client: only_test
	./bin/poker_client_term_test

test_gui: only_test
	./bin/poker_client_test

test_comms: only_test
	./bin/poker_comms laguna 21500

test_comms_server: only_test
	./bin/poker_comms_server 21500

clean:
	rm -f bin/poker_client_term
	rm -f bin/poker_server
	rm -f bin/poker_client
	rm -f bin/poker_comms
	rm -f bin/poker_comms_server
	rm -f bin/poker_bot
	rm -f bin/poker_server_test
	rm -f bin/poker_client_term_test
	rm -f bin/poker_client_test
	rm -f src/*.o

#server and client target
TCP.o: src/TCP.c src/TCP.h
	$(CC) $(CFLAGS) src/TCP.c -o src/TCP.o

#PokerStruct target
rules.o: src/rules.c src/rules.h src/struct.h
	$(CC) $(CFLAGS) src/rules.c -o src/rules.o

games.o: src/games.c src/games.h src/struct.h
	$(CC) $(CFLAGS) src/games.c -o src/games.o

poker_client_term.o: src/poker_client_term.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) src/poker_client_term.c -o src/poker_client_term.o

poker_client_term_test.o: src/poker_client_term.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) $(DEBUG_FLAG) src/poker_client_term.c -o src/poker_client_term_test.o

poker_bot.o: src/poker_bot.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) src/poker_bot.c -o src/poker_bot.o

poker_server.o: src/poker_server.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) src/poker_server.c -o src/poker_server.o

poker_server_test.o: src/poker_server.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) $(DEBUG_FLAG) src/poker_server.c -o src/poker_server_test.o

poker_client.o: src/poker_client.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) $(INC) src/poker_client.c -o src/poker_client.o

poker_client_test.o: src/poker_client.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) $(INC) $(DEBUG_FLAG) src/poker_client.c -o src/poker_client_test.o

poker_comms.o: src/poker_comms.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) src/poker_comms.c -o src/poker_comms.o

poker_comms_server.o: src/poker_comms_server.c src/TCP.h src/struct.h src/rules.h src/games.h
	$(CC) $(CFLAGS) src/poker_comms_server.c -o src/poker_comms_server.o

poker_client: poker_client.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS) $(LIBS) src/poker_client.o src/TCP.o src/rules.o src/games.o -o bin/poker_client

poker_client_test: poker_client_test.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS) $(LIBS) $(DEBUG_FLAG) src/poker_client_test.o src/TCP.o src/rules.o src/games.o -o bin/poker_client_test

poker_client_term: src/poker_client_term.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS)  src/poker_client_term.o src/TCP.o src/rules.o src/games.o -o bin/poker_client_term

poker_client_term_test: poker_client_term_test.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS) $(DEBUG_FLAG) src/poker_client_term_test.o src/TCP.o src/rules.o src/games.o -o bin/poker_client_term_test

poker_bot: src/poker_bot.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS)  src/poker_bot.o src/TCP.o src/rules.o src/games.o -o bin/poker_bot

poker_server: src/poker_server.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS)  src/poker_server.o src/TCP.o src/rules.o src/games.o -o bin/poker_server

poker_server_test: poker_server_test.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS) $(DEBUG_FLAG) src/poker_server_test.o src/TCP.o src/rules.o src/games.o -o bin/poker_server_test

poker_comms: src/poker_comms.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS)  src/poker_comms.o src/TCP.o src/rules.o src/games.o -o bin/poker_comms

poker_comms_server: src/poker_comms_server.o src/TCP.o src/rules.o src/games.o
	$(CC) $(LFLAGS)  src/poker_comms_server.o src/TCP.o src/rules.o src/games.o -o bin/poker_comms_server
