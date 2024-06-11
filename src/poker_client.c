#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include "TCP.h"
#include "struct.h"
#include "games.h"
#include "rules.h"
#include <ctype.h>
#include <netdb.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#define BUFFER_SIZE 256

void show_role(int pos);
gboolean update_timer(gpointer data);
void reset_timer();
gboolean on_socket_ready(GIOChannel *source, GIOCondition condition, gpointer data);
void on_button_clicked(GtkButton *button, gpointer user_data);
void on_action_button_clicked(GtkButton *button, gpointer user_data);
gboolean on_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data);
void on_resolution_changed(GtkComboBox *combo, gpointer user_data);
void show_final_hands();
void reset_player_hands();
gboolean on_debug_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data);

char buffer[BUFFER_SIZE];
int sockfd, portno;
struct sockaddr_in serv_addr;
struct hostent *server;
char read_message[512];
char print_message1[512] = "";
char print_message2[512] = "";
char print_message3[512] = "";

char current_turn[100] = "";
int current_chips = 0;
int current_bet_chips = 0;
int current_bet_chips_min = 0;
int current_pot = 0;
int rounds = 0;  // Added to track rounds
int player_pos = -1;  // To store the player's position

int gameMode; // 0 for Texas Hold'em, 1 for blackjack

fd_set readfds;
struct timeval timeout;

GtkWidget *drawing_area;
GtkWidget *status_label;
GtkWidget *check_button;
GtkWidget *bet_button;
GtkWidget *fold_button;
GtkWidget *hit_button;   // New HIT button
GtkWidget *stand_button; // New STAND button
GtkWidget *entry;
GdkPixbuf *card1_pixbuf = NULL, *card2_pixbuf = NULL;
GdkPixbuf *flop1_pixbuf = NULL, *flop2_pixbuf = NULL, *flop3_pixbuf = NULL;
GdkPixbuf *turn_pixbuf = NULL, *river_pixbuf = NULL;
GdkPixbuf *back_pixbuf = NULL;
GdkPixbuf *dealer_pixbuf = NULL, *small_blind_pixbuf = NULL, *big_blind_pixbuf = NULL;  // New pixbufs for position images

GdkPixbuf *player_cards_pixbuf[10] = { NULL }; // For blackjack
GdkPixbuf *dealer_cards_pixbuf[10] = { NULL }; 
int global_hand_value = 0;//global var to store sum of player's handcard

int num_players = 0;
bool name_entered = false;

int timer_seconds = 120;
guint timer_id = 0;

typedef struct {
    int pos;
    char name[50];
    char c1_suit;
    int c1_value;
    char c2_suit;
    int c2_value;
    int chip;
} PlayerHand;

PlayerHand player_hands[10];
int player_hand_count = 0;

const char* get_card_filename(char suit, int value) {
    char *filename = malloc(256);
    const char* suit_str;
    const char* value_str;

    switch (suit) {
        case 'H': suit_str = "hearts"; break;
        case 'D': suit_str = "diamonds"; break;
        case 'C': suit_str = "clubs"; break;
        case 'S': suit_str = "spades"; break;
        default: free(filename); return NULL;
    }

    switch (value) {
        case 1: value_str = "ace"; break;
        case 11: value_str = "jack"; break;
        case 12: value_str = "queen"; break;
        case 13: value_str = "king"; break;
        
        default:
            snprintf(filename, 256, "PNG-cards/%d_of_%s.png", value, suit_str);
            return filename;
    }

    snprintf(filename, 256, "PNG-cards/%s_of_%s.png", value_str, suit_str);
    return filename;
}

void update_blackjack_card_images(card hand[], int handLength, int playersTurn, int handValue) {
    

    if (playersTurn == 1 || playersTurn == 3 || playersTurn == -1) {
        global_hand_value = handValue; // Update the global hand value
        for (int i = 0; i < 10; i++) {
            if (player_cards_pixbuf[i]) {
                g_object_unref(player_cards_pixbuf[i]);
                player_cards_pixbuf[i] = NULL;
            }
        }
        for (int i = 0; i < handLength; i++) {
            const char* card_filename = get_card_filename(hand[i].suit, hand[i].value);
            if (card_filename) {
                player_cards_pixbuf[i] = gdk_pixbuf_new_from_file_at_size(card_filename, 125, 181, NULL);
                free((char*)card_filename); // Free allocated memory
            }
        }
        
    } else if (playersTurn == 0) {
        // Display only the first two cards for the dealer
        for (int i = 0; i < 2; i++) {
            if (dealer_cards_pixbuf[i]) {
                g_object_unref(dealer_cards_pixbuf[i]);
                dealer_cards_pixbuf[i] = NULL;
            }
            const char* card_filename;
            if (hand[i].suit == 'X' && hand[i].value == 0) {
                card_filename = "PNG-cards/back.png";
            } else {
                card_filename = get_card_filename(hand[i].suit, hand[i].value);
            }
            if (card_filename) {
                dealer_cards_pixbuf[i] = gdk_pixbuf_new_from_file_at_size(card_filename, 125, 181, NULL);
                if (hand[i].suit != 'X' || hand[i].value != 0) {
                    free((char*)card_filename); // Free allocated memory only if it's not back.png
                }
            }
        }
    } else if (playersTurn == 2) {
        // Update the face-down card and add more cards to the dealer's hand
        for (int i = 0; i < handLength; i++) {
            const char* card_filename;
            if (i == 1) {
                card_filename = get_card_filename(hand[i].suit, hand[i].value);
            } else {
                card_filename = get_card_filename(hand[i].suit, hand[i].value);
            }
            if (dealer_cards_pixbuf[i]) {
                g_object_unref(dealer_cards_pixbuf[i]);
                dealer_cards_pixbuf[i] = NULL;
            }
            if (card_filename) {
                dealer_cards_pixbuf[i] = gdk_pixbuf_new_from_file_at_size(card_filename, 125, 181, NULL);
                free((char*)card_filename); // Free allocated memory
            }
        }
    }
    gtk_widget_queue_draw(drawing_area); // Redraw the drawing area
}





