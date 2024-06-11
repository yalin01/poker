Poker game made by team 20 King Bomb aka. Wang Zha
Version: Release V1.0
**Please note that this game works best in a resolution of 1800*1200

FEATURES:
Texas Hold'em game with customizable game size and minumum bet
    Classic rules and intuitive experience
    Real-time multiplayer game
    Detailed GUI
    Seating options
    Bot that plays automatically like a human
    Timer that forces quit after timeout
Blackjack game
    Basic rules and intuitive experience
    Play against computer as the dealer
    Detailed GUI


INSTALL:
See INSTALL.txt

TESTING:
1. use "make test"
2. use "./poker_comms_server <port number>" to start the communication testing server
3. use "./client_comms <server name> <port number>" to start the communication testing client
4. use "make test_gui" to start the testing for GUI
5. use "make test_server" to start the testing for server
6. use "make test_client" to start the testing for client

TEST_COMMS & TEST_COMMS_SERVER:
NOTE: the default port number is 21500, the server name is laguna, minimum client number is 1



PLAYING:

Server:In bin folder, 
        use "./poker_server <port number> Texas <player number> <minimum bet>" to start the Texas Hold'Em server
        use "./poker_server <port number> Blackjack 1 <minimum bet>" to start the Blackjack server
Client: In bin folder, 
        use "./poker_client <server name> <port number>" to start the client
        if GUI is unavailable, use "./poker_client_term <server name> <port number>" to start the client for Blackjack
Bot:    use "./poker_bot <server name> <port number> <difficulty>" to start the client, for <difficulty>, 1 = easy, 2 = hard


AUTHORS:
The names of the autors are listed by alphabetical order of last names, not by contribution.
    Yixuan  Jing    yixuanj9@uci.edu
	Yian    Lin		yianl8@uci.edu
	Thomas  Yeung   yeungks@uci.edu
	Tangqin Zhu		tangqinz@uci.edu
