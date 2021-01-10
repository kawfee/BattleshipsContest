/*
    Authors: Joey Gorski, Matthew Bouch
    Battleships Server

    Used resources:
    Code help for using and setting up sockets
        https://simpledevcode.wordpress.com/2016/06/16/client-server-chat-in-c-using-sockets/
        https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
        https://github.com/amnonpaz/unix_sockets

    Github libraries:
        https://github.com/kawfee/BattleshipsContest
        https://github.com/nlohmann/json
        https://github.com/arun11299/cpp-subprocess
*/


#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <iostream>
#include <fstream>
#include <cstring>

#include "json.hpp"
#include "subprocess.hpp"

#include "defines.h"
#include "socket_defs.h"

#define TRUE   1
#define FALSE  0
#define PORT 54321

using namespace std;
using json = nlohmann::json;
using namespace subprocess;


int bind_address(int sock, const char *path);
int create_socket(const char *path);
void destroy_socket(int sock, const char *path);
void setupServer(int &max_clients, int (&client_socket)[30], int &sd, int &master_socket, int &opt, sockaddr_un &address, int &i, int &addrlen, const char *path);
void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &countConnected);
int masterSocketConnection(fd_set &readfds, int &master_socket, int &max_sd, int &activity, int &new_socket, sockaddr_un &address,
            int &addrlen, int &max_clients, int (&client_socket)[30], int &i, int &countConnected);
void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_un &address, int &addrlen, int &max_clients,
            int (&client_socket)[30], int &i, int &countConnected);
void childDisconnect(int &sd, sockaddr_un &address, int &addrlen, int (&client_socket)[30], int &dConnect);
void childParse(string &clientStr, json &clientResponse, int &valread, char (&buffer)[1500]);
void printAll(int sd, string clientStr, json msg, int boardSize, char c1Board[10][10], char c2Board[10][10]);
void logAll(int boardSize, char c1Board[10][10], char c2Board[10][10], Player player1, Player player2, ofstream &log_stream, json msg1, json msg2);
bool performAction(string messageType, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int (&shipLengths)[6], char (&buffer)[1500], int &activity, sockaddr_un address, int &addrlen, int (&client_socket)[30],
            int &dConnect, Popen &c, int &valread, string &clientStr, json &clientResponse, string currentClient, char c1Board[10][10],
            char c2Board[10][10], char c1ShipBoard[10][10], char c2ShipBoard[10][10], int &boardSize, int totalGameRound,
            json (&c1Ships)[6], json (&c2Ships)[6]);
bool placeShip(char board[10][10] , char shipBoard[10][10], int boardSize, int row, int col, int length, Direction dir, json &msg, json (&ships)[6]);
char shootShot(char board[10][10] , int boardSize, int row, int col);
int findDeadShip(int numShips, json (&ships)[6], json &msg, char board[10][10]);
int gameOver(json (&c1Ships)[6], json (&c2Ships)[6]);
bool sendReceive(Player &player, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int &activity, sockaddr_un address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread);
GameInfo runMatch(Player player1, Player player2, int boardSize, int master_socket, int addrlen, int (&client_socket)[30], int activity, int valread, int sd,
    int max_sd, int dConnect, int countConnected, sockaddr_un address, fd_set readfds, Popen &c1, Popen &c2, int numGames, int matchNum, ofstream &log_stream);