void update_card_images(char c1_suit, int c1_value, char c2_suit, int c2_value, 
                        char fl1_suit, int fl1_value, char fl2_suit, int fl2_value, char fl3_suit, int fl3_value,
                        char t_suit, int t_value, char r_suit, int r_value) {
    const char* card1_filename = get_card_filename(c1_suit, c1_value);
    const char* card2_filename = get_card_filename(c2_suit, c2_value);
    const char* flop1_filename = (fl1_suit == 'X' && fl1_value == 0) ? "PNG-cards/back.png" : get_card_filename(fl1_suit, fl1_value);
    const char* flop2_filename = (fl2_suit == 'X' && fl2_value == 0) ? "PNG-cards/back.png" : get_card_filename(fl2_suit, fl2_value);
    const char* flop3_filename = (fl3_suit == 'X' && fl3_value == 0) ? "PNG-cards/back.png" : get_card_filename(fl3_suit, fl3_value);
    const char* turn_filename = (t_suit == 'X' && t_value == 0) ? "PNG-cards/back.png" : get_card_filename(t_suit, t_value);
    const char* river_filename = (r_suit == 'X' && r_value == 0) ? "PNG-cards/back.png" : get_card_filename(r_suit, r_value);

    if (card1_filename) {
        if (card1_pixbuf) g_object_unref(card1_pixbuf);
        card1_pixbuf = gdk_pixbuf_new_from_file_at_size(card1_filename, 125, 181, NULL);
        free((char*)card1_filename);  // Free allocated memory
    }

    if (card2_filename) {
        if (card2_pixbuf) g_object_unref(card2_pixbuf);
        card2_pixbuf = gdk_pixbuf_new_from_file_at_size(card2_filename, 125, 181, NULL);
        free((char*)card2_filename);  // Free allocated memory
    }

    if (flop1_filename && flop2_filename && flop3_filename) {
        if (flop1_pixbuf) g_object_unref(flop1_pixbuf);
        if (flop2_pixbuf) g_object_unref(flop2_pixbuf);
        if (flop3_pixbuf) g_object_unref(flop3_pixbuf);
        flop1_pixbuf = gdk_pixbuf_new_from_file_at_size(flop1_filename, 125, 181, NULL);
        flop2_pixbuf = gdk_pixbuf_new_from_file_at_size(flop2_filename, 125, 181, NULL);
        flop3_pixbuf = gdk_pixbuf_new_from_file_at_size(flop3_filename, 125, 181, NULL);
        if (fl1_suit != 'X' || fl1_value != 0) free((char*)flop1_filename);  // Free allocated memory if not back
        if (fl2_suit != 'X' || fl2_value != 0) free((char*)flop2_filename);  // Free allocated memory if not back
        if (fl3_suit != 'X' || fl3_value != 0) free((char*)flop3_filename);  // Free allocated memory if not back
    }

    if (turn_filename && river_filename) {
        if (turn_pixbuf) g_object_unref(turn_pixbuf);
        if (river_pixbuf) g_object_unref(river_pixbuf);
        turn_pixbuf = gdk_pixbuf_new_from_file_at_size(turn_filename, 125, 181, NULL);
        river_pixbuf = gdk_pixbuf_new_from_file_at_size(river_filename, 125, 181, NULL);
        if (t_suit != 'X' || t_value != 0) free((char*)turn_filename);  // Free allocated memory if not back
        if (r_suit != 'X' || r_value != 0) free((char*)river_filename);  // Free allocated memory if not back
    }

    gtk_widget_queue_draw(drawing_area);
}

void show_role(int pos){
    cairo_t *cr = gdk_cairo_create(drawing_area->window);
    int x = 0, y = 0;

    // Draw position images if p_pos is 0, 1, or 2
    GdkPixbuf *position_pixbuf = NULL;
    if (pos == 0) {
        position_pixbuf = dealer_pixbuf;
    } else if (pos == 1) {
        position_pixbuf = small_blind_pixbuf;
    } else if (pos == 2) {
        position_pixbuf = big_blind_pixbuf;
    }

    if (position_pixbuf) {
        int position_y = y - 60; // Above the player's cards
        int position_x = x + 62; // Centered above the cards
        gdk_cairo_set_source_pixbuf(cr, position_pixbuf, position_x, position_y);
        cairo_paint(cr);
    }
    cairo_destroy(cr);
}

