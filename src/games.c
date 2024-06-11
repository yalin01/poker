#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h> 
#include "games.h"
#include "rules.h"


#define DECK_SIZE 52    // 52 cards, no joker

void initializeDeck(card* deck) {
    char suits[] = {'H', 'D', 'C', 'S'};    // H for Heart, D for Diamond, C for Club, S for Spade
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j <= 13; j++) {
            deck[k].suit = suits[i];
            deck[k].value = j;
            k++;
        }
    }
}

void shuffle(card* deck) {
    srand(time(NULL));
    for (int i = 0; i < DECK_SIZE; i++) {
        int r = rand() % DECK_SIZE;
        card temp = deck[i];
        deck[i] = deck[r];
        deck[r] = temp;
    }
}

const char* getPositionName(int pos) {
    switch (pos) {
        case 0: return "Dealer";
        case 1: return "Small Blind";
        case 2: return "Big Blind";
        default: return "Normie";
    }
}

player* createPlayerList(int numPlayers) {
    const char* names[] = {"Alex", "Yian", "Aidan", "Thomas"}; // Player names
    player* head = NULL;
    player* prev = NULL;
    for (int i = 0; i < numPlayers; i++) {
        player* newPlayer = malloc(sizeof(player));
        strcpy(newPlayer->name, names[i]); // Initialize names
        newPlayer->chips = 1000;           // Initialize chips
        newPlayer->pos = i;                // Initialize pos
        newPlayer->next = NULL;
        if (!head) {
            head = newPlayer;
            head->prev = NULL;
        } else {
            prev->next = newPlayer;
            newPlayer->prev = prev;
        }
        prev = newPlayer;
    }
    head->prev = prev; // Makes the list circular
    prev->next = head; // Makes the list circular
    return head;
}

player* createPlayerList_wnames(int numPlayers, char* names[], int player_addr[]) {
    //const char* names[] = {"Alex", "Yian", "Aidan", "Thomas"}; // Player names
    player* head = NULL;
    player* prev = NULL;
    for (int i = 0; i < numPlayers; i++) {
        player* newPlayer = malloc(sizeof(player));
        strcpy(newPlayer->name, names[i]); // Initialize names
        newPlayer->chips = 1000;           // Initialize chips
        newPlayer->pos = i;                // Initialize pos
        newPlayer->next = NULL;
        newPlayer->addr = player_addr[i];
        newPlayer->checkflag=false;
        newPlayer->ingameStatus = true;
        newPlayer->bet_chips = 0;
        if (!head) {
            head = newPlayer;
            head->prev = NULL;
        } else {
            prev->next = newPlayer;
            newPlayer->prev = prev;
        }
        prev = newPlayer;
    }
    head->prev = prev; // Makes the list circular
    prev->next = head; // Makes the list circular
    return head;
}

player* createPlayerList_wnames_wchips(int numPlayers, char* names[], int player_addr[], int player_chips[]) {
    //const char* names[] = {"Alex", "Yian", "Aidan", "Thomas"}; // Player names
    player* head = NULL;
    player* prev = NULL;
    for (int i = 0; i < numPlayers; i++) {
        player* newPlayer = malloc(sizeof(player));
        strcpy(newPlayer->name, names[i]); // Initialize names
        newPlayer->chips = player_chips[i];           // Initialize chips
        newPlayer->pos = i;                // Initialize pos
        newPlayer->next = NULL;
        newPlayer->addr = player_addr[i];
        newPlayer->checkflag=false;
        newPlayer->ingameStatus = true;
        newPlayer->bet_chips = 0;
        if (!head) {
            head = newPlayer;
            head->prev = NULL;
        } else {
            prev->next = newPlayer;
            newPlayer->prev = prev;
        }
        prev = newPlayer;
    }
    head->prev = prev; // Makes the list circular
    prev->next = head; // Makes the list circular
    return head;
}

void freePlayerList(player* head) {
    if (!head) return;
    player* tmp = head;
    do {
        player* next = tmp->next;
        free(tmp);
        tmp = next;
    } while (tmp != head);
}