int runGame(int numGames, Player &player1, Player &player2, int boardSize, string matchFile){
    //setup socket functionality
    //create the server variables
    const char *path = "./serversocket";
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients=2 , activity, i , valread=1 , sd=0, max_sd,
        dConnect=0, countConnected=0;
    //struct sockaddr_un address;
    struct sockaddr_un  address;
    //set of socket descriptors
    fd_set readfds;

    json gameMsg = {
        {"messageType", "setupGame"},
        {"row", -1},
        {"col", -1},
        {"str", to_string(boardSize)},
        {"dir", NONE},
        {"length", -1},
        {"client", "none"},
        {"count", 0}
    };

    setupServer(max_clients, client_socket, sd, master_socket, opt, address, i, addrlen, path);

    auto c1 = Popen({"./AI_Executables/"+player1.name});
    masterSocketConnection(readfds, master_socket, max_sd, activity,
        new_socket, address, addrlen,
        max_clients, client_socket, i, countConnected);

    auto c2 = Popen({"./AI_Executables/"+player2.name});
    masterSocketConnection(readfds, master_socket, max_sd, activity,
        new_socket, address, addrlen,
        max_clients, client_socket, i, countConnected);

    sendReceive(player1, readfds, master_socket, max_sd, client_socket[0], countConnected, gameMsg,
        activity, address, addrlen, client_socket, dConnect,
        c1, valread);


    sendReceive(player2, readfds, master_socket, max_sd, client_socket[1], countConnected, gameMsg,
        activity, address, addrlen, client_socket, dConnect,
        c2, valread);        

    
    ofstream log_stream("./logs/" + matchFile);


    // Starting game stuff
    for(int i = 0; i <  numGames; i++){
        GameInfo match = runMatch(player1, player2, boardSize, master_socket, addrlen, client_socket, activity, valread, sd,
            max_sd, dConnect, countConnected, address, readfds, c1, c2, numGames, i, log_stream);
        

        if(match.error == true){
            log_stream << "MATCH_OVER" << endl;

            c1.kill();
            c2.kill();
            childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
            childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);
            
            return -1; // or a break maybe? change as necessary we had this when giving the AI only one chance.
        }

        // string name;
        // string author;
        // int wins;
        // int ties;
        // int losses;

        /* cout << endl << endl
             << "MATCH DATA: " << endl
             << "\t" << "ERROR: " << match.error << endl
             << "\t" << "PLAYER ONE: " << endl
                << "\t\tWINS: " << match.player1.wins << endl
                << "\t\tTIES: " << match.player1.ties << endl
                << "\t\tLOSSES: " << match.player1.losses << endl
             << "\t" << "PLAYER TWO: " << endl
                << "\t\tWINS: " << match.player2.wins << endl
                << "\t\tTIES: " << match.player2.ties << endl
                << "\t\tLOSSES: " << match.player2.losses << endl
             << endl << endl; */

        player1=match.player1;
        player2=match.player2;
    }

    log_stream << player1.name << "'s WIN-LOSS-TIE: " << player1.wins << "-" << player1.losses << "-" << player1.ties << endl;
    log_stream << player2.name << "'s WIN-LOSS-TIE: " << player2.wins << "-" << player2.losses << "-" << player2.ties << endl;

    log_stream.close();

    c1.kill();
    c2.kill();
    childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
    childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);

    destroy_socket(master_socket, path);


    return 1;
}


