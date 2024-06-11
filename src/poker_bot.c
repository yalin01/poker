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
#include <time.h>
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
    int rounds;
    int pos;
    int valid_input_int;
    char c1_suit,c2_suit,fl1_suit,fl2_suit,fl3_suit,t_suit,r_suit;
    int c1_value,c2_value,fl1_value,fl2_value,fl3_value,t_value,r_value;
    char name[50];
    int chips,bet_chips,bet_chips_min,pot;
    card myCards[7], bestHand[5];   // initialize a local hand
    int player_num, initial_bet, player_pos;
    int score;
    int posAttempts = 1;
    int difficulty;

    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s <hostname> <port> <difficulty>\n", argv[0]);
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
    difficulty = atoi(argv[3]);
    if(difficulty == 1)
        printf("Difficulty set to easy.\n");
    else if(difficulty == 2)
        printf("Difficulty set to hard.\n");
    else{
        fprintf(stderr,"Invalid difficulty\n\n");
        exit(0);
    }

    TCP_client_create(sockfd,portno,serv_addr,server);
	printf("The bot has connected to server, waiting for other players\n");

    int exit_status = 0;
    while(exit_status==0){
        TCP_read_noprint(sockfd,buffer);
        if(strcmp(buffer,"game over.\n")==0){
            exit_status=1;
            break;
        }
        else{
            if(strchr(buffer,';')){
                sscanf(buffer,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%[^;];%d;%d;%d;%d",
                &rounds,&pos,&c1_suit,&c1_value,&c2_suit,&c2_value,&fl1_suit,&fl1_value,&fl2_suit,&fl2_value,&fl3_suit,&fl3_value,&t_suit,&t_value,&r_suit,&r_value,&valid_input_int,name,&chips,&bet_chips,&bet_chips_min,&pot);

                // assign variables to the hand for evaluation
                myCards[0].suit = c1_suit; myCards[0].value = c1_value; 
                myCards[1].suit = c2_suit; myCards[1].value = c2_value; 
                myCards[2].suit = fl1_suit; myCards[2].value = fl1_value; 
                myCards[3].suit = fl2_suit; myCards[3].value = fl2_value; 
                myCards[4].suit = fl3_suit; myCards[4].value = fl3_value; 
                myCards[5].suit = t_suit;  myCards[5].value = t_value; 
                myCards[6].suit = r_suit; myCards[6].value = r_value;

                if(valid_input_int == 1){
                    printf("Round: %d, Position:%d\nBot cards:%c%d, %c%d\nCommunity cards:%c%d, %c%d, %c%d, %c%d, %c%d\nWallet: %d chips, Already bet: %d chips, Last bet: %d, Pot: %d\n\n"
                ,rounds,pos, myCards[0].suit,myCards[0].value,myCards[1].suit,myCards[1].value,myCards[2].suit,myCards[2].value,myCards[3].suit,myCards[3].value,myCards[4].suit,myCards[4].value,myCards[5].suit,myCards[5].value,myCards[6].suit,myCards[6].value,chips,bet_chips,bet_chips_min,pot);
                    findBestHand(myCards, bestHand);
                    score = evaluateHand(bestHand);
                    printf("Current score: %d\n", score);
                    char handType[20];
                    switch(score){
                        case 1: strcpy(handType, "high card");      break; 
                        case 2: strcpy(handType, "pair");           break;
                        case 3: strcpy(handType, "2 pairs");        break;
                        case 4: strcpy(handType, "3 of a kind");    break;
                        case 5: strcpy(handType, "straight");       break;
                        case 6: strcpy(handType, "flush");          break;
                        case 7: strcpy(handType, "full house");     break;
                        case 8: strcpy(handType, "4 of a kind");    break;
                        case 9: strcpy(handType, "straight flush"); break;
                        case 10: strcpy(handType, "royal flush");   break;
                    }
                    printf("Current hand: %s\n", handType);
                }
                else{
                    printf("\n\nInvalid input, try again!\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n"
                ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value,name,chips,bet_chips,bet_chips_min,pot);
                }
                // bot logic
                if(difficulty == 1){        // easy
                    if(pos != 1 && bet_chips_min == 0) TCP_write_wstring(sockfd, buffer, "CHECK");     // check if previous players all checked
                    else if(rounds >= 2 && score < 2) TCP_write_wstring(sockfd, buffer, "FOLD");   // fold if hand value too low
                    else TCP_write_wstring(sockfd, buffer, "BET");  // bet or call if no conditions were met
                }
                else if(difficulty == 2){   // hard
                    if(pos == 1 && score < 2 && rounds > 1) TCP_write_wstring(sockfd, buffer, "CHECK");   // check if being SB with a low value hand
                    else if(pos != 1 && bet_chips_min == 0) TCP_write_wstring(sockfd, buffer, "CHECK");     // check if previous players all checked
                    else if(rounds >= 4 && score < 2) TCP_write_wstring(sockfd, buffer, "FOLD");     // fold if high cards only during the fourth round 
                    else TCP_write_wstring(sockfd, buffer, "BET");  // bet or call if no conditions were met
                }
            }
            else if(strcmp(buffer,"How much do you want to bet?")==0){
                char amount2bet[5];
                int bet;
                if(difficulty == 1){
                    if(bet_chips_min > bet) bet = bet_chips_min;
                    if(bet_chips_min == 0) bet = initial_bet;
                }
                else if(difficulty == 2){
                    if(bet_chips_min > bet) bet = bet_chips_min;
                    if(bet_chips_min == 0) bet = initial_bet;
                    if(score >= 3)  bet *= 2;
                    if(score >= 4)  bet *= 3;
                    if(score >= 7)  bet *= 5;
                    if(score == 10) bet = chips;    // ALL IN!!
                }
                sprintf(amount2bet, "%d", bet);
                TCP_write_wstring(sockfd, buffer, amount2bet);
            }
            else if(strchr(buffer,'*')){
                sscanf(buffer, "%d*%d*%d", &player_num, &initial_bet, &player_pos);
                TCP_write_wstring(sockfd,buffer,"bot received.");
            }
            else if(strstr(buffer,"UPDATE_COMPLETE:")){
                TCP_write_wstring(sockfd,buffer,"bot received");
            }
            else if(strstr(buffer,"What is your name?")){   // ramdom name generator based on Exominds from Destiny 2
                char* EXO_names[] = {"Cayde", "Saint", "Banshee", "Eriana", "Ada", "Atraks"};
                srand(time(NULL));
                char name[20];
                int reboots = (rand() % 44) + 1;
                sprintf(name, "%s-%d", EXO_names[rand() % 6], reboots);
                TCP_write_wstring(sockfd, buffer, name);
            }
            else if(strstr(buffer,"PLAYER_RESULT:")){
                int p_pos;
                char p_c1_suit;
                int p_c1_value;
                char p_c2_suit;
                int p_c2_value;
                char p_name[50];
                sscanf(buffer,"PLAYER_RESULT: %d,%[^,],%c,%d,%c,%d",&p_pos,p_name,&p_c1_suit,&p_c1_value,&p_c2_suit,&p_c2_value);
                printf("result: player %s at pos %d have cards: %c%d, %c%d\n",p_name,p_pos,p_c1_suit,p_c1_value,p_c2_suit,p_c2_value);
                TCP_write_wstring(sockfd,buffer,"player read successfully");
        }
            else if(strstr(buffer,"What position do you want?")){
                char tryPosition[256];
                if(strchr(buffer, '!')) posAttempts++;      // keeps trying with different positions until valid
                sprintf(tryPosition, "%d", posAttempts);
                TCP_write_wstring(sockfd,buffer,tryPosition);

            }
            else if(strstr(buffer,"QUIT or REMATCH?")){
                if(chips >= 0) TCP_write_wstring(sockfd,buffer,"REMATCH");
                else TCP_write_wstring(sockfd,buffer,"QUIT");   // auto quit if insufficient balance
            }
            else{
                printf("%s\n",buffer);
		        TCP_write_wstring(sockfd,buffer,"bot received.");
            }
        }
    }

    TCP_write_wstring(sockfd,buffer,"bot received");
	TCP_read(sockfd,buffer);
    TCP_write_wstring(sockfd,buffer,"bot received");

    close(sockfd);
    return 0;
}
