#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "TCP.h" 

#include "struct.h"
#include "games.h"
#include "rules.h"
#include <ctype.h>



int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR no host found, please start the server before client\n");
        exit(0);
    }

    TCP_client_create(sockfd,portno,serv_addr,server);
	printf("You have connected to the server\n");

	TCP_read(sockfd, buffer);   // what's your name
    TCP_write_wstring(sockfd, buffer, "Aiden");
    TCP_read(sockfd, buffer);   // Hello, <name>
    TCP_write_wstring(sockfd, buffer, "CHECK");
    TCP_read(sockfd, buffer);   // You have checked
    TCP_write_wstring(sockfd, buffer, "BET");
    TCP_read(sockfd, buffer);   // How much
    TCP_write_wstring(sockfd, buffer, "6");
    TCP_read(sockfd, buffer);   // You have bet
    TCP_write_wstring(sockfd, buffer, "FOLD");
    TCP_read(sockfd, buffer);   // You have folded
    TCP_write_wstring(sockfd, buffer, "EXIT");
    TCP_read(sockfd, buffer);
    printf("Testing complete\n");
/*    
    int exit_status = 0;
    while(exit_status==0){
        TCP_read_noprint(sockfd,buffer);
        if(strcmp(buffer,"game over.\n")==0){
            exit_status=1;
            break;
        }
        else{
            if(strchr(buffer,';')){
                int rounds;
                int pos;
                int valid_input_int;
                char c1_suit,c2_suit,fl1_suit,fl2_suit,fl3_suit,t_suit,r_suit;
                int c1_value,c2_value,fl1_value,fl2_value,fl3_value,t_value,r_value;
                char name[50];
		//char *name;
                int chips,bet_chips,bet_chips_min,pot;
                sscanf(buffer,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%[^;];%d;%d;%d;%d",
                &rounds,&pos,&c1_suit,&c1_value,&c2_suit,&c2_value,&fl1_suit,&fl1_value,&fl2_suit,&fl2_value,&fl3_suit,&fl3_value,&t_suit,&t_value,&r_suit,&r_value,&valid_input_int,name,&chips,&bet_chips,&bet_chips_min,&pot);

                if(valid_input_int == 1){
                    printf("\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                else{
                    printf("\n\nInvalid input, try again!\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                
//                printf("\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\n\n"
//                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value);
                
                TCP_write(sockfd,buffer);
            }
            else if(strcmp(buffer,"How much do you want to bet?")==0){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
            }
            else{
                printf("%s\n",buffer);
            }
        }
    }

	TCP_read(sockfd,buffer);
*/
    close(sockfd);
    return 0;
}