GameInfo runMatch(Player player1, Player player2, int boardSize, int master_socket, int addrlen, int (&client_socket)[30], int activity, int valread, int sd,
    int max_sd, int dConnect, int countConnected, sockaddr_un address, fd_set readfds, Popen &c1, Popen &c2, int numGames, int matchNum, ofstream &log_stream){
    GameInfo temp;
    temp.player1 = player1;
    temp.player2 = player2;
    temp.error=false;
    
    bool logging;
    if(matchNum < 25){
        logging = true;
    }
    else if ((numGames - matchNum) <= 25){
        logging = true;
    }
    else {
        logging = false;
    }
    if(logging){
        log_stream << "MATCH_START_ROUND: " << matchNum << endl;
    }

    int totalGameRound=1;
    

    char buffer[1500];
    string clientStr, currentClient;

    //result variables when calling game functions for error checking
    bool p1Result, p2Result;

    json msg = {
        {"messageType", "placeShip"},
        {"row", -1},
        {"col", -1},
        {"str", ""},
        {"dir", NONE},
        {"length", 3},
        {"client", "none"},
        {"count", 0}
    };
    json clientResponse;
    json savedMsg;


    //create the game variables here
    int shipLengths[] = { 3,3,4,3,3,4 };
    int numShips = boardSize-2;
    if(numShips>6){
        numShips=6;
    }
    char c1ShipBoard[10][10];
    char c2ShipBoard[10][10];
    char c1Board[10][10];
    char c2Board[10][10];
    json c1Ships[6];
    json c2Ships[6];

    //populate the boards
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            c1Board[row][col] = WATER;
            c2Board[row][col] = WATER;
            c1ShipBoard[row][col] = WATER;
            c2ShipBoard[row][col] = WATER;
        }
    }

    while(TRUE){

        // plan for the future
        //     p1Result = placeShip();
        //     p2Result = placeShip();
        //     if p1 and p2 == false -1 ...:
        //         both lose
        //     if p1==true and p2==false:
        //         killRemaining(p1Result);
        

        // Note:
        //     -Logic that deals with either placing a ship or shooting a shot depending on game round through
        //     the use of the performAction function which does both a write and a read over our sockets.

        //     -The first string in performAction makes the decision for performAction of what action to take.

        //     -findDeadShip is suspect in its workability--give it a once-over before starting to work on the rest.

        
        if(totalGameRound <= 6){
            // for the sd for performAction, pass client_socket[0] or client_socket[1] seperately
            p1Result = performAction("placeShip", readfds, master_socket, max_sd, client_socket[0],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c1,
                valread, clientStr, clientResponse, "client1",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);
            p2Result = performAction("placeShip", readfds, master_socket, max_sd, client_socket[1],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c2,
                valread, clientStr, clientResponse, "client2",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);
            
            //do error checking
            if(!p1Result && !p2Result){
                // return no winner
                cout << "BOTH AI CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(p1Result && !p2Result){
                // return p1 as winner
                cout << "p2 CRASHED!" << endl;
                temp.player1.wins += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(!p1Result && p2Result){
                // return p2 as winner
                cout << "p1 CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.wins += 1;
                temp.error = true;
                return temp;
            }

        }else{
            p1Result = performAction("shootShot", readfds, master_socket, max_sd, client_socket[0],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c1,
                valread, clientStr, clientResponse, "client1",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);

            //json* savedMsg = new json(*msg);
            // Just in case we need a deep copy
            savedMsg = {
                {"messageType", msg.at("messageType")},
                {"row", msg.at("row")},
                {"col", msg.at("col")},
                {"str", msg.at("str")},
                {"dir", msg.at("dir")},
                {"length", msg.at("length")},
                {"client", msg.at("client")},
                {"count", msg.at("count")}
            }; 
                
            p2Result = performAction("shootShot", readfds, master_socket, max_sd, client_socket[1],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c2,
                valread, clientStr, clientResponse, "client2",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);

            //do error checking
            if(!p1Result && !p2Result){
                // return no winner
                cout << "BOTH AI CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(p1Result && !p2Result){
                // return p1 as winner
                cout << "p2 CRASHED!" << endl;
                temp.player1.wins += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(!p1Result && p2Result){
                // return p2 as winner
                cout << "p1 CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.wins += 1;
                temp.error = true;
                return temp;
            }

            
            //do dead ship checking 
            json deadShipMsg = {
                {"messageType", msg.at("messageType")},
                {"row", msg.at("row")},
                {"col", msg.at("col")},
                {"str", msg.at("str")},
                {"dir", msg.at("dir")},
                {"length", msg.at("length")},
                {"client", msg.at("client")},
                {"count", msg.at("count")}
            }; 

            int p1DeadShip = findDeadShip(numShips, c1Ships, deadShipMsg, c1Board);
            if(p1DeadShip>=0){
                json currShip = c1Ships[p1DeadShip];

                p1Result = performAction("shipDied", readfds, master_socket, max_sd, client_socket[0],
                    countConnected, currShip, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c1,
                    valread, clientStr, clientResponse, "client1",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);
                    
                p2Result = performAction("killedShip", readfds, master_socket, max_sd, client_socket[1],
                    countConnected, currShip, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c2,
                    valread, clientStr, clientResponse, "client2",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);
            }

            int p2DeadShip = findDeadShip(numShips, c2Ships, deadShipMsg, c2Board);
            if(p2DeadShip>=0){
                json currShip = c2Ships[p2DeadShip];

                p1Result = performAction("killedShip", readfds, master_socket, max_sd, client_socket[0],
                    countConnected, currShip, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c1,
                    valread, clientStr, clientResponse, "client1",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);

                p2Result = performAction("shipDied", readfds, master_socket, max_sd, client_socket[1],
                    countConnected, currShip, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c2,
                    valread, clientStr, clientResponse, "client2",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);
            }
            
            //do error checking part two, electric baloogaloo
            if(!p1Result && !p2Result){
                // return no winner
                cout << "BOTH AI CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(p1Result && !p2Result){
                // return p1 as winner
                cout << "p2 CRASHED!" << endl;
                temp.player1.wins += 1;
                temp.player2.losses += 1;
                temp.error = true;
                return temp;
            }
            else if(!p1Result && p2Result){
                // return p2 as winner
                cout << "p1 CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.wins += 1;
                temp.error = true;
                return temp;
            }
        }
        
        //log_stream << "Both AI did a move!" << endl;
        if(totalGameRound > 6 && logging){
            logAll(boardSize, c1Board, c2Board, player1, player2, log_stream, savedMsg, msg);
        }

        if(totalGameRound > 6){
            int gameStatus = gameOver(c1Ships, c2Ships);

            if(gameStatus>=0){
                // perform game over logic
                // modify struct data
                if(gameStatus==0){
                    temp.player1.ties += 1;
                    temp.player2.ties += 1;
                }else if(gameStatus==1){
                    temp.player1.wins += 1;
                    temp.player2.losses += 1;
                }else if(gameStatus==2){
                    temp.player1.losses += 1;
                    temp.player2.wins += 1;
                }

                p1Result = performAction("matchOver", readfds, master_socket, max_sd, client_socket[0],
                    countConnected, msg, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c1,
                    valread, clientStr, clientResponse, "client1",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);
                p2Result = performAction("matchOver", readfds, master_socket, max_sd, client_socket[1],
                    countConnected, msg, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c2,
                    valread, clientStr, clientResponse, "client2",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships);

                if(logging){
                    log_stream << "MATCH_OVER" << endl;
                }
                
                return temp;
            } 
        }
        
        //this is the maximum number of turns in a game--anything past that and both AI eat a loss
        if(totalGameRound >= (boardSize*boardSize*2) ){
            temp.player1.losses += 1;
            temp.player2.losses += 1;

            p1Result = performAction("matchOver", readfds, master_socket, max_sd, client_socket[0],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c1,
                valread, clientStr, clientResponse, "client1",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);
            p2Result = performAction("matchOver", readfds, master_socket, max_sd, client_socket[1],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c2,
                valread, clientStr, clientResponse, "client2",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);
            if(logging){
                log_stream << "MATCH_OVER" << endl;
            }

            return temp;
        }

        if(dConnect>=2){
            temp.error=true;
            return temp;
        }

        totalGameRound++;
    }

    temp.error=true;
    return temp;
}



















/**
 * bind_address: Bind a path to a socket
 *
 * Returns: Fail/Pass
 */
int bind_address(int sock, const char *path){
    struct sockaddr_un addr;

    memset(&addr, 0x00, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);

    return bind(sock, (struct sockaddr *)&addr, sizeof(addr));
}

/**
 * create_socket: Create a new Unix-domain soket,
 * clear the desired path and bind the socket to
 * the path.
 *
 * Returns: Fail/Pass
 */
int create_socket(const char *path){
    int res;

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        LOGE("Failed creating a new socket");
        return sock;
    }

    /* Clearing the path, in case the last 
       execution didn't terminate properly */
    unlink(path);

    res = bind_address(sock, path);
    if (res < 0) {
        LOGE("Failed binding socket to %s (%d: %s)",
             path, errno, strerror(errno));
        goto bind_error;
    }

    res = listen(sock, 2);
    if (res < 0) {
        LOGE("Failed listening on socket (%d: %s)", errno, strerror(errno));
        goto listen_error;
    }

    return sock;

    listen_error:
        unlink(path);
    bind_error:
        close(sock);

    return res;
}