gboolean update_timer(gpointer data) {
    if (timer_seconds > 0) {
        timer_seconds--;
        gtk_widget_queue_draw(drawing_area);
    } else {
        gtk_main_quit();
        return FALSE; // Quit the window and stop the timer        
    }
    return TRUE;
}

void reset_timer() {
    timer_seconds = 120;
    if (timer_id != 0) {
        g_source_remove(timer_id);
    }
    timer_id = g_timeout_add_seconds(1, update_timer, NULL);
}

gboolean on_socket_ready(GIOChannel *source, GIOCondition condition, gpointer data) {
    if (condition & G_IO_IN) {
        int n = read(sockfd, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("read failed");
            return FALSE; // Stop watching this source
        } else if (n == 0) {
            printf("Server closed the connection\n");
            return FALSE; // Stop watching this source
        } else {
            buffer[n] = '\0';
            printf("Received data: %s\n", buffer);

            if (strchr(buffer, ';')) {
                gtk_label_set_text(GTK_LABEL(status_label), "It is your round, please choose to CHECK,FOLD,or BET,(for BET, please press BET then enter how much you want to bet and press submit)");
                int pos, valid_input_int;
                char c1_suit, c2_suit, fl1_suit, fl2_suit, fl3_suit, t_suit, r_suit;
                int c1_value, c2_value, fl1_value, fl2_value, fl3_value, t_value, r_value;
                char name[50];
                int chips, bet_chips, bet_chips_min, pot;
                sscanf(buffer, "%d;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%c;%d;%d;%[^;];%d;%d;%d;%d",
                       &rounds, &pos, &c1_suit, &c1_value, &c2_suit, &c2_value, &fl1_suit, &fl1_value,
                       &fl2_suit, &fl2_value, &fl3_suit, &fl3_value, &t_suit, &t_value, &r_suit, &r_value,
                       &valid_input_int, name, &chips, &bet_chips, &bet_chips_min, &pot);

                player_pos = pos;  // Update player position

                update_card_images(c1_suit, c1_value, c2_suit, c2_value, fl1_suit, fl1_value, fl2_suit, fl2_value, fl3_suit, fl3_value, t_suit, t_value, r_suit, r_value);
                show_role(player_pos);// Display the player's role
                // Store the information for display
                snprintf(current_turn, sizeof(current_turn), "Turn: %s", name);
                current_chips = chips;
                current_bet_chips = bet_chips;
                current_bet_chips_min = bet_chips_min;
                current_pot = pot;

                gtk_widget_queue_draw(drawing_area);

                if (valid_input_int == 1) {
                    sprintf(read_message, "\n\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n",
                            rounds, pos, c1_suit, c1_value, c2_suit, c2_value, fl1_suit, fl1_value, fl2_suit, fl2_value, fl3_suit, fl3_value, t_suit, t_value, r_suit, r_value, name, chips, bet_chips, bet_chips_min, pot);
                } else {
                    sprintf(read_message, "\n\nInvalid input, try again!\nrounds: %d\npos:%d\nc1:%c%d c2:%c%d\nflop1: %c%d flop2: %c%d flop3: %c%d turn: %c%d river: %c%d\nPlayer %s, you have %d chips and have bet %d.\nPlayers bet to %d and pot is %d on the table\nWhat is your decision?[CHECK,BET,FOLD]\n\n",
                            rounds, pos, c1_suit, c1_value, c2_suit, c2_value, fl1_suit, fl1_value, fl2_suit, fl2_value, fl3_suit, fl3_value, t_suit, t_value, r_suit, r_value, name, chips, bet_chips, bet_chips_min, pot);
                }

                reset_timer(); // Reset the timer when a new message is received
            }
            else if (strchr(buffer, '*')) {
                reset_player_hands(); // Reset player hands at the start of a new game
                int player_num, initial_bet, pos;
                sscanf(buffer, "%d*%d*%d", &player_num, &initial_bet, &pos);
                sprintf(read_message, "The game is for %d players, minimum bet is %d and you position is %d. What is your name?", player_num, initial_bet, pos);
                num_players = player_num;
                show_role(pos); // show player role
                gtk_widget_queue_draw(drawing_area); // Trigger redraw of the drawing area
                TCP_write_wstring(sockfd, buffer, "Player read successfully");
            }
            else if (strstr(buffer,"What is your name?")){
                gtk_label_set_text(GTK_LABEL(status_label), buffer);
                strcpy(read_message,buffer);
            }
            else if (strstr(buffer,"What position do you want?")){
                gtk_label_set_text(GTK_LABEL(status_label), buffer);
                strcpy(read_message,buffer);
            }
            else if(strstr(buffer,"PLAYER_RESULT:")){
                strcpy(read_message,buffer);
                int p_pos;
                char p_c1_suit;
                int p_c1_value;
                char p_c2_suit;
                int p_c2_value;
                char p_name[50];
                int p_chips;
                sscanf(buffer,"PLAYER_RESULT: %d,%[^,],%c,%d,%c,%d,%d",&p_pos,p_name,&p_c1_suit,&p_c1_value,&p_c2_suit,&p_c2_value,&p_chips);
                printf("result: player %s at pos %d have cards: %c%d, %c%d and chips %d\n",p_name,p_pos,p_c1_suit,p_c1_value,p_c2_suit,p_c2_value,p_chips);
                
                // Store the player's hand in the array
                PlayerHand *hand = &player_hands[player_hand_count++];
                hand->pos = p_pos;
                strcpy(hand->name, p_name);
                hand->c1_suit = p_c1_suit;
                hand->c1_value = p_c1_value;
                hand->c2_suit = p_c2_suit;
                hand->c2_value = p_c2_value;
                hand->chip = p_chips;
                
                TCP_write_wstring(sockfd,buffer,"player read successfully");
            }
            else if (strstr(buffer,"How much do you want to bet?")){
                gtk_label_set_text(GTK_LABEL(status_label), buffer);
                strcpy(read_message,buffer);
            }
            else if(strstr(buffer,"UPDATE_COMPLETE:")){
                gtk_label_set_text(GTK_LABEL(status_label), "Waiting for other player action...");
                strcpy(read_message,buffer);
                char current_turn_local[50];
                int current_chips_local;
                int current_bet_chips_local;
                int current_bet_chips_min_local;
                int current_pot_local;
                sscanf(buffer,"UPDATE_COMPLETE: %[^,],%d,%d,%d,%d",current_turn_local,&current_chips_local,&current_bet_chips_local,&current_bet_chips_min_local,&current_pot_local);
                snprintf(current_turn, sizeof(current_turn), "Turn: %s", current_turn_local);
                current_chips = current_chips_local;
                current_bet_chips = current_bet_chips_local;
                current_bet_chips_min = current_bet_chips_min_local;
                current_pot = current_pot_local;
                TCP_write_wstring(sockfd, buffer, "Player read successfully");
                gtk_widget_queue_draw(drawing_area);
            }  
            else if (strstr(buffer, "QUIT or REMATCH?")) {
                gtk_label_set_text(GTK_LABEL(status_label), buffer);
                strcpy(read_message,buffer);
		//show_final_hands(); 

            }
            else if (strstr(buffer, "You are the winner.")) {
                gtk_label_set_text(GTK_LABEL(status_label), "You win!");
                strcpy(read_message, buffer);
                TCP_write_wstring(sockfd, buffer, "Player read successfully");
                show_final_hands(); // Show the final hands when the game ends

            }  
            else if (strstr(buffer, "Sorry you lose.")) {
                gtk_label_set_text(GTK_LABEL(status_label), "You lose");
                strcpy(read_message, buffer);
                TCP_write_wstring(sockfd, buffer, "Player read successfully");
                show_final_hands(); // Show the final hands when the game ends
            } 

            else if(strstr(buffer,"Continue? (Y or N)")){
                printf("%s\n",buffer);
                gtk_label_set_text(GTK_LABEL(status_label), buffer);
                strcpy(read_message, buffer);
                //TCP_write(sockfd,buffer);
        }
        else if(strstr(buffer,"GAMEMODE:")){
            sscanf(buffer,"GAMEMODE: %d", &gameMode);
            //printf("Game mode is %d\n", gameMode);
            strcpy(read_message, buffer);
            TCP_write_wstring(sockfd,buffer,"player received.");
        }
            else if(strchr(buffer, '#')){
                int handIndex = 0;
                int handLength, handValue;
                int playersTurn, valid_input;
                char handInfo[50], addCard[10];
                card hand[10];
                char label_msg[256];
                //card dealerHand[10], playerHand[10];

                sscanf(buffer, "%d#%d#%d#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d#%c#%d", &playersTurn, &handLength, &handValue, &valid_input,
                &hand[0].suit, &hand[0].value, &hand[1].suit, &hand[1].value, &hand[2].suit, &hand[2].value, &hand[3].suit, &hand[3].value, &hand[4].suit, &hand[4].value,
                &hand[5].suit, &hand[5].value, &hand[6].suit, &hand[6].value, &hand[7].suit, &hand[7].value, &hand[8].suit, &hand[8].value, &hand[9].suit, &hand[9].value);

                update_blackjack_card_images(hand, handLength, playersTurn, handValue);

                sprintf(handInfo, "%c%d, %c%d", hand[0].suit, hand[0].value, hand[1].suit, hand[1].value);
                handIndex += 2;
                while(handIndex < handLength){
                    sprintf(addCard, ", %c%d", hand[handIndex].suit, hand[handIndex].value);
                    strcat(handInfo, addCard);
                    handIndex++;
                }

                if(playersTurn == 0){   // players's turn, dealer hides one card, dealer handcard
                    printf("The Dealer's cards are: %s.\n", handInfo);
                    sprintf(label_msg,"The Dealer's cards are: %s.", handInfo);
                    gtk_label_set_text(GTK_LABEL(status_label), label_msg);
                    strcpy(read_message, buffer);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 1){  // player's turn, player shows all cards and makes decision, player handcard
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    strcpy(read_message, buffer);
                    if(valid_input != 1){
                        printf("Invalid input! Please make a decision (HIT or STAND): ");
                        gtk_label_set_text(GTK_LABEL(status_label), "Invalid input! Please make a decision (HIT or STAND): ");
                    }
                    else{
                        printf("Please make a decision (HIT or STAND): ");
                        gtk_label_set_text(GTK_LABEL(status_label), "Please make a decision (HIT or STAND): ");
                    } 
                    //TCP_write(sockfd, buffer);
                }
                else if(playersTurn == 2){  // dealer's turn, dealer shows all cards, dealer handcard
                    printf("The Dealer's cards are: %s. Their hand value is %d.\n", handInfo, handValue);
                    strcpy(read_message, buffer);
                    sprintf(label_msg,"The Dealer's cards are: %s. Their hand value is %d.", handInfo, handValue);
                    gtk_label_set_text(GTK_LABEL(status_label), label_msg);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == 3){  // Blackjack, player handcard
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    printf("Blackjack!\n");
                    sprintf(label_msg,"Your cards are: %s. Your hand value is %d.Blackjack", handInfo, handValue);
                    gtk_label_set_text(GTK_LABEL(status_label), label_msg);
                    strcpy(read_message, buffer);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
                else if(playersTurn == -1){ // player's turn, player busted, player handcard
                    printf("Your cards are: %s. Your hand value is %d.\n", handInfo, handValue);
                    sprintf(label_msg,"Your cards are: %s. Your hand value is %d.", handInfo, handValue);
                    gtk_label_set_text(GTK_LABEL(status_label), label_msg);
                    strcpy(read_message, buffer);
                    TCP_write_wstring(sockfd,buffer,"player received.");
                }
            }

            else {
                gtk_label_set_text(GTK_LABEL(status_label), "Waiting for other player action...");
                strcpy(read_message, buffer);
                TCP_write_wstring(sockfd, buffer, "Player read successfully");
            }

            strcpy(print_message3, print_message2);
            strcpy(print_message2, print_message1);
            strcpy(print_message1, read_message);
            printf("print_msg3: %s\n", print_message3);
            printf("print_msg2: %s\n", print_message2);
            printf("print_msg1: %s\n", print_message1);

            reset_timer(); // Reset the timer when a new message is received
        }
    }
    return TRUE; // Continue watching this source
}

void on_button_clicked(GtkButton *button, gpointer user_data) {
    const gchar *gtk_text = gtk_entry_get_text(GTK_ENTRY(entry));

    // Make a mutable copy of the text
    char *text = g_strdup(gtk_text);
    
    TCP_write_wstring(sockfd, buffer, text);

    // Print the content of the entry
    printf("Entry content: %s\n", text);

    if (!name_entered) {
        name_entered = true;
        gtk_label_set_text(GTK_LABEL(status_label), "Waiting for other player's action.");
        gtk_widget_set_sensitive(check_button, TRUE);
        gtk_widget_set_sensitive(bet_button, TRUE);
        gtk_widget_set_sensitive(fold_button, TRUE);
        gtk_widget_set_sensitive(hit_button, TRUE);
        gtk_widget_set_sensitive(stand_button, TRUE);
    }

    gtk_entry_set_text(GTK_ENTRY(entry), "");

    // Free the copied string when done
    g_free(text);
}

void on_action_button_clicked(GtkButton *button, gpointer user_data) {
    char *action = (char *)user_data;
    TCP_write_wstring(sockfd, buffer, action);
    printf("Action: %s\n", action);

    if (strcmp(action, "BET") == 0) {
        gtk_label_set_text(GTK_LABEL(status_label), "Enter your bet number:");
    }
}

void on_hit_button_clicked(GtkButton *button, gpointer user_data) {
    char *action = "HIT";
    TCP_write_wstring(sockfd, buffer, action);
    printf("Action: %s\n", action);
}

void on_stand_button_clicked(GtkButton *button, gpointer user_data) {
    char *action = "STAND";
    TCP_write_wstring(sockfd, buffer, action);
    printf("Action: %s\n", action);
}

gboolean on_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);
    int widget_width = widget->allocation.width;
    int widget_height = widget->allocation.height;

    // Load background image
    GdkPixbuf *background = gdk_pixbuf_new_from_file("PNG-cards/background.png", NULL);
    if (background) {
        gdk_cairo_set_source_pixbuf(cr, background, 0, 0);
        cairo_paint(cr);
        g_object_unref(background);
    }

    if (gameMode == 1) {
        //Display sum of player's card values
        int player_card_sum = global_hand_value;

        // Draw the sum of player's card     
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White color
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 70);

        char card_sum_text[50];
        snprintf(card_sum_text, sizeof(card_sum_text), "%d", player_card_sum);
        cairo_move_to(cr, 80, widget_height - 300); // Position to the left of player's cards
        cairo_show_text(cr, card_sum_text);

        // Draw the player's cards at the bottom
        int card_y = widget_height - 181 - 10; // 10 pixels padding from the bottom
        for (int i = 0; i < 10 && player_cards_pixbuf[i]; i++) {
            int card_x = (widget_width / 2) - (4 * 125) + (i * 130); // Centered with padding between cards
            gdk_cairo_set_source_pixbuf(cr, player_cards_pixbuf[i], card_x, card_y);
            cairo_paint(cr);
        }

        // Draw the dealer's cards at the top
        card_y = 10; // 10 pixels padding from the top
        for (int i = 0; i < 10 && dealer_cards_pixbuf[i]; i++) {
            int card_x = (widget_width / 2) - (4 * 125) + (i * 130); // Centered with padding between cards
            gdk_cairo_set_source_pixbuf(cr, dealer_cards_pixbuf[i], card_x, card_y);
            cairo_paint(cr);
        }
        
    }


    // Load back image for placeholder
    if (!back_pixbuf) {
        back_pixbuf = gdk_pixbuf_new_from_file_at_size("PNG-cards/back.png", 125, 181, NULL);
    }

    // Load position images if not already loaded
    if (!dealer_pixbuf) {
        dealer_pixbuf = gdk_pixbuf_new_from_file_at_size("PNG-cards/dealer.png", 50, 50, NULL);
    }
    if (!small_blind_pixbuf) {
        small_blind_pixbuf = gdk_pixbuf_new_from_file_at_size("PNG-cards/small_blind.png", 50, 50, NULL);
    }
    if (!big_blind_pixbuf) {
        big_blind_pixbuf = gdk_pixbuf_new_from_file_at_size("PNG-cards/big_blind.png", 50, 50, NULL);
    }

    // Draw player positions based on the number of players
    if (num_players > 1) {
        // Top middle
        int x = (widget_width - 250) / 2;
        int y = 0;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 2) {
        // Left middle
        int x = 0;
        int y = (widget_height - (2 * 181)) / 2 + 150;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 3) {
        // Right middle
        int x = widget_width - 250;
        int y = (widget_height - (2 * 181)) / 2 + 150;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 4) {
        // Top right (moved 100 pixels toward center)
        int x = widget_width - 250 - 100;
        int y = 0;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 5) {
        // Top left (moved 100 pixels toward center)
        int x = 100;
        int y = 0;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 6) {
        // Left top
        int x = 0;
        int y = (widget_height - (3 * 181)) / 4 + 100;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 7) {
        // Right top
        int x = widget_width - 250;
        int y = (widget_height - (3 * 181)) / 4 + 100;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players > 8) {
        // Left bottom
        int x = 0;
        int y = 3 * (widget_height - 181) / 4 + 125;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }
    if (num_players == 10) {
        // Right bottom
        int x = widget_width - 250;
        int y = 3 * (widget_height - 181) / 4 + 125;
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x, y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, back_pixbuf, x + 125, y);
        cairo_paint(cr);
    }

    // Draw the flop, turn, and river cards in the middle
    if (flop1_pixbuf && flop2_pixbuf && flop3_pixbuf && turn_pixbuf && river_pixbuf) {
        int card_y = (widget_height - 181) / 2; // Center vertically
        int flop_x1 = (widget_width / 2) - 312; // Centered with 5 pixels padding between cards
        int flop_x2 = (widget_width / 2) - 187;
        int flop_x3 = (widget_width / 2) - 62;
        int turn_x = (widget_width / 2) + 62;
        int river_x = (widget_width / 2) + 187;

        gdk_cairo_set_source_pixbuf(cr, flop1_pixbuf, flop_x1, card_y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, flop2_pixbuf, flop_x2, card_y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, flop3_pixbuf, flop_x3, card_y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, turn_pixbuf, turn_x, card_y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, river_pixbuf, river_x, card_y);
        cairo_paint(cr);
    }

    // Draw the player's cards at the bottom
    if (card1_pixbuf && card2_pixbuf) {
        int card_y = widget_height - 181 - 10; // 10 pixels padding from the bottom
        int card_x1 = (widget_width / 2) - 125 - 5; // Centered with 5 pixels padding between cards
        int card_x2 = (widget_width / 2) + 5;

        gdk_cairo_set_source_pixbuf(cr, card1_pixbuf, card_x1, card_y);
        cairo_paint(cr);
        gdk_cairo_set_source_pixbuf(cr, card2_pixbuf, card_x2, card_y);
        cairo_paint(cr);

        // Draw position images if player_pos is 0, 1, or 2
        GdkPixbuf *position_pixbuf = NULL;
        if (player_pos == 0) {
            position_pixbuf = dealer_pixbuf;
        } else if (player_pos == 1) {
            position_pixbuf = small_blind_pixbuf;
        } else if (player_pos == 2) {
            position_pixbuf = big_blind_pixbuf;
        }

        if (position_pixbuf) {
            int position_y = card_y - 60; // Above the player's cards
            int position_x = card_x1 + 62; // Centered above the cards
            gdk_cairo_set_source_pixbuf(cr, position_pixbuf, position_x, position_y);
            cairo_paint(cr);
        }

        // Draw yellow messages to the left of user's card
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0); // Yellow color
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 18);

        int text_x = card_x1 - 250; // Adjust as needed to position text
        int text_y = card_y;

        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, current_turn);

        char message[256];
        snprintf(message, sizeof(message), "Round: %d", rounds);  // Added round info
        cairo_move_to(cr, text_x, text_y + 25);
        cairo_show_text(cr, message);

        snprintf(message, sizeof(message), "Chips: %d", current_chips);
        cairo_move_to(cr, text_x, text_y + 50);
        cairo_show_text(cr, message);

        snprintf(message, sizeof(message), "Bet Chips: %d", current_bet_chips);
        cairo_move_to(cr, text_x, text_y + 75);
        cairo_show_text(cr, message);

        snprintf(message, sizeof(message), "Min Bet: %d", current_bet_chips_min);
        cairo_move_to(cr, text_x, text_y + 100);
        cairo_show_text(cr, message);

        snprintf(message, sizeof(message), "Pot: %d", current_pot);
        cairo_move_to(cr, text_x, text_y + 125);
        cairo_show_text(cr, message);
    }

    // Draw the timer at the top left corner
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White color for the timer
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);

    int minutes = timer_seconds / 60;
    int seconds = timer_seconds % 60;
    char timer_text[10];
    snprintf(timer_text, sizeof(timer_text), "%02d:%02d", minutes, seconds);

    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, timer_text);

    cairo_destroy(cr);
    return TRUE;
}

