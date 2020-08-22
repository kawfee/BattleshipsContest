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
bool shootShot(char board[10][10] , int boardSize, int row, int col);
bool checkLiveShips(int &numShips, json (&ships)[6], json &msg, char board[10][10]);





//runGame(10, "client", "client");
//int runGame(int boardSize, string client1, string client2)
int main(int argc , char *argv[]){

    //create the server variables
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients=2 , activity, i , valread , sd, max_sd,
        dConnect=0, countConnected=0, totalGameRound=1;
    struct sockaddr_in address;
    char buffer[1500];
    string clientStr, currentClient;

    //result variables when calling game functions for error checking
    bool p1Result, p2Result;

    //set of socket descriptors
    fd_set readfds;
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
    int boardSize = 10;
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

    setupServer(max_clients, client_socket, sd, master_socket, opt, address, i, addrlen);

    auto c1 = Popen({"./client"});
    auto c2 = Popen({"./client"});

    while(TRUE){
        if(countConnected < 2){
            //make sure that there are two connections to the master socket
            masterSocketConnection(readfds, master_socket, max_sd, activity,
                        new_socket, address, addrlen, msg,
                        max_clients, client_socket, i, countConnected);
            masterSocketConnection(readfds, master_socket, max_sd, activity,
                        new_socket, address, addrlen, msg,
                        max_clients, client_socket, i, countConnected);
        }

        /* plan for the future
            p1Result = placeShip();
            p2Result = placeShip();
            if p1 and p2 == false -1 ...:
                both lose
            if p1==true and p2==false:
                killRemaining(p1Result);
        */
        if(totalGameRound < 6){
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
            cout << "checkLiveShips == " << checkLiveShips(numShips, c1Ships, msg, c1Board) << endl;
            /*
                if( checkKilledShip(...) == True ) {
                performAction("shipKilled", ...);
            }
            */
            p2Result = performAction("shootShot", readfds, master_socket, max_sd, client_socket[1],
                countConnected, msg, shipLengths, buffer, activity,
                address, addrlen, client_socket, dConnect, c2,
                valread, clientStr, clientResponse, "client2",
                c1Board, c2Board, c1ShipBoard, c2ShipBoard, boardSize, totalGameRound,
                c1Ships, c2Ships);
            cout << "checkLiveShips == " << checkLiveShips(numShips, c2Ships, msg, c2Board) << endl;
        }
        // for the sd for performAction, pass client_socket[0] and client_socket[1] seperately

        //checkShips(c1Ships, c2Ships);

        if(totalGameRound >= 8){
            c1.kill();
            c2.kill();
            childDisconnect(client_socket[0], address, addrlen, client_socket, dConnect);
            childDisconnect(client_socket[1], address, addrlen, client_socket, dConnect);
        }

        cout << "dConnect is equal to: " << dConnect << endl;
        if(dConnect>=2){
            return 0;
        }

        totalGameRound++;
    }

    return 0;
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
    //prepare the sockets for the game
    prepSockets(readfds, master_socket, max_sd, sd, countConnected);

    cout << "currentClient: " << currentClient << endl;

    // send message
    msg.at("messageType") = messageType;
    msg.at("length") = shipLengths[totalGameRound];
    strcpy(buffer, msg.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );

    //wait at the sockets for a change,
    //if it takes longer than timeval tv the process is killed.
    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

    cout << "Activity: " << activity << endl;

    // Handles a timeout error from the above select statement
    if ((activity <= 0) && (errno!=EINTR)){
        //client_socket[0] and [1] are the two sd values
        childDisconnect(sd, address, addrlen, client_socket, dConnect);

        c.kill();

        return false;
    }

    if(FD_ISSET(sd, &readfds)){
        if ((valread = read( sd , buffer, 1499)) <= 0 || dConnect !=0){
            //get the details of the disconnected client and print them
            childDisconnect(sd, address, addrlen, client_socket, dConnect);
        }else{
            //this else statement is where the data is processed for the game
            childParse(clientStr, clientResponse, valread, buffer);

            msg.at("client") = clientResponse.at("client");
            msg.at("count") = clientResponse.at("count");
            msg.at("row") = clientResponse.at("row");
            msg.at("col") = clientResponse.at("col");
            msg.at("dir") = clientResponse.at("dir");
            msg.at("str") = clientResponse.at("str");

            cout << "CurrentClient: " << currentClient << endl;
            cout << "Message Direction: " << msg.at("dir") << endl;

            if(currentClient=="client1"){
                cout << "Got into client1" <<endl;
                if(messageType=="placeShip"){
                    c1Ships[totalGameRound-1]=clientResponse;
                    placeShip(c1Board, c1ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c1Ships);
                }else if(messageType=="shootShot"){
                    shootShot(c1Board, boardSize, msg.at("row"), msg.at("col"));
                }
            }else if(currentClient=="client2"){
                cout << "Got into client2" <<endl;
                if(messageType=="placeShip"){
                    c2Ships[totalGameRound-1]=clientResponse;
                    placeShip(c2Board, c2ShipBoard, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), msg, c2Ships);
                }else if(messageType=="shootShot"){
                    shootShot(c2Board, boardSize, msg.at("row"), msg.at("col"));
                }
            }

            if(currentClient == "client2"){
                printAll(sd, clientStr, msg, boardSize, c1Board, c2Board);
            }

        }//FD_ISSET
    }
    return true;
}
bool placeShip(char board[10][10], char shipBoard[10][10], int boardSize, int row, int col, int length, Direction dir, json &msg, json (&ships)[6]){
    std::cout << "Ship Stuff: (len, dir) " << length << " " << dir << std::endl;
    if( (row+length>boardSize || col+length>boardSize) || (row<0 || col<0) ){
        cout << "Ship placement error: out of bounds" << endl;
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
            cout << "HORIZONTAL check space: " << "row, " << row << " col, " << (col+len) << " Spot, " << board[row][col+len] << endl;
            if(board[row][col+len]!=WATER){
                cout << "Ship placement error: HORIZONTAL" << endl;
                board[row][col+len] = 'E';
                return false;
            }
            board[row][col+len] = SHIP;
            shipBoard[row][col+len] = SHIP;
        }
    }else if(dir==VERTICAL){
        for(int len=0;len<length;len++){
            cout << "VERTICAL check space: "  << "row, " << (row+len) << " col, " << col << " Spot, " << board[row+len][col] << endl;
            if(board[row+len][col]!=WATER){
                cout << "Ship placement error: VERTICAL" << endl;
                board[row+len][col] = 'E';
                return false;
            }
            board[row+len][col] = SHIP;
            shipBoard[row+len][col] = SHIP;
        }
    }else{
        cout << "I am sorry my friend, but something has gone horribly wrong. It would be best for you to both repent and call an exorcist immediately." << endl;
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
bool shootShot(char board[10][10], int boardSize, int row, int col){
    if( (row>=boardSize || col>=boardSize) || (row<0 || col<0) ){
        cout << "Shot out of bounds" << endl;
        return false;
    }
    if(board[row][col]==WATER){
        board[row][col]=MISS;
    }else if(board[row][col]==SHIP){
        board[row][col]=HIT;
    }else if(board[row][col]==HIT || board[row][col]==KILL){
        board[row][col]=DUPLICATE_SHOT;
        return false;
    }

    return true;
}



bool checkLiveShips(int &numShips, json (&ships)[6], json &msg, char board[10][10]){
    //numShips, board[][], msg
    for(int i=0; i<numShips; i++){
        bool allHit = true;
        for(int len=0;len<ships[i].at("length") && allHit; len++){
            if(ships[i].at("dir")==VERTICAL){
                if(board[ int(ships[i].at("row")) + len ][ int(ships[i].at("col")) ] != HIT){
                    allHit = false;
                }
            }else if(ships[i].at("dir")==HORIZONTAL){
                if(board[ int(ships[i].at("row")) ][ int(ships[i].at("col")) + len ] != HIT){
                    allHit = false;
                    break;
                }
            }
        }

        if(allHit==true){
            msg=ships[i];
            msg.at("messageType")="shipDead";
            printf("THE SHIP IS GOING DOWN AHHHHHHHHHHH!");
            return true;
        }
        
    }
    return false;
}

/* getClientName
void getClientName(string (&client_names)[2], int &sd){
    json dummyMessage = {"messageType", "getName"};

    //send
    char buffer[1500]
    strcpy(buffer, dummyMessage.dump().c_str());
    send(sd , buffer , strlen(buffer) , 0 );
    //read

    struct timeval tv = {0, 500000}; // Half a second
    activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

    cout << "Activity: " << activity << endl;

    read( sd , tempBuffer, 1499));
    //set array
}
*/
/* base game idea
    if (rounds < countships):
        askforship()
    else():
        shootShot()
*/