/**
 * destroy_socket: Close the socket and clear
 * the path
 */
void destroy_socket(int sock, const char *path){
    close(sock);
    unlink(path);
}

void setupServer(int &max_clients, int (&client_socket)[30], int &sd, int &master_socket, int &opt, sockaddr_un &address, int &i, int &addrlen, const char *path){
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = -1;
    }

    //create a master socket
    master_socket = create_socket(path);

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    puts("Waiting for connections ...");
}

void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &countConnected){
    //clear the socket set
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    //socket descriptor

    //if valid socket descriptor then add to read list
    if(sd > 0){
        FD_SET( sd , &readfds);
    }

    //highest file descriptor number, need it for the select function
    if(sd > max_sd){
        max_sd = sd;
    }
}

int masterSocketConnection(fd_set &readfds, int &master_socket, int &max_sd, int &activity,
            int &new_socket, sockaddr_un &address, int &addrlen,
            int &max_clients, int (&client_socket)[30], int &i, int &countConnected){
    //clear the socket set
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed.
    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

    if(activity <= 0 && errno==EINTR){
        // failed connection to master socket.
        return activity;
    }

    //MASTER_SOCKET IS TOUCHED
    if (FD_ISSET(master_socket, &readfds)){
        //this adds new connections to the client_socket array when master socket is modified/accessed
        masterSocketTouched(new_socket, master_socket, address, addrlen, max_clients, client_socket, i, countConnected);
    }

    return 1;
}