void on_resolution_changed(GtkComboBox *combo, gpointer user_data) {
    const gchar *resolution = gtk_combo_box_get_active_text(combo);
    int width, height;
    if (sscanf(resolution, "%dx%d", &width, &height) == 2) {
        gtk_window_resize(GTK_WINDOW(user_data), width, height);
        gtk_widget_set_size_request(drawing_area, width, height);
        gtk_widget_queue_draw(drawing_area);
    }
}

void show_final_hands() {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *table;

    dialog = gtk_dialog_new_with_buttons("Final Hands",
                                         NULL,
                                         GTK_DIALOG_MODAL,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_OK,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    table = gtk_table_new(player_hand_count, 4, FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), table);

    for (int i = 0; i < player_hand_count; ++i) {
        PlayerHand *hand = &player_hands[i];

        GtkWidget *label = gtk_label_new(hand->name);
        gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, i, i + 1);

        const char *card1_filename = get_card_filename(hand->c1_suit, hand->c1_value);
        const char *card2_filename = get_card_filename(hand->c2_suit, hand->c2_value);

        if (card1_filename) {
            GdkPixbuf *card1_pixbuf = gdk_pixbuf_new_from_file_at_size(card1_filename, 125, 181, NULL);
            GtkWidget *card1_image = gtk_image_new_from_pixbuf(card1_pixbuf);
            gtk_table_attach_defaults(GTK_TABLE(table), card1_image, 1, 2, i, i + 1);
            g_object_unref(card1_pixbuf);
            free((char*)card1_filename);
        }

        if (card2_filename) {
            GdkPixbuf *card2_pixbuf = gdk_pixbuf_new_from_file_at_size(card2_filename, 125, 181, NULL);
            GtkWidget *card2_image = gtk_image_new_from_pixbuf(card2_pixbuf);
            gtk_table_attach_defaults(GTK_TABLE(table), card2_image, 2, 3, i, i + 1);
            g_object_unref(card2_pixbuf);
            free((char*)card2_filename);
        }

        char chip_text[50];
        snprintf(chip_text, sizeof(chip_text), "Score: %d", hand->chip);
        GtkWidget *chip_label = gtk_label_new(chip_text);
        gtk_table_attach_defaults(GTK_TABLE(table), chip_label, 3, 4, i, i + 1);
    }

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void reset_player_hands() {
    player_hand_count = 0;
    memset(player_hands, 0, sizeof(player_hands));
}

