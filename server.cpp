/*
    Authors: Joey Gorski, Matthew Bouch
    Battleships Server

    Used resources:
    Code help for using and setting up sockets
        https://simpledevcode.wordpress.com/2016/06/16/client-server-chat-in-c-using-sockets/
        https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

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
#include <cstring>

#include "json.hpp"
#include "subprocess.hpp"
#include "defines.h"

#define TRUE   1
#define FALSE  0
#define PORT 54321

using namespace std;
using json = nlohmann::json;
using namespace subprocess;



void setupServer(int &max_clients, int (&client_socket)[30], int &sd, int &master_socket, int &opt, sockaddr_in &address, int &i, int &addrlen);
void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &sdTurn, int &countConnected);
int masterSocketConnection(fd_set &readfds, int &master_socket, int &max_sd, int &activity, int &new_socket, sockaddr_in &address,
            int &addrlen, json &msg, int &max_clients, int (&client_socket)[30], int &i, int &countConnected);
void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_in &address, int &addrlen, json &msg, int &max_clients,
            int (&client_socket)[30], int &i, int &countConnected);
void childDisconnect(int &sd, sockaddr_in &address, int &addrlen, int (&client_socket)[30], int &dConnect);
void childParse(string &clientStr, json &clientResponse, int &valread, char (&buffer)[1500]);
void printAll(int sd, string clientStr, json msg, int boardSize, char c1Board[10][10], char c2Board[10][10]);
bool performAction(string messageType, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int (&shipLengths)[6], char (&buffer)[1500], int &activity, sockaddr_in address, int &addrlen, int (&client_socket)[30],
            int &dConnect, Popen &c, int &valread, string &clientStr, json &clientResponse, string currentClient, char c1Board[10][10],
            char c2Board[10][10], char c1ShipBoard[10][10], char c2ShipBoard[10][10], int &boardSize, int totalGameRound,
            json (&c1Ships)[6], json (&c2Ships)[6]);
bool placeShip(char board[10][10] , char shipBoard[10][10], int boardSize, int row, int col, int length, Direction dir, json &msg, json (&ships)[6]);
char shootShot(char board[10][10] , int boardSize, int row, int col);
int findDeadShip(int numShips, json (&ships)[6], json &msg, char board[10][10]);
int gameOver(json (&c1Ships)[6], json (&c2Ships)[6]);
bool sendReceive(Player &player, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int &activity, sockaddr_in address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread);
GameInfo runMatch(Player player1, Player player2, int boardSize, int master_socket, int addrlen, int (&client_socket)[30], int activity, int valread, int sd,
    int max_sd, int dConnect, int countConnected, sockaddr_in address, fd_set readfds, Popen &c1, Popen &c2);



int runGame(int numGames, Player &player1, Player &player2, int boardSize){
    //setup socket functionality
    //create the server variables
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients=2 , activity, i , valread=1 , sd, max_sd,
        dConnect=0, countConnected=0;
    struct sockaddr_in address;
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

    setupServer(max_clients, client_socket, sd, master_socket, opt, address, i, addrlen);

    auto c1 = Popen({"./client_Ais/"+player1.name});
    masterSocketConnection(readfds, master_socket, max_sd, activity,
        new_socket, address, addrlen, gameMsg,
        max_clients, client_socket, i, countConnected);

    auto c2 = Popen({"./client_Ais/"+player2.name});
    masterSocketConnection(readfds, master_socket, max_sd, activity,
        new_socket, address, addrlen, gameMsg,
        max_clients, client_socket, i, countConnected);

    sendReceive(player1, readfds, master_socket, max_sd, client_socket[0], countConnected, gameMsg,
        activity, address, addrlen, client_socket, dConnect,
        c1, valread);

    sendReceive(player2, readfds, master_socket, max_sd, client_socket[1], countConnected, gameMsg,
        activity, address, addrlen, client_socket, dConnect,
        c2, valread);        

    cout << "Authors: "        << player1.author << endl
         << "+ Author other: " << player2.author << endl;
    

    // Starting game stuff
    for(int i = 0; i <  numGames; i++){
        cout << "FOR LOOP ITERATIONS: " << i << "/" << numGames << endl;
        GameInfo match = runMatch(player1, player2, boardSize, master_socket, addrlen, client_socket, activity, valread, sd,
            max_sd, dConnect, countConnected, address, readfds, c1, c2);
        
        cout << "RUNMATCH: " << match.error << endl;

        if(match.error == true){
            c1.kill();
            c2.kill();
            childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
            childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);
            return -1; // or a break maybe? change as necessary
        }

        // string name;
        // string author;
        // int wins;
        // int ties;
        // int losses;

        cout << endl << endl
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
             << endl << endl;

        player1=match.player1;
        player2=match.player2;
    }

    c1.kill();
    c2.kill();
    childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
    childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);


    return 1;
}


GameInfo runMatch(Player player1, Player player2, int boardSize, int master_socket, int addrlen, int (&client_socket)[30], int activity, int valread, int sd,
    int max_sd, int dConnect, int countConnected, sockaddr_in address, fd_set readfds, Popen &c1, Popen &c2){
    GameInfo temp;
    temp.player1 = player1;
    temp.player2 = player2;
    temp.error=false;
    
    int totalGameRound=1;
    
    cout << endl << endl
            << "MATCH DATA: " << endl
            << "\t" << "PLAYER ONE: " << endl
            << "\t\tWINS: " << player1.wins << endl
            << "\t\tTIES: " << player1.ties << endl
            << "\t\tLOSSES: " << player1.losses << endl
            << "\t" << "PLAYER TWO: " << endl
            << "\t\tWINS: " << player2.wins << endl
            << "\t\tTIES: " << player2.ties << endl
            << "\t\tLOSSES: " << player2.losses << endl
            << endl << endl;

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
        }else{
            p1Result = performAction("shootShot", readfds, master_socket, max_sd, client_socket[0],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c1,
                valread, clientStr, clientResponse, "client1",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);

            //json* tempMsg = new json(*msg);
            // Just in case we need a deep copy
            json tempMsg = {
                {"messageType", msg.at("messageType")},
                {"row", msg.at("row")},
                {"col", msg.at("col")},
                {"str", msg.at("str")},
                {"dir", msg.at("dir")},
                {"length", msg.at("length")},
                {"client", msg.at("client")},
                {"count", msg.at("count")}
            }; 
            cout << "SETTING TEMPMESSAGE TO MESSAGE: " << endl << tempMsg << endl;
                
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
                cout << "returned from return1" << endl;
                return temp;
            }
            else if(p1Result && !p2Result){
                // return p1 as winner
                cout << "p2 CRASHED!" << endl;
                temp.player1.wins += 1;
                temp.player2.losses += 1;
                temp.error = true;
                cout << "returned from return2" << endl;
                return temp;
            }
            else if(!p1Result && p2Result){
                // return p2 as winner
                cout << "p1 CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.wins += 1;
                temp.error = true;
                cout << "returned from return3" << endl;
                return temp;
            }

            
            //do dead ship checking 
            cout << "tempMessage:" << endl << tempMsg << endl;
            cout << "message: " << endl << msg << endl;
            int p1DeadShip = findDeadShip(numShips, c1Ships, tempMsg, c1Board);
            int p2DeadShip = findDeadShip(numShips, c2Ships, msg, c2Board);

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
                cout << "returned from return4" << endl;
                return temp;
            }
            else if(p1Result && !p2Result){
                // return p1 as winner
                cout << "p2 CRASHED!" << endl;
                temp.player1.wins += 1;
                temp.player2.losses += 1;
                temp.error = true;
                cout << "returned from return5" << endl;
                return temp;
            }
            else if(!p1Result && p2Result){
                // return p2 as winner
                cout << "p1 CRASHED!" << endl;
                temp.player1.losses += 1;
                temp.player2.wins += 1;
                temp.error = true;
                cout << "returned from return6" << endl;
                return temp;
            }
        }
        


        if(totalGameRound>=6){
            int gameStatus = gameOver(c1Ships, c2Ships);

            if(gameStatus>=0){
                // perform game over logic
                // modify struct data
                if(gameStatus==0){
                    cout << "TIE!" << endl;
                    temp.player1.ties += 1;
                    temp.player2.ties += 1;
                }else if(gameStatus==1){
                    cout << "PLAYER 1 WINS!" << endl;
                    temp.player1.wins += 1;
                    temp.player2.losses += 1;
                }else if(gameStatus==2){
                    cout << "PLAYER 2 WINS!" << endl;
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
                
                
                cout << "returned from return7" << endl;
                return temp;
            } 
        }
        
        //this is NOT the final number of rounds to be played--this is just a testing number. was at 13 for low turn count
        if(totalGameRound >= 300){
            cout << "THE INSIDE OF THE TOTALGAMEROUND CHECKER WAS REACHED" << endl;
            c1.kill();
            c2.kill();
            childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
            childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);
        }

        cout << "dConnect is equal to: " << dConnect << endl;
        if(dConnect>=2){
            temp.error=true;
            cout << "returned from return8" << endl;
            return temp;
        }

        totalGameRound++;
    }

    temp.error=true;
    cout << "returned from return9" << endl;
    return temp;
}























void setupServer(int &max_clients, int (&client_socket)[30], int &sd, int &master_socket, int &opt, sockaddr_in &address, int &i, int &addrlen){
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 54321
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
}

void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &countConnected){
    //clear the socket set
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    //socket descriptor
    std::cout << "The SD for this turn is: " << sd << '\n';

    //if valid socket descriptor then add to read list
    if(sd > 0)
        FD_SET( sd , &readfds);

    //highest file descriptor number, need it for the select function
    if(sd > max_sd)
        max_sd = sd;
}

int masterSocketConnection(fd_set &readfds, int &master_socket, int &max_sd, int &activity,
            int &new_socket, sockaddr_in &address, int &addrlen, json &msg,
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
        masterSocketTouched(new_socket, master_socket, address, addrlen, msg, max_clients, client_socket, i, countConnected);
    }

    return 1;
}

void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_in &address, int &addrlen,
            json &msg, int &max_clients, int (&client_socket)[30], int &i, int &countConnected){
    if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //inform user of socket number - used in send and receive commands
    printf("New connection , sid is %d , ip is : %s , port : %d  \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));


    //add new socket to array of sockets
    for (i = 0; i < max_clients; i++)
    {
        //if position is empty
        if( client_socket[i] == 0 )
        {
            client_socket[i] = new_socket;
            printf("Adding to list of sockets in position %d\n" , i);

            break;
        }
    }

    countConnected++;
}

void childDisconnect(int &sd, sockaddr_in &address, int &addrlen, int (&client_socket)[30], int &dConnect){
    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Socket %d disconnected , ip %s , port %d \n", sd, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

    //Close the socket and mark as 0 in list for reuse
    close( sd );

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

bool performAction(string messageType, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg, int (&shipLengths)[6],
            char (&buffer)[1500], int &activity, sockaddr_in address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread, string &clientStr, json &clientResponse, string currentClient, char c1Board[10][10],
            char c2Board[10][10], char c1ShipBoard[10][10], char c2ShipBoard[10][10], int &boardSize, int totalGameRound, json (&c1Ships)[6], json (&c2Ships)[6]){
    //prepare the sockets for the connection
    prepSockets(readfds, master_socket, max_sd, sd, countConnected);

    cout << "currentClient: " << currentClient << endl;

    // send message by setting json data given in function call
    msg.at("messageType") = messageType;
    if(messageType == "placeShip"){
        cout <<  "totalGameRound: " << totalGameRound << endl;
        msg.at("length") = shipLengths[totalGameRound-1];
    }
    strcpy(buffer, msg.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );

    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed and activity gets a value less then 0.
    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

    cout << "Activity: " << activity << endl;

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

            cout << "CurrentClient: " << currentClient << endl;
            cout << "Message Direction: " << msg.at("dir") << endl;

            /*
            Depending on which client we have passed to this function we go into their if statement and procede to call our
            different helper functions depending on which action we are going to perform.
            */
            
            char shotReturnValue = PLACE_SHIP;

            if(currentClient=="client1"){
                // cout << "Got into client1" <<endl;
                if(messageType=="placeShip"){
                    c1Ships[totalGameRound-1]=clientResponse;
                    placeShip(c1Board, c1ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c1Ships);
                }else if(messageType=="shootShot"){
                    // cout << "msg row and col: " << msg.at("row") << " " << msg.at("col") << endl;
                    int tempRow = msg.at("row");
                    int tempCol = msg.at("col");
                    shotReturnValue = shootShot(c2Board, boardSize, tempRow, tempCol);
                }
            }else if(currentClient=="client2"){
                // cout << "Got into client2" <<endl;
                if(messageType=="placeShip"){
                    c2Ships[totalGameRound-1]=clientResponse;
                    placeShip(c2Board, c2ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c2Ships);
                }else if(messageType=="shootShot"){
                    // cout << "msg row and col: " << msg.at("row") << " " << msg.at("col") << endl;
                    int tempRow = msg.at("row");
                    int tempCol = msg.at("col");
                    shotReturnValue = shootShot(c1Board, boardSize, tempRow, tempCol);
                }
            }
            
            if(messageType == "shootShot"){
                cout << "Got into shotReturnValue if statement" << endl;
                if(shotReturnValue==INVALID_SHOT){
                    //TODO deal with INVALID_SHOTs
                    cout << "INVALID_SHOT returned by shootShot handler" << endl;
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
                printAll(sd, clientStr, msg, boardSize, c1Board, c2Board);
            }

        }//FD_ISSET
    }
    return true;
}