void dealCards(table* gameTable, card* deck) {
    int deckIndex = 0;
    player* current = gameTable->playerList;
    do {
        current->c1 = deck[deckIndex++];
        current->c2 = deck[deckIndex++];
        current = current->next;
    } while (current != gameTable->playerList);
    gameTable->boardCards.flop1 = deck[deckIndex++];
    gameTable->boardCards.flop2 = deck[deckIndex++];
    gameTable->boardCards.flop3 = deck[deckIndex++];
    gameTable->boardCards.turn = deck[deckIndex++];
    gameTable->boardCards.river = deck[deckIndex++];
}

void initializeTable(table* gameTable, int numPlayers) {
    gameTable->playerList = createPlayerList(numPlayers);
    gameTable->pot = 0;
}

void initializeTable_wnames(table* gameTable, int numPlayers,char* names[],int player_addr[]) {
    gameTable->playerList = createPlayerList_wnames(numPlayers,names,player_addr);
    gameTable->pot = 0;
	gameTable->bet_chips_min = 0;
}

void initializeTable_wnames_wchips(table* gameTable, int numPlayers,char* names[],int player_addr[],int player_chips[]) {
    gameTable->playerList = createPlayerList_wnames_wchips(numPlayers,names,player_addr,player_chips);
    gameTable->pot = 0;
	gameTable->bet_chips_min = 0;
}

int evaluateHand(card* hand) {
    // return a ranking score for the given hand
    if (isRoyalFlush(hand)) return 10;
    if (isStraightFlush(hand)) return 9;
    if (isFourOfAKind(hand)) return 8;
    if (isFullHouse(hand)) return 7;
    if (isFlush(hand)) return 6;
    if (isStraight(hand)) return 5;
    if (isThreeOfAKind(hand)) return 4;
    if (isTwoPair(hand)) return 3;
    if (isOnePair(hand)) return 2;
    return highCard(hand); // High card
}

void findBestHand(card* hand, card* bestHand) {
    card tempHand[5];
    int bestRank = -1;

    // Generate all combinations of 5 cards out of 7
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 7; j++) {
            int k = 0;
            for (int m = 0; m < 7; m++) {
                if (m != i && m != j) {
                    tempHand[k++] = hand[m];
                }
            }
            // Evaluate the current combination
            int rank = evaluateHand(tempHand);
            if (rank > bestRank) {
                bestRank = rank;
                memcpy(bestHand, tempHand, sizeof(card) * 5);
            }
        }
    }
}

/*void findBestCombination(table* gameTable) {
    player* current = gameTable->playerList;
    card hand[7], bestHand[5];

    // Combine player's cards and community cards into hand array
    do {
        hand[0] = current->c1;
        hand[1] = current->c2;
        hand[2] = gameTable->boardCards.flop1;
        hand[3] = gameTable->boardCards.flop2;
        hand[4] = gameTable->boardCards.flop3;
        hand[5] = gameTable->boardCards.turn;
        hand[6] = gameTable->boardCards.river;

        findBestHand(hand, bestHand);

        int handRank = evaluateHand(bestHand);

        // Output the best hand and its rank for the current player
        printf("Best hand for %s: ", current->name);
        for (int i = 0; i < 5; i++) {
            printf("%c%d ", bestHand[i].suit, bestHand[i].value);
        }
        printf(" | Hand Rank: %d\n", handRank);

        current = current->next;
    } while (current != gameTable->playerList);
}*/

void findBestCombination(table* gameTable) {
    player* current = gameTable->playerList;
    card hand[7], bestHand[5];

    // Combine player's cards and community cards into hand array
    do {
        hand[0] = current->c1;
        hand[1] = current->c2;
        hand[2] = gameTable->boardCards.flop1;
        hand[3] = gameTable->boardCards.flop2;
        hand[4] = gameTable->boardCards.flop3;
        hand[5] = gameTable->boardCards.turn;
        hand[6] = gameTable->boardCards.river;


        findBestHand(hand, bestHand);
        current->handRank = evaluateHand(bestHand);        // store hand rank value directly to the new variable ‘int handRank’  in ‘struct player’

        for (int i = 0; i < 5; i++) {
            current->bestHand[i] = bestHand[i];            // store best-5 combo directly to the new card array ‘card bestHand[5]’  in ‘struct player’
        }
        
        // Output the best hand and its rank for the current player
        printf("Best hand for %s: ", current->name);
        for (int i = 0; i < 5; i++) {
            printf("%c%d ", current->bestHand[i].suit, current->bestHand[i].value);    // print the ‘card bestHand[5]’ for each player from their struct
        }
        printf(" | Hand Rank: %d\n", current->handRank);        // print the ‘int handRank’ for each player from their struct

        current = current->next;
    } while (current != gameTable->playerList);
}