int main(int argc, char *argv[]) {
    #ifndef DEBUG
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

    TCP_client_create(sockfd, portno, serv_addr, server);
    printf("You have connected to the server, waiting for other players\n");

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create a new window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // Set the window title
    gtk_window_set_title(GTK_WINDOW(window), "Poker_client");
    
    // Set the window's default size and make it non-resizable
    gtk_window_set_default_size(GTK_WINDOW(window), 1800, 1200);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    
    // Set a handler for delete_event that immediately exits GTK.
    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box to hold the drawing area, button, and entry
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);

    // Create a horizontal box for the resolution combobox
    GtkWidget *resolution_hbox = gtk_hbox_new(FALSE, 5);

    // Add a label to the resolution hbox
    GtkWidget *resolution_label = gtk_label_new("Resolution:");
    gtk_box_pack_start(GTK_BOX(resolution_hbox), resolution_label, FALSE, FALSE, 0);

    // Create the resolution combobox
    GtkWidget *resolution_combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "900x600");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "1200x800");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "1500x1000");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "1800x1200");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "2100x1400");
    gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_combo), "2400x1600");

    g_signal_connect(resolution_combo, "changed", G_CALLBACK(on_resolution_changed), window);
    gtk_box_pack_start(GTK_BOX(resolution_hbox), resolution_combo, FALSE, FALSE, 0);

    // Add the resolution hbox to the vbox
    gtk_box_pack_start(GTK_BOX(vbox), resolution_hbox, FALSE, FALSE, 0);

    // Create a drawing area
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1800, 1200);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 0);

    // Connect the draw signal
    g_signal_connect(drawing_area, "expose-event", G_CALLBACK(on_draw), NULL);

    // Create a status label
    status_label = gtk_label_new("Waiting for other player action...");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    // Create a horizontal box for the entry and button
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);

    // Create a new button with label "Quit"
    GtkWidget *quit_button = gtk_button_new_with_label("Quit");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), quit_button, FALSE, FALSE, 0);

    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    GtkWidget *send_button = gtk_button_new_with_label("Submit");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), send_button, FALSE, FALSE, 0);

    // Add the hbox to the vbox
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Create a horizontal box for action buttons and blackjack action buttons
    GtkWidget *action_hbox = gtk_hbox_new(FALSE, 5);
    GtkWidget *blackjack_action_hbox = gtk_hbox_new(FALSE, 5);
    GtkWidget *hbox_main = gtk_hbox_new(FALSE, 5); // New main horizontal box

    check_button = gtk_button_new_with_label("CHECK");
    g_signal_connect(check_button, "clicked", G_CALLBACK(on_action_button_clicked), (gpointer)"CHECK");
    gtk_box_pack_start(GTK_BOX(action_hbox), check_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(check_button, FALSE);

    bet_button = gtk_button_new_with_label("BET");
    g_signal_connect(bet_button, "clicked", G_CALLBACK(on_action_button_clicked), (gpointer)"BET");
    gtk_box_pack_start(GTK_BOX(action_hbox), bet_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(bet_button, FALSE);

    fold_button = gtk_button_new_with_label("FOLD");
    g_signal_connect(fold_button, "clicked", G_CALLBACK(on_action_button_clicked), (gpointer)"FOLD");
    gtk_box_pack_start(GTK_BOX(action_hbox), fold_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(fold_button, FALSE);

    hit_button = gtk_button_new_with_label("HIT");
    g_signal_connect(hit_button, "clicked", G_CALLBACK(on_hit_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(blackjack_action_hbox), hit_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(hit_button, FALSE);

    stand_button = gtk_button_new_with_label("STAND");
    g_signal_connect(stand_button, "clicked", G_CALLBACK(on_stand_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(blackjack_action_hbox), stand_button, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(stand_button, FALSE);

    // Add the action_hbox and blackjack_action_hbox to hbox_main
    gtk_box_pack_start(GTK_BOX(hbox_main), action_hbox, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(hbox_main), blackjack_action_hbox, FALSE, FALSE, 0); // Pack to the end

    // Add the hbox_main to the vbox
    gtk_box_pack_start(GTK_BOX(vbox), hbox_main, FALSE, FALSE, 0);

    // Add the vbox to the window
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Make sure that everything, window and vbox (including label and button), are visible
    gtk_widget_show_all(window);

    GIOChannel *channel = g_io_channel_unix_new(sockfd);
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)on_socket_ready, NULL);

    // Initialize and start the timer
    reset_timer();

    gtk_main();

    close(sockfd);
    return 0;
    #endif
    #ifdef DEBUG
    printf("This is client gui debug test.\n");
    gtk_init(&argc, &argv);

    GtkWidget *debug_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(debug_window), "Debug Test");
    gtk_window_set_default_size(GTK_WINDOW(debug_window), 1200, 800);
    gtk_window_set_resizable(GTK_WINDOW(debug_window), TRUE);
    g_signal_connect(debug_window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *debug_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(debug_drawing_area, 1200, 800);
    gtk_container_add(GTK_CONTAINER(debug_window), debug_drawing_area);

    g_signal_connect(debug_drawing_area, "expose-event", G_CALLBACK(on_debug_draw), NULL);

    gtk_widget_show_all(debug_window);
    gtk_main();
    return 0;
    #endif
}



gboolean on_debug_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    cairo_t *cr = gdk_cairo_create(widget->window);

    // Load background image
    GdkPixbuf *background = gdk_pixbuf_new_from_file("bin/PNG-cards/background.png", NULL);
    if (background) {
        gdk_cairo_set_source_pixbuf(cr, background, 0, 0);
        cairo_paint(cr);
        g_object_unref(background);
    }

    // Draw the text 
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // White color
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 400, 50);
    cairo_show_text(cr, "This is client gui debug test.");

    // Load and draw the black joker image
    GdkPixbuf *black_joker_pixbuf = gdk_pixbuf_new_from_file_at_size("bin/PNG-cards/black_joker.png", 125, 181, NULL);
    if (black_joker_pixbuf) {
        gdk_cairo_set_source_pixbuf(cr, black_joker_pixbuf, 300, 300);
        cairo_paint(cr);
        g_object_unref(black_joker_pixbuf);
    }

    // Load and draw the red joker image
    GdkPixbuf *red_joker_pixbuf = gdk_pixbuf_new_from_file_at_size("bin/PNG-cards/red_joker.png", 125, 181, NULL);
    if (red_joker_pixbuf) {
        gdk_cairo_set_source_pixbuf(cr, red_joker_pixbuf, 600, 300);
        cairo_paint(cr);
        g_object_unref(red_joker_pixbuf);
    }

    cairo_destroy(cr);
    return TRUE;
}
