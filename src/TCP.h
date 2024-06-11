#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "struct.h"
#include "games.h" 

#ifndef TCP_h
void error(const char *msg);
bool TCP_read(int sockfd,char buffer[256]);
bool TCP_write(int sockfd,char buffer[256]);
bool TCP_write_wstring(int sockfd,char buffer[256],char input[256]);
bool TCP_client_create(int sockfd,int portno, struct sockaddr_in serv_addr, struct hostent *server);
bool TCP_write_wstring_gmsg(int sockfd,char buffer[256],table* gameTable,player* p,bool valid_input, int rounds);
bool TCP_write_gmsg(int sockfd,char buffer[256],table* gameTable,player* p, int rounds,bool valid_input);
bool TCP_read_gmsg(int sockfd,char buffer[256]);
bool TCP_read_noprint(int sockfd,char buffer[256]);
bool UpdateAllPlayer(player* p,char buffer[256],int player_action,int player_amount,int player_num);
int compareString(const char *input);
bool UpdateAllPlayer_status(table* gameTable,player* p, char buffer[256],int player_num);
bool UpdateAllPlayer_self(table* gameTable,player* p, char buffer[256]);
bool UpdateAllPlayer_complete(table* gameTable,player* p, char buffer[256],int player_num);
void TCP_write_BJmsg(int sockfd, char buffer[256], int playersTurn, card* hand, int valid_input);
#endif