bool placeShip(char board[10][10], char shipBoard[10][10], int boardSize, int row, int col, int length, Direction dir, json &msg, json (&ships)[6]){
    // std::cout << "Ship Stuff: (len, dir) " << length << " " << dir << std::endl;
    if( (row+length>boardSize || col+length>boardSize) || (row<0 || col<0) ){
        // cout << "Ship placement error: out of bounds" << endl;
        return false;
    }
    if(dir==VERTICAL){
        cout << "Direction is VERTICAL" << endl;
    }else if(dir==HORIZONTAL){
        cout << "Direction is HORIZONTAL" << endl;
    }else{
        cout << "Direction is WACC" << endl;
        return false;
    }
    if(dir==HORIZONTAL){
        for(int len=0;len<length;len++){
            // cout << "HORIZONTAL check space: " << "row, " << row << " col, " << (col+len) << " Spot, " << board[row][col+len] << endl;
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
            // cout << "VERTICAL check space: "  << "row, " << (row+len) << " col, " << col << " Spot, " << board[row+len][col] << endl;
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

    cout << "Got to end of placeShip" << endl;
    return true;
}

char shootShot(char board[10][10], int boardSize, int row, int col){
    printf("Got into shootShot\n");
    if( (row>=boardSize || col>=boardSize) || (row<0 || col<0) ){
        cout << "Shot out of bounds" << endl;
        return INVALID_SHOT;
    }
    if(board[row][col]==WATER){
        board[row][col]=MISS;
        return MISS;
    }else if(board[row][col]==SHIP){
        board[row][col]=HIT;
        return HIT;
    }else if(board[row][col]==HIT || board[row][col]==KILL || board[row][col]==MISS){
        board[row][col]=DUPLICATE_SHOT;
        return INVALID_SHOT; // maybe change to DUPLICATE_SHOT if necessary
    }
    printf("Invalid shot returned in shootShot\n");
    return INVALID_SHOT;
}

int findDeadShip(int numShips, json (&ships)[6], json &msg, char board[10][10]){

    printf("Got into findDeadShip\n");
    cout << "Message: " << msg << endl;
    //numShips, board[][], msg
    bool allHit;
    for(int i=0; i<numShips; i++){
        printf("Top of main for loop\n");
        // cout << "Ship coordinates: (" << ships[i].at("row") << ", " << ships[i].at("col") << ")" << endl;
        allHit = true;
        for(int len=0;len<ships[i].at("length") && allHit; len++){
            cout << "Top of checker for loop: " << len << endl;
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

        printf("Reached outside allHit checker\n");
        if(allHit==true){
            for(int len=0;len<ships[i].at("length");len++){
                if(ships[i].at("dir")==VERTICAL){
                    board[int(ships[i].at("row"))+len][int(ships[i].at("col"))]=KILL;
                }else if(ships[i].at("dir")==HORIZONTAL){
                    board[int(ships[i].at("row"))][int(ships[i].at("col"))+len]=KILL;
                }
            }
            printf("Reached inside allHit checker\n");
            msg.at("row")=ships[i].at("row");
            msg.at("col")=ships[i].at("col");
            msg.at("dir")=ships[i].at("dir");
            msg.at("length")=ships[i].at("length");

            ships[i].at("messageType")="shipDead";
            msg.at("messageType")="shipDead";

            printf("A ship has been sunk.\n");
            return i;
        }//end allHit if

    }//end for loop
    printf("Finished findDeadShip\n");
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
    }
    else if(!c1Ships_areDead && c2Ships_areDead){
        return 1;
    }
    else if(c1Ships_areDead && !c2Ships_areDead){
        return 2;
    }
    else{
        return -1;
    }
}

bool sendReceive(Player &player, fd_set &readfds, int &master_socket, int &max_sd, int sd, int &countConnected, json &msg,
            int &activity, sockaddr_in address, int &addrlen, int (&client_socket)[30], int &dConnect,
            Popen &c, int &valread){
    //prepare the sockets for the connection
    prepSockets(readfds, master_socket, max_sd, sd, countConnected);
    
    char buffer[1500];

    strcpy(buffer, msg.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );

    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed and activity gets a value less then 0.
    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

    cout << "Activity: " << activity << endl;

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
