#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "TCP.h"

#include "struct.h"
#include "games.h"
#include "rules.h"
#include <ctype.h>

#define DECK_SIZE 52


int main(int argc, char *argv[])
{
    int player_num = 1;
//    int initial_bet = 4; 
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    printf("Testing server started\n");

    int player_addr;
    listen(sockfd,player_num);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
          error("ERROR on accept");
    
    player_addr=newsockfd;
    printf("Player %d connected.\n",1);
        
    TCP_write_wstring(player_addr,buffer,"What is your name?");
    TCP_read(player_addr,buffer);

    int length = strlen(buffer);
    char* player_name = (char*)malloc((length+1)*sizeof(char));
    strcpy(player_name,buffer);

    char write_message1[256];
    sprintf(write_message1,"Hello %s",player_name);
    TCP_write_wstring(player_addr,buffer,write_message1);

    TCP_read(player_addr,buffer);
    if(strcmp(buffer,"CHECK")==0){
        TCP_write_wstring(player_addr,buffer,"You have checked");
    }
    else{
        exit(1);
    }

    TCP_read(player_addr,buffer);
    if(strcmp(buffer,"BET")==0){
        TCP_write_wstring(player_addr,buffer,"How much would you like to bet?");
        TCP_read(player_addr,buffer);
        int amount = atoi(buffer);
        sprintf(write_message1,"You have bet %d",amount);
        TCP_write_wstring(player_addr,buffer,write_message1);
    }
    else{
        exit(1);
    }

    TCP_read(player_addr,buffer);
    if(strcmp(buffer,"FOLD")==0){
        TCP_write_wstring(player_addr,buffer,"You have folded");
    }
    else{
        exit(1);
    }

    TCP_read(player_addr,buffer);
    if(strcmp(buffer,"EXIT")==0){
        TCP_write_wstring(player_addr,buffer,"Bye bye");
    }
    else{
        exit(1);
    }

    

    //game end here
    close(newsockfd);
    close(sockfd);
    

     return 0; 
}