#include <stdbool.h>
#ifndef struct_h
#define struct_h

typedef struct card {
    char suit;
    int value;
} card;

typedef struct player {
    char name[50];
    int pos;  // 0 for dealer, 1 for small blind, 2 for big blind, 3 for normie
    int chips;
    int bet_chips;
    card c1;
    card c2;
    struct player* prev;
    struct player* next;
    int addr;
    bool ingameStatus;
    bool checkflag;
    int handRank;       // add int value for their hand rank
    card bestHand[5];   // add a card array which stores the best combo of 5 out of 7

} player;

typedef struct onboardCard {
    card flop1;
    card flop2;
    card flop3;
    card turn;
    card river;
} onboardCard;

typedef struct table {
    player* playerList;
    onboardCard boardCards;
    int pot;
    int bet_chips_min;
} table;


#endif // STRUCT_H
