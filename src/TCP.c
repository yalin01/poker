#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "TCP.h"
#include "struct.h"
#include "games.h"


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

bool TCP_read(int sockfd,char buffer[256]){
    //printf("start reading to the target.\n");
    bzero(buffer,256);
    int n = read(sockfd,buffer,255);
    if (n < 0){ 
         error("ERROR reading from socket\n\n");
         return false;
    }
    else{
        printf("%s\n",buffer);
        //printf("Message from target: %s\n",buffer);
	    //printf("reading succeed.\n\n");
        return true;
    }
}

bool TCP_write(int sockfd,char buffer[256]){
    //printf("start writing to the target.\n");
    bzero(buffer,256);
    //printf("What is the message you want to send?: ");
    fgets(buffer,255,stdin);
	buffer[strcspn(buffer,"\n")]='\0';
    int n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){ 
         error("ERROR writing to target\n\n");
         return false;
    }
    else{
        //printf("writing succeed.\n\n");
        return true;
    }
}

bool TCP_write_wstring(int sockfd,char buffer[256],char input[256]){
    //printf("start writing to the target.\n");
    bzero(buffer,256);
    //printf("What is the message you want to send?: ");
    //fgets(buffer,255,stdin);
    strcpy(buffer,input);
    int n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){ 
         error("ERROR writing to target\n\n");
         return false;
    }
    else{
        //printf("writing succeed.\n\n");
        return true;
    }
}

bool TCP_client_create(int sockfd,int portno, struct sockaddr_in serv_addr, struct hostent *server){
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
        error("ERROR connecting");
        return false;
    }
    else{
        return true;
    }
}


bool TCP_write_wstring_gmsg(int sockfd,char buffer[256],table* gameTable,player* p,bool valid_input, int rounds){
            char write_message1[256];
            if(valid_input == true){
                if(rounds == 1){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\n What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value
                    );
                }
                else if(rounds==2){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d.\n What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value
                    );
                }
                else if(rounds ==3){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d,%c%d.\n What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value,
                    gameTable->boardCards.turn.suit, gameTable->boardCards.turn.value
                    );
                }
                else{
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d,%c%d,%c%d.\n What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value,
                    gameTable->boardCards.turn.suit, gameTable->boardCards.turn.value,
                    gameTable->boardCards.river.suit, gameTable->boardCards.river.value
                    );
                }
            
            }

            else{
                if(rounds == 1){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\n Try again! What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value
                    );
                }
                else if(rounds==2){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d.\n Try again! What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value
                    );
                }
                else if(rounds ==3){
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d,%c%d.\n Try again! What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value,
                    gameTable->boardCards.turn.suit, gameTable->boardCards.turn.value
                    );
                }
                else{
                    sprintf(write_message1,"\n\nround %d.\n Player %s (%s):\n You have %d chips and bet %d (min bet are %d).\n Your Cards are %c%d,%c%d.\nPublic cards: %c%d,%c%d,%c%d,%c%d,%c%d.\n Try again! What is your decision?\n\n"
                    ,rounds,p->name, getPositionName(p->pos), 
                    p->chips,p->bet_chips, gameTable->bet_chips_min, 
                    p->c1.suit, p->c1.value, p->c2.suit, p->c2.value,
                    gameTable->boardCards.flop1.suit, gameTable->boardCards.flop1.value,
                    gameTable->boardCards.flop2.suit, gameTable->boardCards.flop2.value,
                    gameTable->boardCards.flop3.suit, gameTable->boardCards.flop3.value,
                    gameTable->boardCards.turn.suit, gameTable->boardCards.turn.value,
                    gameTable->boardCards.river.suit, gameTable->boardCards.river.value
                    );
                }
            }
            bool n = TCP_write_wstring(p->addr,buffer,write_message1);
		return n;
}