bool isallFold(table* gameTable,int player_num){
    //player* current = head;
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if(current->ingameStatus==false){
            current = current->next;
        }
        else{
            return false;
        }
    }
    return true;
}

bool isallChipsEqual(table* gameTable,int player_num){
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if((current->bet_chips==gameTable->bet_chips_min &&current->ingameStatus==true) || current->ingameStatus==false){
            current = current->next;
        }
        else{
            return false;
        }
    }
    return true;
}

bool isallCheck(table* gameTable,int player_num){
    //player* current = head;
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if((current->checkflag==true && current->ingameStatus == true)||current->ingameStatus == false){
            current = current->next;
        }
        else{
            return false;
        }
    }
    return true;
}

void resetallCheck(table* gameTable, int player_num){
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        //current->checkflag = false;
	if(current->ingameStatus == true){
                current->checkflag = false;
        }
        current = current->next; 
    }
}


bool isanyCheck(table* gameTable,int player_num){
    //player* current = head;
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if(current->checkflag==true && current->ingameStatus == true){
            //current = current->next;
		printf("At least one player CHECK\n");
		return true;
        }
	current = current->next;
	/**
        else{
            //return true;
        }
	**/
    }
	printf("no player CHECK\n");
    return false;
}

player* inital_bet_setup(table* gameTable, int initial_bet, int player_num){
    printf("TRIGGERED inital_bet_setup\n");
	player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if(current->pos==1){
            current->bet_chips=(initial_bet/2);
            current->chips -= (initial_bet/2);
	//gameTable->bet_chips_min = initial_bet/2;
		gameTable->pot += initial_bet/2;
		//printf("PLAYER %s now has chips %d wtih bet %d\n",current->name,current->chips,current->bet_chips);
		//printf("Table min bet is %d\n",gameTable->bet_chips_min);
        }
        else if(current->pos==2){
            current->bet_chips=initial_bet;
            current->chips -= initial_bet;
		//gameTable->bet_chips_min = initial_bet;
		gameTable->pot += initial_bet;
		//printf("PLAYER %s now has chips %d wtih bet %d\n",current->name,current->chips,current->bet_chips);

        }
	current = current->next;
    }
	gameTable->bet_chips_min = initial_bet;
    //gameTable.bet_chips_min = initial_bet;
	//printf("after looping,table bet chips min is %d\n",gameTable->bet_chips_min);
    player* result = gameTable->playerList;
    if(player_num==2 || player_num==3){
        //printf("after looping2,table bet chips min is %d\n",gameTable->bet_chips_min);
	return result;
    }
    else{
        result = result->next;
        result = result->next;
        result = result->next;
	//printf("after looping3,table bet chips min is %d\n",gameTable->bet_chips_min);
        return result;
    }
}

/*player* determineWinner(table* gameTable) {
    player* currentPlayer = gameTable->playerList;
    player* winner = currentPlayer;
    int highestScore = 0;

    do {
        card hand[7] = {
            currentPlayer->c1, currentPlayer->c2,
            gameTable->boardCards.flop1, gameTable->boardCards.flop2,
            gameTable->boardCards.flop3, gameTable->boardCards.turn,
            gameTable->boardCards.river
        };

        int score = evaluateHand(hand);
        if (score > highestScore) {
            highestScore = score;
            winner = currentPlayer;
        }

        currentPlayer = currentPlayer->next;
    } while (currentPlayer != gameTable->playerList);

    return winner;
}*/