void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_un &address, int &addrlen,
            int &max_clients, int (&client_socket)[30], int &i, int &countConnected){

    int t = sizeof(address);
    if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&t))<0){
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //inform user of socket number - used in send and receive commands
    printf("New connection , sid is %d\n", new_socket);


    //add new socket to array of sockets
    for (i = 0; i < max_clients; i++)
    {
        //if position is empty
        if( client_socket[i] == -1 )
        {
            client_socket[i] = new_socket;
            printf("Adding to list of sockets in position %d\n" , i);

            break;
        }
    }

    countConnected++;
}

void childDisconnect(int &sd, sockaddr_un &address, int &addrlen, int (&client_socket)[30], int &dConnect){
    getpeername(sd , NULL, NULL);
    //printf("Socket %d disconnected , ip %s , port %d \n", sd, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

    //Close the socket and mark as 0 in list for reuse
    close( sd);

    if(client_socket[0]==sd){
        client_socket[0] = (int)0;
    }else{
        client_socket[1] = (int)0;
    }

    dConnect++;
}

void childParse(string &clientStr, json &clientResponse, int &valread, char (&buffer)[1500]){
    //set the string terminating NULL byte on the end
    //of the data read
    buffer[valread] = '\0';

    clientStr = "";
    clientStr.append(buffer);
    clientResponse = json::parse(clientStr);
}

void printAll(int sd, string clientStr, json msg, int boardSize, char c1Board[10][10], char c2Board[10][10]){
    printf("---------BEGINNING OF PRINTALL---------\n");

    cout << "Test Print: \nsid = "
         << sd
         << " : \nbuffer = "
         << clientStr
         << " : \nmsg = \n"
         << msg.dump(4)
         << endl;

    printf("***** c1Board *****\n");
    cout << " 0123456789" << endl;
    for(int row=0;row<boardSize;row++){
        cout << row;
        for(int col=0;col<boardSize;col++){
            std::cout << c1Board[row][col];
        }
        cout << endl;
    }
    printf("*******************\n\n");
    printf("***** c2Board *****\n");
    cout << " 0123456789" << endl;
    for(int row=0;row<boardSize;row++){
        cout << row;
        for(int col=0;col<boardSize;col++){
            std::cout << c2Board[row][col];
        }
        cout << endl;
    }
    printf("*******************\n\n");
    printf("---------END OF PRINTALL---------\n");
}