bool TCP_write_gmsg(int sockfd,char buffer[256],table* gameTable,player* p, int rounds,bool valid_input){
            char write_message1[256];
            	char fl1_suit = 'X';
		char fl2_suit = 'X';
		char fl3_suit = 'X';
		char t_suit = 'X';
		char r_suit='X';
            	int fl1_value = 0;
		int fl2_value = 0;
		int fl3_value = 0;
		int t_value = 0;
		int r_value=0;
        int valid_input_int;
        if(valid_input == true){
            valid_input_int = 1;
        }
        else{
            valid_input_int = 0;
        }
            if(rounds == 2){
                fl1_suit = gameTable->boardCards.flop1.suit;
                fl2_suit = gameTable->boardCards.flop2.suit;
                fl3_suit = gameTable->boardCards.flop3.suit;
                fl1_value = gameTable->boardCards.flop1.value;
                fl2_value = gameTable->boardCards.flop2.value;
                fl3_value = gameTable->boardCards.flop3.value;
            }
            else if(rounds == 3){
                fl1_suit = gameTable->boardCards.flop1.suit;
                fl2_suit = gameTable->boardCards.flop2.suit;
                fl3_suit = gameTable->boardCards.flop3.suit;
                t_suit = gameTable->boardCards.turn.suit;
                fl1_value = gameTable->boardCards.flop1.value;
                fl2_value = gameTable->boardCards.flop2.value;
                fl3_value = gameTable->boardCards.flop3.value;
                t_value = gameTable->boardCards.turn.value;
            }
            else if (rounds ==4){
                fl1_suit = gameTable->boardCards.flop1.suit;
                fl2_suit = gameTable->boardCards.flop2.suit;
                fl3_suit = gameTable->boardCards.flop3.suit;
                t_suit = gameTable->boardCards.turn.suit;
                r_suit = gameTable->boardCards.river.suit;
                fl1_value = gameTable->boardCards.flop1.value;
                fl2_value = gameTable->boardCards.flop2.value;
                fl3_value = gameTable->boardCards.flop3.value;
                t_value = gameTable->boardCards.turn.value;
                r_value = gameTable->boardCards.river.value;
            }
            sprintf(write_message1,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%s;%d;%d;%d;%d"
            ,rounds,p->pos
            ,p->c1.suit,p->c1.value
            ,p->c2.suit,p->c2.value
            ,fl1_suit,fl1_value
            ,fl2_suit,fl2_value
            ,fl3_suit,fl3_value
            ,t_suit,t_value
            ,r_suit,r_value
            ,valid_input_int
            ,p->name
            ,p->chips,p->bet_chips
            ,gameTable->bet_chips_min,gameTable->pot);
		printf("When sending, the round is: %d\n",rounds);
            bool n = TCP_write_wstring(p->addr,buffer,write_message1);
		return n;
}

bool TCP_read_gmsg(int sockfd,char buffer[256]){
    //printf("start reading to the target.\n");
    bzero(buffer,256);
    int n = read(sockfd,buffer,255);
    if (n < 0){ 
         error("ERROR reading from socket\n\n");
         return false;
    }
    else{
    
    if(strchr(buffer,';')){
        int rounds;
        int pos;
        char c1_suit,c2_suit,fl1_suit,fl2_suit,fl3_suit,t_suit,r_suit;
        int c1_value,c2_value,fl1_value,fl2_value,fl3_value,t_value,r_value;
    sscanf(buffer,"%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d",
    &rounds,&pos,&c1_suit,&c1_value,&c2_suit,&c2_value,&fl1_suit,&fl1_value,&fl2_suit,&fl2_value,&fl3_suit,&fl3_value,&t_suit,&t_value,&r_suit,&r_value);

    printf("\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\n\n"
    ,rounds,pos, c1_suit,c1_value,c2_suit,c2_value,fl1_suit,fl1_value,fl2_suit,fl2_value,fl3_suit,fl3_value,t_suit,t_value,r_suit,r_value);
    }
    else{
        printf("%s\n",buffer);
    }
     
        return true;
    }
}

bool TCP_read_noprint(int sockfd,char buffer[256]){
    //printf("start reading to the target.\n");
    bzero(buffer,256);
    int n = read(sockfd,buffer,255);
    if (n < 0){ 
         error("ERROR reading from socket\n\n");
         return false;
    }
    else{
        //printf("%s\n",buffer);
        //printf("Message from target: %s\n",buffer);
	    //printf("reading succeed.\n\n");
        return true;
    }
}