player* determineWinner_wfold(player* head, int numPlayers) {
    if (head == NULL) {
        return NULL;
    }
    
    player* current = head;
    player* winner = NULL;

    // Iterate through the circular doubly linked list of players
    for (int i = 0; i < numPlayers; i++) {
        if (current->ingameStatus) {
            if (winner == NULL) {
                winner = current;
            } else if (current->handRank > winner->handRank) {
                winner = current;
            } else if (current->handRank == winner->handRank) {
                printf("TRIGGER TIEBREAKER\n");
                // Compare hands for tie-breaking
                int comparisonResult = compareHands(current->bestHand, winner->bestHand, current->handRank);
                if (comparisonResult > 0) {
                    winner = current;
                }
            }
        }
        current = current->next;
    }

    return winner;
}

player* findPrevActivePlayer(player* currentPlayer,int player_num){
    for(int i =0;i<player_num;i++){
        currentPlayer = currentPlayer->prev;
        if(currentPlayer->ingameStatus==true){
            return currentPlayer;
        }
    }
    return NULL;
}

bool isrestFold(table* gameTable,int player_num){
    //player* current = head;
    player* current = gameTable->playerList;
    int fold_count = 0;
    for(int i =0;i<player_num;i++){
        if(current->ingameStatus==false){
            fold_count++;
        }
	current = current->next;
    }
	printf("Among all %d players, %d fold\n",player_num,fold_count);
    if(fold_count==player_num-1){
        return true;
    }
    else{
        return false;
    }
}

void resetallBetChips(table* gameTable, int player_num){
    player* current = gameTable->playerList;
    for(int i =0;i<player_num;i++){
        if(current->ingameStatus == true){
		current->bet_chips = 0;
	}
	//current->bet_chips = 0;
        current = current->next;
    }
}

int compareHands(card* hand1, card* hand2, int handRank) {
    // tie-breaking rules based on hand rank
    switch (handRank) {
        case 9: return compareStraightFlush(hand1, hand2); // Straight flush
        case 8: return compareFourOfAKind(hand1, hand2);   // Four of a kind
        case 7: return compareFullHouse(hand1, hand2);     // Full house
        case 6: return compareFlush(hand1, hand2);         // Flush
        case 5: return compareStraight(hand1, hand2);      // Straight
        case 4: return compareThreeOfAKind(hand1, hand2);  // Three of a kind
        case 3: return compareTwoPair(hand1, hand2);       // Two pair
        case 2: return compareOnePair(hand1, hand2);       // One pair
        default: return compareHighCard(hand1, hand2);     // High card
    }
}

bool contains(int arr[], int num, int index) { 
   if(index==0){
		return false;
	} 
   for (int i = 0; i <= index ; i++) {
        if (arr[i] == num) {
            return true;
        }
    }
    return false;
}

void dealCardsBJ(int deckIndex, card* deck, card* dealerHand, card* playerHand){
    dealerHand[0] = deck[deckIndex++];
    dealerHand[1] = deck[deckIndex++];
    playerHand[0] = deck[deckIndex++];
    playerHand[1] = deck[deckIndex++];
    for(int i = 2; i < 10; i++){
        dealerHand[i].suit = 'X';
        dealerHand[i].value = 0 ;
        playerHand[i].suit = 'X';
        playerHand[i].value = 0 ;
    }
}

int findHandLength(card* hand){
    int length = 0;
    for(int i = 0; i < 10; i++){
        if(hand[i].value != 0) length++;
    }
    return length;
}

int findHandValue(card* hand){
    int value = 0;
    bool ace = false;
    for(int i = 0; i < 10; i++){
        if(hand[i].value == 1){
            value += 11;
            ace = true;
        }
        else if(hand[i].value >= 10) value += 10;
        else value += hand[i].value;
    }
    if(value > 21 && ace == true) value -= 10;
    return value;
}

int findBJWinner(card* dealerHand, card* playerHand){
    int playerScore, dealerScore;
    playerScore = findHandValue(playerHand);
    dealerScore = findHandValue(dealerHand);
    if(playerScore > dealerScore) return 1;
    if(playerScore == dealerScore) return 2;
    else return 0;
}