void logAll(int boardSize, char c1Board[10][10], char c2Board[10][10], Player player1, Player player2, ofstream &log_stream, json msg1, json msg2){
    log_stream << player1.author << endl
               << player1.name << endl;
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            log_stream << c1Board[row][col];
        }
        log_stream << endl;
    }
    int p1Row = msg1.at("row");
    int p1Col = msg1.at("col");
    log_stream << player1.name << "'s "; 
    if (c2Board[p1Row][p1Col] == HIT || c2Board[p1Row][p1Col] == KILL){
        log_stream << "HIT: ";
    }else {
        log_stream << "MISS: ";
    }
    log_stream << p1Row << "," << p1Col << endl;
    
    log_stream << player2.author << endl
               << player2.name << endl;
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            log_stream << c2Board[row][col];
        }
        log_stream << endl;
    }
    int p2Row = msg2.at("row");
    int p2Col = msg2.at("col");
    log_stream << player2.name << "'s ";
    if(c1Board[p2Row][p2Col] == HIT || c1Board[p2Row][p2Col] == KILL){
        log_stream << "HIT: ";
    }else{
        log_stream << "MISS: ";
    }
    log_stream << p2Row << "," << p2Col << endl;
}

bool performAction(string messageType, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg, int (&shipLengths)[6],
            char (&buffer)[1500], int &activity, sockaddr_un address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread, string &clientStr, json &clientResponse, string currentClient, char c1Board[10][10],
            char c2Board[10][10], char c1ShipBoard[10][10], char c2ShipBoard[10][10], int &boardSize, int totalGameRound, json (&c1Ships)[6], json (&c2Ships)[6]){
    //prepare the sockets for the connection
    prepSockets(readfds, master_socket, max_sd, sd, countConnected);

    // send message by setting json data given in function call
    msg.at("messageType") = messageType;
    if(messageType == "placeShip"){
        msg.at("length") = shipLengths[totalGameRound-1];
    }
    strcpy(buffer, msg.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );

    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed and activity gets a value less then 0.
    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);
    //cout << "Current Client: " << currentClient << " Activity: " << activity << endl;


    // Handles a timeout error from the above select statement
    if ((activity <= 0) && (errno!=EINTR)){
        c.kill();
        printf("Child disconnected in activity\n");
        childDisconnect(sd, address, addrlen, client_socket, dConnect);
        return false;
    }

    // If everything worked when setting up sockets
    if(FD_ISSET(sd, &readfds)){
        // If/else here:
        //if: this means that read has failed and we shut things down. we notice this with valread.
        //else: everything went fine and all values are acceptable--begin the actual processing for the game.
        if ((valread = read( sd , buffer, 1499)) <= 0 || dConnect !=0){
            //get the details of the disconnected client and print them
            c.kill();
            printf("Child disconnected in valread\n");
            childDisconnect(sd, address, addrlen, client_socket, dConnect);
            return false;
        }else{
            //this else statement is where the data is processed for the game
            childParse(clientStr, clientResponse, valread, buffer);

            // Set json data here.
            msg.at("client") = clientResponse.at("client");
            msg.at("count") = clientResponse.at("count");
            msg.at("row") = clientResponse.at("row");
            msg.at("col") = clientResponse.at("col");
            msg.at("dir") = clientResponse.at("dir");
            msg.at("str") = clientResponse.at("str");


            /*
            Depending on which client we have passed to this function we go into their if statement and procede to call our
            different helper functions depending on which action we are going to perform.
            */
            
            char shotReturnValue = PLACE_SHIP;

            if(currentClient=="client1"){
                if(messageType=="placeShip"){
                    c1Ships[totalGameRound-1]=clientResponse;
                    return placeShip(c1Board, c1ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c1Ships);
                }else if(messageType=="shootShot"){
                    int tempRow = msg.at("row");
                    int tempCol = msg.at("col");
                    shotReturnValue = shootShot(c2Board, boardSize, tempRow, tempCol);
                }
            }else if(currentClient=="client2"){
                if(messageType=="placeShip"){
                    c2Ships[totalGameRound-1]=clientResponse;
                    return placeShip(c2Board, c2ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c2Ships);
                }else if(messageType=="shootShot"){
                    int tempRow = msg.at("row");
                    int tempCol = msg.at("col");
                    shotReturnValue = shootShot(c1Board, boardSize, tempRow, tempCol);
                }
            }
            
            if(messageType == "shootShot"){
                if(shotReturnValue==INVALID_SHOT){
                    //TODO deal with INVALID_SHOTs
                    return false;
                }
                string temp(1, shotReturnValue); // converts char to string of size 1
                msg.at("str") = temp;
                //communicate the shotReturnValue to both clients
                performAction("shotReturn", readfds, master_socket, max_sd, client_socket[0],
                    countConnected, msg, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c,
                    valread, clientStr, clientResponse, "client1",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships); // we have c in this function call bad for now

                string temp2(1, shotReturnValue); // converts char to string of size 1
                msg.at("str") = temp2;
                performAction("shotReturn", readfds, master_socket, max_sd, client_socket[1],
                    countConnected, msg, shipLengths, buffer, activity,
                    address, addrlen, client_socket, dConnect, c,
                    valread, clientStr, clientResponse, "client2",
                    c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                    c1Ships, c2Ships); // we have c in this function call bad for now
            }

            if(msg.at("messageType")!="matchOver"){
                //printAll(sd, clientStr, msg, boardSize, c1Board, c2Board);
            }

        }//FD_ISSET
    }
    return true;
}

