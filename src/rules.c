#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "rules.h"

int highCard(card* hand) {
    int max = 0;
    for (int i = 0; i < 5; i++) {
        if (hand[i].value > max) {
            max = hand[i].value;
        }
    }
    return 1 + (max - 1) / 13; // Scale high card values to 1-13 range
}

bool isOnePair(card* hand) {
    int counts[14] = {0};
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    for (int i = 1; i <= 13; i++) {
        if (counts[i] == 2) return true;
    }
    return false;
}

bool isTwoPair(card* hand) {
    int counts[14] = {0};
    int pairCount = 0;
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    for (int i = 1; i <= 13; i++) {
        if (counts[i] == 2) pairCount++;
    }
    return pairCount == 2;
}

bool isThreeOfAKind(card* hand) {
    int counts[14] = {0};
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    for (int i = 1; i <= 13; i++) {
        if (counts[i] == 3) return true;
    }
    return false;
}

bool isStraight(card* hand) {
    int values[5];
    for (int i = 0; i < 5; i++) {
        values[i] = hand[i].value;
    }
    // Sort the values
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (values[i] > values[j]) {
                int temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }
    // Check for straight
    for (int i = 0; i < 4; i++) {
        if (values[i] + 1 != values[i + 1]) {
            return false;
        }
    }
    return true;
}

bool isFlush(card* hand) {
    /*char suit = hand[0].suit;
    for (int i = 1; i < 5; i++) {
        if (hand[i].suit != suit) return false;
    }
    return true;*/
    int suits[4] = {0};     // Club, Diamond, Heart, Spade
    for(int i = 0; i < 4; i++){
        if(hand[i].suit == 'C') suits[0]++;
        else if(hand[i].suit == 'D') suits[1]++;
        else if(hand[i].suit == 'H') suits[2]++;
        else if(hand[i].suit == 'S') suits[3]++;
    }
    if(suits[0] >= 5 || suits[1] >= 5 || suits[2] >= 5 || suits[3] >= 5)
        return true;
    return false;
}

bool isFullHouse(card* hand) {
    int counts[14] = {0};
    bool hasThree = false, hasTwo = false;
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    for (int i = 1; i <= 13; i++) {
        if (counts[i] == 3) hasThree = true;
        if (counts[i] == 2) hasTwo = true;
    }
    return hasThree && hasTwo;
}

bool isFourOfAKind(card* hand) {
    int counts[14] = {0};
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    for (int i = 1; i <= 13; i++) {
        if (counts[i] == 4) return true;
    }
    return false;
}

bool isStraightFlush(card* hand) {
    // Check for flush and straight
    return isFlush(hand) && isStraight(hand);
}

bool isRoyalFlush(card* hand){
    int counts[14] = {0};
    for (int i = 0; i < 5; i++) {
        counts[hand[i].value]++;
    }
    if(counts[1]  == 1 &&
       counts[10] == 1 &&
       counts[11] == 1 &&
       counts[12] == 1 &&
       counts[13] == 1) return true;
    return false;
}

//Comparison for Tie-breakng logic

int compareHighCard(card* hand1, card* hand2) {
    // Compare high cards in descending order
    for (int i = 4; i >= 0; i--) {
        if (hand1[i].value > hand2[i].value) return 1;
        if (hand1[i].value < hand2[i].value) return -1;
    }
    return 0;
}

int compareStraightFlush(card* hand1, card* hand2) {
    return compareHighCard(hand1, hand2); // Straight flush is determined by the highest card
}

int compareFourOfAKind(card* hand1, card* hand2) {
    // Compare the four of a kind first, then the kicker
    int four1 = (hand1[0].value == hand1[3].value) ? hand1[0].value : hand1[1].value;
    int four2 = (hand2[0].value == hand2[3].value) ? hand2[0].value : hand2[1].value;
    if (four1 > four2) return 1;
    if (four1 < four2) return -1;
    return compareHighCard(hand1, hand2); // Compare kickers if four of a kind is the same
}

int compareFullHouse(card* hand1, card* hand2) {
    // Compare the three of a kind first, then the pair
    int three1 = (hand1[0].value == hand1[2].value) ? hand1[0].value : hand1[2].value;
    int three2 = (hand2[0].value == hand2[2].value) ? hand2[0].value : hand2[2].value;
    if (three1 > three2) return 1;
    if (three1 < three2) return -1;
    int pair1 = (hand1[0].value == hand1[2].value) ? hand1[3].value : hand1[0].value;
    int pair2 = (hand2[0].value == hand2[2].value) ? hand2[3].value : hand2[0].value;
    if (pair1 > pair2) return 1;
    if (pair1 < pair2) return -1;
    return 0;
}

int compareFlush(card* hand1, card* hand2) {
    return compareHighCard(hand1, hand2); // Flush is determined by the highest card
}

int compareStraight(card* hand1, card* hand2) {
    return compareHighCard(hand1, hand2); // Straight is determined by the highest card
}

int compareThreeOfAKind(card* hand1, card* hand2) {
    // Compare the three of a kind first, then the remaining high cards
    int three1 = (hand1[0].value == hand1[2].value) ? hand1[0].value : hand1[2].value;
    int three2 = (hand2[0].value == hand2[2].value) ? hand2[0].value : hand2[2].value;
    if (three1 > three2) return 1;
    if (three1 < three2) return -1;
    return compareHighCard(hand1, hand2); // Compare remaining high cards if three of a kind is the same
}

int compareTwoPair(card* hand1, card* hand2) {
    // Compare the highest pair, then the second pair, then the kicker
    int pair1a = (hand1[0].value == hand1[1].value) ? hand1[0].value : hand1[2].value;
    int pair1b = (hand1[2].value == hand1[3].value) ? hand1[2].value : hand1[4].value;
    int pair2a = (hand2[0].value == hand2[1].value) ? hand2[0].value : hand2[2].value;
    int pair2b = (hand2[2].value == hand2[3].value) ? hand2[2].value : hand2[4].value;
    if (pair1a > pair2a) return 1;
    if (pair1a < pair2a) return -1;
    if (pair1b > pair2b) return 1;
    if (pair1b < pair2b) return -1;
    return compareHighCard(hand1, hand2); // Compare kicker if pairs are the same
}

int compareOnePair(card* hand1, card* hand2) {
    // Compare the pair first, then the remaining high cards
    int pair1 = (hand1[0].value == hand1[1].value) ? hand1[0].value : (hand1[1].value == hand1[2].value) ? hand1[1].value : hand1[2].value;
    int pair2 = (hand2[0].value == hand2[1].value) ? hand2[0].value : (hand2[1].value == hand2[2].value) ? hand2[1].value : hand2[2].value;
    if (pair1 > pair2) return 1;
    if (pair1 < pair2) return -1;
    return compareHighCard(hand1, hand2); // Compare remaining high cards if pairs are the same
}