bool UpdateAllPlayer(player* p, char buffer[256],int player_action,int player_amount,int player_num){
	player* current = p->next;
    char write_message1[256];
	char player_action_string[10];
	printf("Start updating..\n");
    for(int i=0;i<player_num;i++){
	switch(player_action){
		case 0:
			strcpy(player_action_string,"FOLD");
			break;
		case 1:
			strcpy(player_action_string,"CHECK");
			break;
	}

        if(player_action !=2){
            sprintf(write_message1,"Player %s made decision: %s",p->name,player_action_string);
        }
        else{
            sprintf(write_message1,"Player %s made decision: bet to %d",p->name,player_amount);
        }
        bool n = TCP_write_wstring(current->addr,buffer,write_message1);
	    printf("Finish updating with %s\n",current->name);
        if(n==false){
            return false;
        }
        TCP_read(current->addr,buffer);
        current = current->next;
    }
    return true;
}

int compareString(const char *input) {
    // List of strings to compare against
    const char *stringsToCompare[] = {
        "How much do you want to bet?"
        // Add more strings as needed
    };

    // Number of strings in the list
    int numStrings = sizeof(stringsToCompare) / sizeof(stringsToCompare[0]);

    // Iterate over the list of strings
    for (int i = 0; i < numStrings; i++) {
        if (strcmp(input, stringsToCompare[i]) == 0) {
            return 0; // false
        }
    }

    return 1; // true
}

bool UpdateAllPlayer_status(table* gameTable,player* p, char buffer[256],int player_num){
    player* current = gameTable->playerList;
    char write_message1[256];
    sprintf(write_message1,"UPDATE_STATUS: player %s's turn, plyer minimum bet: %d. pot: %d.",p->name,gameTable->bet_chips_min,gameTable->pot);
    for(int i=0;i<player_num;i++){
        bool n = TCP_write_wstring(current->addr,buffer,write_message1);
        if(n==false){
            return false;
        }
        TCP_read(current->addr,buffer);
        current = current->next;
    }
    return true;
}

bool UpdateAllPlayer_self(table* gameTable,player* p, char buffer[256]){
    char write_message1[256];
    sprintf(write_message1,"UPDATE_SELF: bet: %d. have bet: %d. min bet:%d.",p->chips,p->bet_chips,gameTable->bet_chips_min);
    bool n = TCP_write_wstring(p->addr,buffer,write_message1);
    if(n==false){
        return false;
    }
    TCP_read(p->addr,buffer);
    
    return true;
}

bool UpdateAllPlayer_complete(table* gameTable,player* p, char buffer[256],int player_num){
    player* current = gameTable->playerList;
    char write_message1[256];
    //sprintf(write_message1,"UPDATE_COMPLETE: %s,%d,%d,%d,%d",p->name,current->chips,current->bet_chips,gameTable->bet_chips_min,gameTable->pot);
    for(int i=0;i<player_num;i++){
	sprintf(write_message1,"UPDATE_COMPLETE: %s,%d,%d,%d,%d",p->name,current->chips,current->bet_chips,gameTable->bet_chips_min,gameTable->pot);
        bool n = TCP_write_wstring(current->addr,buffer,write_message1);
        if(n==false){
            return false;
        }
        TCP_read(current->addr,buffer);
        current = current->next;
    }
    return true;
}

// for playersTurn: -1 = player busted, 0 = dealer hides one card, 1 = player shows all cards, 2 = dealer shows all cards, 3 = Blackjack
void TCP_write_BJmsg(int sockfd, char buffer[256], int playersTurn, card* hand, int valid_input){
    int handLength, handValue;
    char BJmsg[256];
    handLength = findHandLength(hand);
    handValue = findHandValue(hand);
    if(playersTurn == 0){
        hand[1].suit = 'X';
        hand[1].value = 0;
    }
    sprintf(BJmsg, "%d#%d#%d#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d", playersTurn, handLength, handValue, valid_input, 
            hand[0].suit, hand[0].value, hand[1].suit, hand[1].value, hand[2].suit, hand[2].value, hand[3].suit, hand[3].value, hand[4].suit, hand[4].value,
            hand[5].suit, hand[5].value, hand[6].suit, hand[6].value, hand[7].suit, hand[7].value, hand[8].suit, hand[8].value, hand[9].suit, hand[9].value);
    TCP_write_wstring(sockfd, buffer, BJmsg);
}