bool placeShip(char board[10][10], char shipBoard[10][10], int boardSize, int row, int col, int length, Direction dir, json &msg, json (&ships)[6]){
    if( (row+length>boardSize || col+length>boardSize) || (row<0 || col<0) ){
        return false;
    }
    if(dir!=VERTICAL && dir!=HORIZONTAL){
        cerr << "Direction is WACC" << endl;
        return false;
    }
    if(dir==HORIZONTAL){
        for(int len=0;len<length;len++){
            if(board[row][col+len]!=WATER){
                cerr << "Ship placement error: HORIZONTAL" << endl;
                board[row][col+len] = 'E';
                return false;
            }
            board[row][col+len] = SHIP;
            shipBoard[row][col+len] = SHIP;
        }
    }else if(dir==VERTICAL){
        for(int len=0;len<length;len++){
            if(board[row+len][col]!=WATER){
                cerr << "Ship placement error: VERTICAL" << endl;
                board[row+len][col] = 'E';
                return false;
            }
            board[row+len][col] = SHIP;
            shipBoard[row+len][col] = SHIP;
        }
    }else{
        cerr << "Dir error: NONE is not valid" << endl;
        return false;
    }

    for(int num=0; num<6; num++){
        if(ships[num]=="null"){
            ships[num]= msg;
            break;
        }
    }

    return true;
}

char shootShot(char board[10][10], int boardSize, int row, int col){
    if( (row>=boardSize || col>=boardSize) || (row<0 || col<0) ){
        return INVALID_SHOT;
    }
    if(board[row][col]==WATER){
        board[row][col]=MISS;
        return MISS;
    }else if(board[row][col]==SHIP){
        board[row][col]=HIT;
        return HIT;
    }else if(board[row][col]==HIT || board[row][col]==KILL || board[row][col]==MISS || board[row][col] == DUPLICATE_SHOT){
        board[row][col]=DUPLICATE_SHOT;
        return DUPLICATE_SHOT; // maybe change to DUPLICATE_SHOT if necessary
    }
    return INVALID_SHOT;
}

