#include <stdbool.h>
#include "struct.h"
#ifndef games_h
#define games_h

void initializeDeck(card* deck);
void shuffle(card* deck);
const char* getPositionName(int pos);
player* createPlayerList(int numPlayers);
player* createPlayerList_wnames(int numPlayers, char* names[],int player_addr[]);
void freePlayerList(player* head);
void dealCards(table* gameTable, card* deck);
void initializeTable(table* gameTable, int numPlayers);
void initializeTable_wnames(table* gameTable, int numPlayers, char* names[],int player_addr[]);
int evaluateHand(card* hand);
void findBestHand(card* hand, card* bestHand);
void findBestCombination(table* gameTable);
//bool isallFold(player* head,int player_num);
bool isallFold(table* gameTable,int player_num);
bool isallChipsEqual(table* gameTable,int player_num);
bool isallCheck(table* gameTable,int player_num);
void resetallCheck(table* gameTable, int player_num);
bool isanyCheck(table* gameTable,int player_num);
player* inital_bet_setup(table* gameTable, int initial_bet, int player_num);
player* determineWinner(player* head, int numPlayers);
player* determineWinner_wfold(player* head, int numPlayers);
player* findPrevActivePlayer(player* currentPlayer,int player_num);
bool isrestFold(table* gameTable,int player_num); 
void resetallBetChips(table* gameTable, int player_num);
int compareHands(card* hand1, card* hand2, int handRank);
bool contains(int arr[], int num, int index);
player* createPlayerList_wnames_wchips(int numPlayers, char* names[], int player_addr[], int player_chips[]);
void initializeTable_wnames_wchips(table* gameTable, int numPlayers,char* names[],int player_addr[],int player_chips[]);

void dealCardsBJ(int deckIndex, card* deck, card* dealerHand, card* playerHand);
int findHandLength(card* hand);
int findHandValue(card* hand);
int findBJWinner(card* dealerHand, card* playerHand);

#endif // GAMES_H
