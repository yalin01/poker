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

#define DECK_SIZE 52    // 52 cards, no joker


int main(int argc, char *argv[])
{
    #ifndef DEBUG
	if (argc < 5) {
       		printf("Usage: poker_server <port#> <gameMode> <playerNum> <mininumBet>\n");
       		exit(0);
    }
	int player_num_input=atoi(argv[3]);
    int initial_bet; 
    char* gameMode;
    initial_bet= atoi(argv[4]);
    gameMode = argv[2];
    if(strcmp(gameMode, "Texas") == 0){
        printf("Now playing Texas Hold'Em\n");
        if(player_num_input < 2 || player_num_input > 10){
            printf("Player number must be between 2 and 10 for Texas Hold'Em\n");
            exit(0);
        }
    }
    else if(strcmp(gameMode, "Blackjack") == 0){
        printf("Now playing Blackjack\n");
        if(player_num_input != 1){
            printf("Player number is limited to 1 for Blackjack\n");
            exit(0);
        }
    }
    else if(strcmp(gameMode, "Blackjack") != 0 || strcmp(gameMode, "Blackjack") != 0){
        printf("Invalid gameMode\n");
        exit(0);
    }
    int player_num;
	player_num = player_num_input;
    printf("game player number has been set to: %d\n", player_num);
    printf("initial_bet has been set to: %d\n", initial_bet);
     
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

    int player_addr_init[player_num];
    for(int i =0;i<player_num;i++){
        listen(sockfd,player_num);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
          error("ERROR on accept");
        player_addr_init[i]=newsockfd;
        printf("Player %d connected.\n",i+1);
        
    }

   
    char* player_name_init[player_num];
    for(int i = 0; i<player_num;i++){
        TCP_write_wstring(player_addr_init[i],buffer,"What is your name?");
        TCP_read(player_addr_init[i],buffer);
        int length = strlen(buffer);
	    player_name_init[i]=(char*)malloc((length+1)*sizeof(char));
        strcpy(player_name_init[i],buffer);
    }

    int player_pos[player_num];
    for(int i = 0; i<player_num;i++){
        char write_message[256];
        sprintf(write_message,"What position do you want? (1-%d)",player_num);
        char write_message2[256];
        sprintf(write_message2,"Try again! What position do you want? (1-%d)",player_num);
        //TCP_write_wstring(player_addr_init[i],buffer,"What position do you want?");
        TCP_write_wstring(player_addr_init[i],buffer,write_message);
        TCP_read(player_addr_init[i],buffer);
        int order = atoi(buffer);
        while(contains(player_pos,order,i)==true || order<1 || order>player_num){
            TCP_write_wstring(player_addr_init[i],buffer,write_message2);
            TCP_read(player_addr_init[i],buffer);
            order = atoi(buffer);
        }
        player_pos[i]=order;
    }

    int player_pos_index[player_num];
    for(int i = 0; i<player_num;i++){
        player_pos_index[i]=player_pos[i]-1;
    }

    char *player_name[player_num];
    int player_addr[player_num];
    for(int i = 0; i<player_num;i++){
	int length = strlen(player_name_init[i]);
	player_name[player_pos_index[i]]=(char*)malloc((length+1)*sizeof(char));
	strcpy(player_name[player_pos_index[i]],player_name_init[i]);
        player_addr[player_pos_index[i]]=player_addr_init[i];
    }

    int player_chips[player_num];
    for(int i = 0; i<player_num;i++){
        player_chips[i]=1000;
    }

   /* for(int i = 0; i<player_num;i++){
        char write_message2[256];
        sprintf(write_message2,"%d*%d*%d",player_num,initial_bet,i);
        TCP_write_wstring(player_addr[i],buffer,write_message2);
	    TCP_read(player_addr[i],buffer);
    }
    */
    table gameTable;
    card deck[DECK_SIZE];
    bool valid_input;
    int rounds;
    bool rounds_start;
    int player_action;
    int player_amount;
    int game_over = 0;

    if(strcmp(gameMode, "Texas") == 0){    // game is Texas Hold'Em
        
        while(game_over == 0){
        for(int i = 0; i<player_num;i++){
            char write_message2[256];
            sprintf(write_message2,"GAMEMODE: %d", 0);
            TCP_write_wstring(player_addr[i],buffer,write_message2);
            TCP_read(player_addr[i],buffer);
            sprintf(write_message2,"%d*%d*%d",player_num,initial_bet,i);
            TCP_write_wstring(player_addr[i],buffer,write_message2);
            TCP_read(player_addr[i],buffer);
        }
        
        initializeDeck(deck);
        shuffle(deck);
        //initializeTable_wnames(&gameTable, player_num,player_name,player_addr);
        initializeTable_wnames_wchips(&gameTable, player_num,player_name,player_addr,player_chips);
        dealCards(&gameTable, deck);
        player* p = gameTable.playerList;

        p = inital_bet_setup(&gameTable,initial_bet,player_num);


        valid_input = true;
        rounds = 1;
        rounds_start = false;
        player_action = 0;
        player_amount = 0;
        while(1){
        
            
            if(isallChipsEqual(&gameTable,player_num)==true && rounds_start==false &&isanyCheck(&gameTable,player_num)==false){
                printf("TRIGGER all bets the same next round\n");
            rounds++;
                rounds_start = true;
                resetallCheck(&gameTable,player_num);
                gameTable.bet_chips_min = 0;
            resetallBetChips(&gameTable,player_num);
            p = gameTable.playerList;
            p = p->next;
            }

            if(isallCheck(&gameTable,player_num)==true && rounds_start == false){
                printf("TRIGGER chekc next rounds\n");
            rounds++;
                rounds_start = true;
                resetallCheck(&gameTable,player_num);
                gameTable.bet_chips_min = 0;
            resetallBetChips(&gameTable,player_num);
            p = gameTable.playerList;
                    p = p->next;

            }

            if(rounds>4){
                            break;
                        }

                if(isrestFold(&gameTable,player_num)==true){
                    break;
                }


                if(p->ingameStatus==true){

                    UpdateAllPlayer_complete(&gameTable,p,buffer,player_num);
                    TCP_write_gmsg(p->addr,buffer,&gameTable,p,rounds,valid_input);
                    TCP_read(p->addr,buffer);
                valid_input = false;
                    if(strcmp(buffer,"FOLD")==0){
                    printf("Player typed FOLD\n");
                        p->ingameStatus=false;
                        valid_input = true;
                        player_action = 0;
                    }
                    
                    else if(strcmp(buffer,"CHECK")==0){
                printf("TRIGGER CHECK\n");
                        if(rounds_start == true || findPrevActivePlayer(p,player_num)->checkflag==true){
                            printf("VALID CHECK\n");
                    p->checkflag = true;
                            valid_input = true;
                            player_action = 1;
                        }
                        else{
                    printf("INVALID CHECK\n");
                            valid_input = false;
                        }
                    }
                    

                    else if(strcmp(buffer,"BET")==0){
                        TCP_write_wstring(p->addr,buffer,"How much do you want to bet?");
                        TCP_read(p->addr,buffer);
                        int bet_amount = atoi(buffer);
                        int bet_change = bet_amount-p->bet_chips;
                        if(bet_change < 0){
                    printf("CANNOT BET LESS\n");
                            valid_input = false;
                        }
                        else if((bet_amount<initial_bet || (bet_amount>initial_bet&&bet_amount<2*initial_bet)) && gameTable.bet_chips_min == 0){
                        printf("FIRST ONE BET INVALID VALUE\n"); 
                valid_input = false;
                        }
                else if((bet_amount<gameTable.bet_chips_min|| (bet_amount >gameTable.bet_chips_min && bet_amount <2*gameTable.bet_chips_min))&& gameTable.bet_chips_min >0){
                    printf("NOT FIRST ONE, BUT BET WITH LOWER CHIPS\n");
                    valid_input = false;
                }
                        else if(bet_change>0 && p->chips<bet_change){
                    printf("NOT ENOUGH CHIPS TO BET\n");
                            valid_input = false;
                        }
                        else if(bet_change == 0){
                    printf("CANNOT BET SAME AMOUNT\n");
                            valid_input = false;
                        }
                        else{
                            p->bet_chips += bet_change;
                            p->chips -= bet_change;
                            gameTable.pot += bet_change;
                            if(p->bet_chips>gameTable.bet_chips_min){
                                gameTable.bet_chips_min = p->bet_chips; 
                            }
                            valid_input = true;
                            player_action = 2;
                            player_amount = bet_amount;
                            resetallCheck(&gameTable,player_num);
                        }
                    }

                    else{
                        valid_input = false;
                    }

                    if(valid_input == true){
                        if(strcmp(buffer,"FOLD")!=0){
                        printf("Trigger rounds start equal to false, BUFFER: %s\n",buffer);
                            rounds_start = false;
                        }
                UpdateAllPlayer(p,buffer,player_action,player_amount,player_num);
                UpdateAllPlayer_complete(&gameTable,p,buffer,player_num);
                        p = p->next;
                    }

                }
            else{
                p = p->next;
            }
            }

            /*
            for(int i = 0; i<player_num;i++){
                TCP_write_wstring(player_addr[i],buffer,"game over.");
                TCP_read(player_addr[i],buffer);
            }
            */

            
            


            findBestCombination(&gameTable);
            player* winnerPlayer = gameTable.playerList;
            player* winner = determineWinner_wfold(winnerPlayer, player_num);
            printf("The winner is %s with score %d\n", winner->name, winner->handRank);
            winner->chips+=gameTable.pot;

            player *p_chips = gameTable.playerList;
            for(int i=0;i<player_num;i++){
                player_chips[i]=p_chips->chips;
                p_chips=p_chips->next;
            }

            for(int i = 0; i<player_num;i++){
                char write_message3[256];
                player* p_current = gameTable.playerList;
                for(int j = 0;j<player_num;j++){
                    sprintf(write_message3,"PLAYER_RESULT: %d,%s,%c,%d,%c,%d,%d",p_current->pos,p_current->name,p_current->c1.suit,p_current->c1.value,p_current->c2.suit,p_current->c2.value,p_current->chips);
                    TCP_write_wstring(player_addr[i],buffer,write_message3);
                    TCP_read(player_addr[i],buffer);
                    p_current = p_current->next;       
                }
            }

            player *p_result = gameTable.playerList;
            for(int i = 0; i<player_num;i++){
                if(p_result == winner){
                    TCP_write_wstring(p_result->addr,buffer,"You are the winner.");
                    //p_result->chips+=gameTable.pot;
                }
                else{
                    TCP_write_wstring(p_result->addr,buffer,"Sorry you lose.");
                }
                TCP_read(p_result->addr,buffer);
                p_result = p_result->next;
            }



            char rematchMsg[256];
            sprintf(rematchMsg, "%s won! QUIT or REMATCH?", winner->name);

            for(int i = 0; i<player_num;i++){
                while(1){
                    TCP_write_wstring(player_addr[i],buffer,rematchMsg);
                    TCP_read(player_addr[i],buffer);
                    if(strcmp(buffer,"QUIT")==0){
                        game_over = 1;
                        break;
                    }
                    else if(strcmp(buffer,"REMATCH")==0){
                        break;
                    }
                }
                if(game_over==1){
                    break;
                }
            }

            if(game_over!=1){
                char *player_name_temp[player_num];
                for(int i = 0; i<player_num;i++){
                    int length = strlen(player_name[i]);
                    player_name_temp[i]=(char*)malloc((length+1)*sizeof(char));
                    strcpy(player_name_temp[i],player_name[i]);
                }
                for(int i=0;i<player_num;i++){
                    free(player_name[i]);
                }
                char *first_name = player_name_temp[0];
                int first_name_length = strlen(player_name_temp[0]);
                for(int i = 0; i<player_num-1;i++){
                    int length = strlen(player_name_temp[i+1]);
                    player_name[i]=(char*)malloc((length+1)*sizeof(char));
                    strcpy(player_name[i],player_name_temp[i+1]);
                }
                player_name[player_num-1]=(char*)malloc((first_name_length+1)*sizeof(char));
                strcpy(player_name[player_num-1],first_name);

                int first_addr = player_addr[0];
                for(int i = 0; i<player_num-1;i++){
                    player_addr[i]=player_addr[i+1];
                }
                player_addr[player_num-1] = first_addr;

                int first_chips = player_chips[0];
                for(int i = 0; i<player_num-1;i++){
                    player_chips[i]=player_chips[i+1];
                }
                player_chips[player_num-1] = first_chips;


                
            }
        }
        freePlayerList(gameTable.playerList);  // Freeing the allocated memory
    }

    else if(strcmp(gameMode, "Blackjack") == 0){   // game is Blackjack
        int deckIndex = 0;
        int betPlaced;
        char sendMsg[256];
        //int handLength, handValue;
        card dealerHand[10];
        card playerHand[10];
        bool quit = false;

        char write_message2[256];
        sprintf(write_message2,"GAMEMODE: %d", 1);
        TCP_write_wstring(player_addr[0],buffer,write_message2);
        TCP_read(player_addr[0],buffer);

        initializeDeck(deck);
        shuffle(deck);
        initializeTable_wnames_wchips(&gameTable, player_num,player_name,player_addr,player_chips);
        player* p = gameTable.playerList;

        TCP_write_wstring(p->addr, buffer, "Welcome to Blackjack! You will play against a computer dealer. Have fun!");
        TCP_read(p->addr, buffer);

        while(quit == false){
            bool busted = false;
            bool game_over = false;
            int valid_input = 1;
            p->bet_chips = 0;
            gameTable.pot = 0;
            sprintf(sendMsg, "You have %d chips.", p->chips);
            TCP_write_wstring(p->addr, buffer, sendMsg);
            TCP_read(p->addr, buffer);
            do{ // prompt for action until valid
                if(valid_input == 1)
                    TCP_write_wstring(p->addr, buffer, "How much do you want to bet?");
                else 
                    TCP_write_wstring(p->addr, buffer, "Invalid bet! How much do you want to bet?");
                TCP_read(p->addr, buffer);
                betPlaced = atoi(buffer);
                if(betPlaced <= 0 || betPlaced > p->chips)
                    valid_input = 0;
                else 
                    valid_input = 1;
            } while(valid_input != 1);   
            p->bet_chips += betPlaced;
            p->chips -= betPlaced;
            gameTable.pot += betPlaced;
            gameTable.pot *= 2; // dealer places a bet

            dealCardsBJ(deckIndex, deck, dealerHand, playerHand);
            deckIndex += 4;

            // player's turn
            do{
                if(findHandValue(playerHand) > 21){ // bust
                    busted = true;
                    game_over = true;
                    TCP_write_BJmsg(p->addr, buffer, -1, playerHand, valid_input);
                    TCP_read(p->addr, buffer);
                    sprintf(sendMsg, "You busted! Dealer wins %d chips!\n", gameTable.pot);
                    TCP_write_wstring(p->addr, buffer, sendMsg);
                    TCP_read(p->addr, buffer);
                    break;
                }
                if(findHandValue(playerHand) == 21 && findHandLength(playerHand) == 2){ // Blackjack
                    TCP_write_BJmsg(p->addr, buffer, 3, playerHand, valid_input);
                    TCP_read(p->addr, buffer);
                    break;
                }
                TCP_write_BJmsg(p->addr, buffer, 0, dealerHand, valid_input);
                TCP_read(p->addr, buffer);
                TCP_write_BJmsg(p->addr, buffer, 1, playerHand, valid_input);
                TCP_read(p->addr, buffer);
                if(strcmp(buffer, "HIT") == 0){
                    playerHand[findHandLength(playerHand)] = deck[deckIndex++]; // hit
                    valid_input = 1;
                }
                else if(strcmp(buffer, "STAND") == 0){
                    break;
                    valid_input = 1;
                }
                else valid_input = 0;
            } while(1);

            // dealer's turn
            busted = false;
            while(game_over == false && busted == false){
                if(findHandValue(dealerHand) > 21){
                    busted = true;
                    game_over = true;
                    TCP_write_BJmsg(p->addr, buffer, 2, dealerHand, valid_input);
                    TCP_read(p->addr, buffer);
                    sprintf(sendMsg, "Dealer busted! %s wins %d chips!\n", p->name, gameTable.pot);
                    p->chips += gameTable.pot;
                    TCP_write_wstring(p->addr, buffer, sendMsg);
                    TCP_read(p->addr, buffer);
                    break;
                }
                if(findHandValue(dealerHand) < 17)
                    dealerHand[findHandLength(dealerHand)] = deck[deckIndex++]; // hit
                if(findHandValue(dealerHand) < findHandValue(playerHand) && findHandValue(dealerHand) < 20)
                    dealerHand[findHandLength(dealerHand)] = deck[deckIndex++]; // hit
                else break;
            }

            // send game result if both standed
            if(game_over == false && busted == false){
                TCP_write_BJmsg(p->addr, buffer, 2, dealerHand, valid_input);
                TCP_read(p->addr, buffer);
                int result = findBJWinner(dealerHand, playerHand);
                if(result == 0)
                    sprintf(sendMsg, "Dealer wins %d chips!\n", gameTable.pot);
                if(result == 1){
                    sprintf(sendMsg, "%s wins %d chips!\n", p->name, gameTable.pot);
                    p->chips += gameTable.pot;
                }
                if(result == 2){
                    sprintf(sendMsg, "Push! %d chips were returned to you.\n", p->bet_chips);
                    p->chips += p->bet_chips;
                }
                TCP_write_wstring(p->addr, buffer, sendMsg);
                TCP_read(p->addr, buffer);
            }

            TCP_write_wstring(p->addr, buffer, "Continue? (Y or N)");
            TCP_read(p->addr, buffer);
            if(strcmp(buffer, "N") != 0){   // continue if not N
                TCP_write_wstring(p->addr, buffer, "\n");
                TCP_read(p->addr, buffer);
                TCP_write_wstring(p->addr, buffer, "Starting a new game..");
                TCP_read(p->addr, buffer);
            }
            else break;
            
            if(deckIndex >= 47){    // shuffle when cards left can't stand another game
                shuffle(deck);
                deckIndex = 0;
            }
        }
    }

	for(int i=0;i<player_num;i++){
		free(player_name[i]);
	}

    for(int i=0;i<player_num;i++){
		free(player_name_init[i]);
	}








//GAME IMPLEMENTATION END HERE
/**
    for(int i = 0; i<player_num;i++){
        TCP_write_wstring(player_addr[i],buffer,"game over.\n");
    }
**/
    

    //game end here
    close(newsockfd);
    close(sockfd);
    

     return 0;
    #endif
    #ifdef DEBUG
    printf("This is server debug test.\n");
    int player_chips[4] = {1000,1000,1000,1000};
    char* player_name[4] = {"ALEX","THOMAS","YIAN","AIDEN"};
    int player_num = 4;
    int player_addr[4] = {0,1,2,3};
    table gameTable;
    card deck[DECK_SIZE];
        initializeDeck(deck);
        shuffle(deck);
        initializeTable_wnames_wchips(&gameTable, player_num,player_name,player_addr,player_chips);
        dealCards(&gameTable, deck);

    player* p = gameTable.playerList;
    do {
        printf("Player %s (%s): %d chips, Cards: %c%d, %c%d\n",
               p->name, getPositionName(p->pos), p->chips, p->c1.suit, p->c1.value, p->c2.suit, p->c2.value);
        p = p->next;
    } while (p != gameTable.playerList);
    printf("Onboard Cards: %c%d, %c%d, %c%d, %c%d, %c%d\n",
           gameTable.boardCards.flop1.suit, gameTable.boardCards.flop1.value,
           gameTable.boardCards.flop2.suit, gameTable.boardCards.flop2.value,
           gameTable.boardCards.flop3.suit, gameTable.boardCards.flop3.value,
           gameTable.boardCards.turn.suit, gameTable.boardCards.turn.value,
           gameTable.boardCards.river.suit, gameTable.boardCards.river.value);

    
    findBestCombination(&gameTable);
	player* winnerPlayer = gameTable.playerList;
    
    // Determine and get the winner
    player* winner = determineWinner_wfold(winnerPlayer, player_num);

    if (winner) {
        printf("The winner is: %s with a hand rank of %d\n", winner->name, winner->handRank);
    } else {
        printf("No winner determined.\n");
    }
    freePlayerList(gameTable.playerList);  // Freeing the allocated memory
    return 0;
    #endif 
}