int findDeadShip(int numShips, json (&ships)[6], json &msg, char board[10][10]){

    //numShips, board[][], msg
    bool allHit;
    for(int i=0; i<numShips; i++){
        allHit = true;
        for(int len=0;len<ships[i].at("length") && allHit; len++){
            if(ships[i].at("dir")==VERTICAL){
                if(board[ int(ships[i].at("row")) + len ][ int(ships[i].at("col")) ] != HIT){
                    allHit = false;
                    break;
                }
            }else if(ships[i].at("dir")==HORIZONTAL){
                if(board[ int(ships[i].at("row")) ][ int(ships[i].at("col")) + len ] != HIT){
                    allHit = false;
                    break;
                }
            }
        }

        if(allHit==true){
            for(int len=0;len<ships[i].at("length");len++){
                if(ships[i].at("dir")==VERTICAL){
                    board[int(ships[i].at("row"))+len][int(ships[i].at("col"))]=KILL;
                }else if(ships[i].at("dir")==HORIZONTAL){
                    board[int(ships[i].at("row"))][int(ships[i].at("col"))+len]=KILL;
                }
            }
            msg.at("row")=ships[i].at("row");
            msg.at("col")=ships[i].at("col");
            msg.at("dir")=ships[i].at("dir");
            msg.at("length")=ships[i].at("length");

            ships[i].at("messageType")="shipDead";
            msg.at("messageType")="shipDead";

            return i;
        }//end allHit if

    }//end for loop
    return -1;
}

int gameOver(json (&c1Ships)[6], json (&c2Ships)[6]){
    //0 is tie, -1 game is playing/no winners, 1 is p1 wins, 2 is p2 wins
    bool c1Ships_areDead=true, c2Ships_areDead = true;

    for(int i=0;i<6;i++){
        if(c1Ships[i].at("messageType")!="shipDead"){
            c1Ships_areDead=false;
        }
    }
    for(int i=0;i<6;i++){
        if(c2Ships[i].at("messageType")!="shipDead"){
            c2Ships_areDead=false;
        }
    }

    if(c1Ships_areDead && c2Ships_areDead){
        return 0;
    }else if(!c1Ships_areDead && c2Ships_areDead){
        return 1;
    }else if(c1Ships_areDead && !c2Ships_areDead){
        return 2;
    }
    else{
        return -1;
    }
}

bool sendReceive(Player &player, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int &activity, sockaddr_un address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread){
    //prepare the sockets for the connection


    prepSockets(readfds, master_socket, max_sd, sd, countConnected);


    char buffer[1500];

    strcpy(buffer, msg.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );

    

    fd_set otherfds = readfds;
    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed and activity gets a value less then 0.
    struct timeval tv = {0, 500000}; // Half a second = 500000
    //activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);
    activity = select( max_sd + 1 , &otherfds , NULL , NULL , &tv);


    // Handles a timeout error from the above select statement
    if ((activity <= 0) && (errno!=EINTR)){
        c.kill();
        printf("Child disconnected in activity\n");
        childDisconnect(sd, address, addrlen, client_socket, dConnect);
        return false;
    }


    // If everything worked when setting up sockets
    if(FD_ISSET(sd, &readfds)){
        // If/else here:
        //if: this means that read has failed and we shut things down. we notice this with valread.
        //else: everything went fine and all values are acceptable--begin the actual processing for the game.
        if ((valread = read( sd , buffer, 1499)) <= 0 || dConnect !=0){
            //get the details of the disconnected client and print them
            c.kill();
            printf("Child disconnected in valread\n");
            childDisconnect(sd, address, addrlen, client_socket, dConnect);
            return false;
        }else{
            //this else statement is where the data is processed for the game
            string temp="";
            childParse(temp, msg, valread, buffer);

            // get the authors from str
            player.author = msg.at("str");

        }//FD_ISSET
    }
    
    return true;
}
