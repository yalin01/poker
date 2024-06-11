#include <stdbool.h>
#include "struct.h"
#ifndef rules_h
#define rules_h

bool isRoyalFlush(card* hand);
bool isStraightFlush(card* hand);
bool isFourOfAKind(card* hand);
bool isFullHouse(card* hand);
bool isFlush(card* hand);
bool isStraight(card* hand);
bool isThreeOfAKind(card* hand);
bool isTwoPair(card* hand);
bool isOnePair(card* hand);
int highCard(card* hand);

int compareHighCard(card* hand1, card* hand2);
int compareStraightFlush(card* hand1, card* hand2);
int compareFourOfAKind(card* hand1, card* hand2);
int compareFullHouse(card* hand1, card* hand2);
int compareFlush(card* hand1, card* hand2);
int compareStraight(card* hand1, card* hand2);
int compareThreeOfAKind(card* hand1, card* hand2);
int compareTwoPair(card* hand1, card* hand2);
int compareOnePair(card* hand1, card* hand2);


#endif // RULES_H
