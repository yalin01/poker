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
    #ifndef DEBUG
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
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    TCP_client_create(sockfd,portno,serv_addr,server);
	printf("You have connected to the server, waiting for other players\n");

/*
    TCP_read(sockfd,buffer);
    TCP_write_wstring(sockfd,buffer,"player received");
    TCP_read(sockfd,buffer);
	printf("before read.\n");
    TCP_write(sockfd,buffer);
	printf("after read.\n");
	TCP_read(sockfd,buffer);
    TCP_write_wstring(sockfd,buffer,"player received");
	TCP_read(sockfd,buffer);
    TCP_write_wstring(sockfd,buffer,"player received");
*/

    int exit_status = 0;
    int gameMode;
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
                int chips,bet_chips,bet_chips_min,pot;
                sscanf(buffer,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%[^;];%d;%d;%d;%d",
                &rounds,&pos,&c1_suit,&c1_value,&c2_suit,&c2_value,&fl1_suit,&fl1_value,&fl2_suit,&fl2_value,&fl3_suit,&fl3_value,&t_suit,&t_value,&r_suit,&r_value,&valid_input_int,name,&chips,&bet_chips,&bet_chips_min,&pot);

                if(valid_input_int == 1){
                    printf("\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                else{
                    printf("\n\nInvalid input, try again!\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                TCP_write(sockfd,buffer);
            }
            else if(strstr(buffer,"How much do you want to bet?")){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
            }
		else if(strstr(buffer,"UPDATE_COMPLETE:")){
                TCP_write_wstring(sockfd,buffer,"player received");
                }
        else if(strstr(buffer,"What is your name?")){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
        }
        else if(strstr(buffer,"PLAYER_RESULT:")){
                int p_pos;
                char p_c1_suit;
                int p_c1_value;
                char p_c2_suit;
                int p_c2_value;
                char p_name[50];
                int p_chips;
                sscanf(buffer,"PLAYER_RESULT: %d,%[^,],%c,%d,%c,%d,%d",&p_pos,p_name,&p_c1_suit,&p_c1_value,&p_c2_suit,&p_c2_value,&p_chips);
                printf("result: player %s at pos %d have cards: %c%d, %c%d\n",p_name,p_pos,p_c1_suit,p_c1_value,p_c2_suit,p_c2_value);
                TCP_write_wstring(sockfd,buffer,"player read successfully");
        }
        else if(strstr(buffer,"What position do you want?")){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
        }
        else if(strstr(buffer,"QUIT or REMATCH?")){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
        }
        else if(strstr(buffer,"Continue? (Y or N)")){
                printf("%s\n",buffer);
                TCP_write(sockfd,buffer);
        }
        else if(strstr(buffer,"GAMEMODE:")){
            sscanf(buffer,"GAMEMODE: %d", &gameMode);
            printf("Game mode is %d\n", gameMode);
            TCP_write_wstring(sockfd,buffer,"player received.");
        }
            else if(strchr(buffer, '#')){
                int handIndex = 0;
                int handLength, handValue;
                int playersTurn, valid_input;
                char handInfo[50], addCard[10];
                card hand[10];
                //card dealerHand[10], playerHand[10];

                sscanf(buffer, "%d#%d#%d#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d", &playersTurn, &handLength, &handValue, &valid_input,
                &hand[0].suit, &hand[0].value, &hand[1].suit, &hand[1].value, &hand[2].suit, &hand[2].value, &hand[3].suit, &hand[3].value, &hand[4].suit, &hand[4].value,
                &hand[5].suit, &hand[5].value, &hand[6].suit, &hand[6].value, &hand[7].suit, &hand[7].value, &hand[8].suit, &hand[8].value, &hand[9].suit, &hand[9].value);

                sprintf(handInfo, "%c%d, %c%d", hand[0].suit, hand[0].value, hand[1].suit, hand[1].value);
                handIndex += 2;
                while(handIndex < handLength){
                    sprintf(addCard, ", %c%d", hand[handIndex].suit, hand[handIndex].value);
                    strcat(handInfo, addCard);
                    handIndex++;
                }

                if(playersTurn == 0){   // players's turn, dealer hides one card
                    printf("The Dealer's cards are: %s.\n", handInfo);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 1){  // player's turn, player shows all cards and makes decision
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    if(valid_input != 1) printf("Invalid input! Please make a decision (HIT or STAND): ");
                    else printf("Please make a decision (HIT or STAND): ");
                    TCP_write(sockfd, buffer);
                }
                else if(playersTurn == 2){  // dealer's turn, dealer shows all cards
                    printf("The Dealer's cards are: %s. Their hand value is %d.\n", handInfo, handValue);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 3){  // Blackjack
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    printf("Blackjack!\n");
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == -1){ // player's turn, player busted
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
            }
            else{
                printf("%s\n",buffer);
		TCP_write_wstring(sockfd,buffer,"player received.");
            }
        }
    }

    TCP_write_wstring(sockfd,buffer,"player received");
	TCP_read(sockfd,buffer);
    TCP_write_wstring(sockfd,buffer,"player received");

    close(sockfd);
    return 0;
    #endif
    #ifdef DEBUG
    printf("This is client term debug test.\n");
    char buffer_test[256];
    sprintf(buffer_test,"1;1;H;1;H;2;S;1;S;2;D;1;D;2;C;1;1;ALEX;996;4;4;4");
    if(strchr(buffer_test,';')){
                int rounds;
                int pos;
                int valid_input_int;
                char c1_suit,c2_suit,fl1_suit,fl2_suit,fl3_suit,t_suit,r_suit;
                int c1_value,c2_value,fl1_value,fl2_value,fl3_value,t_value,r_value;
                char name[50];
                int chips,bet_chips,bet_chips_min,pot;
                sscanf(buffer_test,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%[^;];%d;%d;%d;%d",
                &rounds,&pos,&c1_suit,&c1_value,&c2_suit,&c2_value,&fl1_suit,&fl1_value,&fl2_suit,&fl2_value,&fl3_suit,&fl3_value,&t_suit,&t_value,&r_suit,&r_value,&valid_input_int,name,&chips,&bet_chips,&bet_chips_min,&pot);

                if(valid_input_int == 1){
                    printf("\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                else{
                    printf("\n\nInvalid input, try again!\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                //TCP_write(sockfd,buffer);
    }
    sprintf(buffer_test,"1#2#15#1#H#7#H#8#X#0#X#0#X#0#X#0#X#0#X#0#X#0#X#0");
    if(strchr(buffer_test, '#')){
                int handIndex = 0;
                int handLength, handValue;
                int playersTurn, valid_input;
                char handInfo[50], addCard[10];
                card hand[10];
                //card dealerHand[10], playerHand[10];

                sscanf(buffer_test, "%d#%d#%d#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d", &playersTurn, &handLength, &handValue, &valid_input,
                &hand[0].suit, &hand[0].value, &hand[1].suit, &hand[1].value, &hand[2].suit, &hand[2].value, &hand[3].suit, &hand[3].value, &hand[4].suit, &hand[4].value,
                &hand[5].suit, &hand[5].value, &hand[6].suit, &hand[6].value, &hand[7].suit, &hand[7].value, &hand[8].suit, &hand[8].value, &hand[9].suit, &hand[9].value);

                sprintf(handInfo, "%c%d, %c%d", hand[0].suit, hand[0].value, hand[1].suit, hand[1].value);
                handIndex += 2;
                while(handIndex < handLength){
                    sprintf(addCard, ", %c%d", hand[handIndex].suit, hand[handIndex].value);
                    strcat(handInfo, addCard);
                    handIndex++;
                }

                if(playersTurn == 0){   // players's turn, dealer hides one card
                    printf("The Dealer's cards are: %s.\n", handInfo);
                    //TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 1){  // player's turn, player shows all cards and makes decision
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    if(valid_input != 1) printf("Invalid input! Please make a decision (HIT or STAND): ");
                    else printf("Please make a decision (HIT or STAND): ");
                    //TCP_write(sockfd, buffer);
                }
                else if(playersTurn == 2){  // dealer's turn, dealer shows all cards
                    printf("The Dealer's cards are: %s. Their hand value is %d.\n", handInfo, handValue);
                    //TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 3){  // Blackjack
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    printf("Blackjack!\n");
                    //TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == -1){ // player's turn, player busted
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    //TCP_write_wstring(sockfd,buffer,"player received.");
                }
            }
    
    return 0;
    #endif
}
