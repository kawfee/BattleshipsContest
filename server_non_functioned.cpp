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


void setupServer(int &max_clients, int (&client_socket)[30], int &master_socket, int &opt, sockaddr_in &address, int &i, int &addrlen);
void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &sdTurn, int (&client_socket)[30], int &countConnected);
void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_in &address, int &addrlen, json &msg, int &max_clients, int (&client_socket)[30], int &i, int &countConnected);
void childDisconnect(int &sd, sockaddr_in &address, int &addrlen, int (&client_socket)[30], int &sdTurn, int &dConnect);
void childParse(string &clientStr, json &clientResponse, int &valread, char (&buffer)[1500]);
void messageHandler(json &msg, string &currentClient, json &clientResponse, char (&c1Board)[10][10] , char (&c2Board)[10][10] , int boardSize);
bool placeShip(char board[10][10] , int boardSize, int row, int col, int length, Direction dir);
bool shootShot(char board[10][10] , int boardSize, int row, int col);

//runGame(10, "client", "client");
//int runGame(int boardSize, string client1, string client2)
int main(int argc , char *argv[]){
    //create the SERVER VARIABLES------------------------------------
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients=2 , activity, i , valread , sd, max_sd,
        sdTurn=0, dConnect=0, countConnected=0, totalGameRound=0;
    struct sockaddr_in address;
    char buffer[1500];
    string clientStr, currentClient;

    //boolean value to decide whether or not to keep running the game.
    bool cont = true;

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

    //create the GAME VARIABLES------------------
    int shipLengths[] = { 3,3,4,3,3,4 };
    int boardSize = 10;
    int numShips = boardSize-2;
    if(numShips>6){
        numShips=6;
    }
    char c1Board[10][10];
    char c2Board[10][10];
    json c1Ships[6];
    json c2Ships[6];

    //populate the boards
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            c1Board[row][col] = WATER;
            c2Board[row][col] = WATER;
        }
    }

    //prepare for the game to start
    setupServer(max_clients, client_socket, master_socket, opt, address, i, addrlen);

    auto c1 = Popen({"./client"});
    auto c2 = Popen({"./client"});

    while(TRUE){
        //ready the sockets
        prepSockets(readfds, master_socket, max_sd, sd, sdTurn, client_socket, countConnected);

        sd = client_socket[sdTurn];

        if (sdTurn == 0) currentClient = "client1";
        else currentClient = "client2";

        cout << "currentClient: " << currentClient << endl;

        //place the ships, or not
        if(countConnected >= 2){
            if(totalGameRound <= 6){
                msg.at("length") = shipLengths[totalGameRound];
            }
            strcpy(buffer, msg.dump().c_str());
            send(sd , buffer , strlen(buffer) , 0 );
        }

        //wait at the sockets for a change,
        //if it takes longer than timeval tv the process is killed.
        struct timeval tv = {0, 500000}; // Half a second
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

        cout << "Activity: " << activity << endl;

        // Handles a timeout error from the above select statement
        if ((activity <= 0) && (errno!=EINTR)){
        // if ((activity <= 0) && (errno!=EINTR) || cont == false){
            if(sdTurn == 0){
                cout << "SID " << client_socket[1] << " won!" << endl;
            }else{
                cout << "SID " << client_socket[0] << " won!" << endl;
            }
            //client_socket[0] and [1] are the two sd values
            childDisconnect(client_socket[0], address, addrlen, client_socket, sdTurn, dConnect);
            childDisconnect(client_socket[1], address, addrlen, client_socket, sdTurn, dConnect);

            c1.kill();
            c2.kill();

            return activity;
            //return winner, kills, ... & game stats;
        }


        //MASTER_SOCKET IS TOUCHED
        if (FD_ISSET(master_socket, &readfds)){
            //this adds new connections to the client_socket array when master socket is modified/accessed
            masterSocketTouched(new_socket, master_socket, address, addrlen, msg, max_clients, client_socket, i, countConnected);
        }

        //ACTIVE CHILD_SOCKET IS TOUCHED


        if (FD_ISSET( sd , &readfds)){
            //Check if it was for closing , and also read the
            //incoming message
            if ((valread = read( sd , buffer, 1499)) <= 0 || dConnect !=0){
                //get the details of the disconnected client and print them
                childDisconnect(sd, address, addrlen, client_socket, sdTurn, dConnect);
            }

            //Echo back the message that came in
            else{
                //this else statement is where the data is processed for the game
                childParse(clientStr, clientResponse, valread, buffer);

                if(!cont){ //kill the game when cont is set to false.
                    //client_socket[0] and [1] are the two sd values

                    c1.kill();
                    c2.kill();

                    childDisconnect(client_socket[0], address, addrlen, client_socket, sdTurn, dConnect);
                    childDisconnect(client_socket[1], address, addrlen, client_socket, sdTurn, dConnect);

                    dConnect = 2;
                    cout << "dConnect is equal to: " << dConnect << endl;


                    cout << "Clients killed successfully"
                         << endl;

                    return 0;
                }

                //sends the message to be processed
                messageHandler(msg, currentClient, clientResponse, c1Board, c2Board, boardSize);

                strcpy(buffer, msg.dump().c_str());
                // End of setting data


                //test print
                cout << "Test Print: \nsid = "
                     << sd
                     << " \ncurrent sdTurn = "
                     << sdTurn
                     << " : \nbuffer = "
                     << clientStr
                     << " : \nmsg = \n"
                     << msg.dump(4)
                     << endl;

                std::cout << "c1Board" << std::endl;
                for(int row=0;row<boardSize;row++){
                    for(int col=0;col<boardSize;col++){
                        std::cout << c1Board[row][col];
                    }
                    cout << endl;
                }
                std::cout << "c2Board" << std::endl;
                for(int row=0;row<boardSize;row++){
                    for(int col=0;col<boardSize;col++){
                        std::cout << c2Board[row][col];
                    }
                    cout << endl;
                }

                if (sdTurn == 0){
                    sdTurn=1;
                }
                else if (sdTurn == 1){
                    sdTurn=0;
                }

                if(currentClient=="client2"){
                    totalGameRound++;
                }

                if(totalGameRound>=8){ // prepare to end the game after round: 2
                    cont = false;
                }
                else if(totalGameRound>=6) {
                    msg.at("messageType") = "shootShot";
                }

            }
        }
        cout << "dConnect is equal to: " << dConnect << endl;
        if(dConnect>=2){
            return 0;
        }
    }

    return 0;
}

void setupServer(int &max_clients, int (&client_socket)[30], int &master_socket, int &opt, sockaddr_in &address, int &i, int &addrlen){
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

void prepSockets(fd_set &readfds, int &master_socket, int &max_sd, int &sd, int &sdTurn, int (&client_socket)[30], int &countConnected){
    //clear the socket set
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    //socket descriptor
    if (countConnected >= 2){
        sd = client_socket[sdTurn];
        std::cout << "The SD for this turn is: " << sd << '\n';

        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET( sd , &readfds);

        //highest file descriptor number, need it for the select function
        if(sd > max_sd)
            max_sd = sd;
    }
}

void masterSocketTouched(int &new_socket, int &master_socket, sockaddr_in &address, int &addrlen, json &msg, int &max_clients, int (&client_socket)[30], int &i, int &countConnected){
    if ((new_socket = accept(master_socket,
        (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
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

void childDisconnect(int &sd, sockaddr_in &address, int &addrlen, int (&client_socket)[30], int &sdTurn, int &dConnect){
    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Client %d disconnected , ip %s , port %d \n" ,
          sd, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

    //Close the socket and mark as 0 in list for reuse
    close( sd );

    client_socket[sdTurn] = (int)0;
    if ( sdTurn == 0){
        sdTurn=1;
    }
    else if ( sdTurn == 1){
        sdTurn=0;
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

void messageHandler(json &msg, string &currentClient, json &clientResponse, char (&c1Board)[10][10] , char (&c2Board)[10][10], int boardSize){
    msg.at("client") = clientResponse.at("client");
    msg.at("count") = clientResponse.at("count");

    if(clientResponse.at("messageType") == "placeShip"){
        msg.at("row") = clientResponse.at("row");
        msg.at("col") = clientResponse.at("col");
        msg.at("dir") = clientResponse.at("dir");

        cout << "CurrentClient: " << currentClient << endl;
        cout << "Message Direction: " << msg.at("dir") << endl;

        if(currentClient=="client1"){
            cout << "Got into client1 shipplacement" <<endl;
            placeShip(c1Board, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"));
        }else if(currentClient=="client2"){
            cout << "Got into client2 shipplacement" <<endl;
            placeShip(c2Board, boardSize, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"));
        }else{
            cerr << "Invalid client" << endl;
        }
    }else if(clientResponse.at("messageType") == "shootShot"){
        msg.at("row") = clientResponse.at("row");
        msg.at("col") = clientResponse.at("col");

        cout << "You shot me ow: (row, col) " << msg.at("row") << " " << msg.at("col") << endl;

        if(currentClient=="client1"){
            cout << "Got into client1 shootShot" <<endl;
            shootShot(c1Board , boardSize, msg.at("row"), msg.at("col") );
        }else if(currentClient=="client2"){
            cout << "Got into client2 shootShot" <<endl;
            shootShot(c2Board , boardSize, msg.at("row"), msg.at("col") );
        }else{
            cerr << "Invalid client" << endl;
        }

    }else{
        cerr << "Invalid message type" << endl;
        return;
    }
}

//returns false if ship fails at placing
bool placeShip(char board[10][10] , int boardSize, int row, int col, int length, Direction dir){
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
        }
    }else{
        cout << "I am sorry my friend, but something has gone horribly wrong. It would be best for you to both repent and call an exorcist immediately." << endl;
        return false;
    }

    cout << "Got to end of placeShip" << endl;
    return true;
}

/* oldBattleshipsCode--placeShips
const int NumShips = 6;
string shipNames[NumShips]  = { "Submarine", "Destroyer", "Aircraft Carrier", "Destroyer 2", "Submarine 2", "Aircraft Carrier 2" };
int shipLengths[NumShips] = { 3,3,4,3,3,4 };
int maxShips = boardSize-2;
if( maxShips > NumShips ) {
    maxShips = NumShips;
}
for( int i=0; i<maxShips; i++ ) {
    Message loc = player->placeShip( shipLengths[i] );
    bool placedOk = placingBoard->placeShip( loc.getRow(), loc.getCol(), shipLengths[i], loc.getDirection() );
    if( ! placedOk ) {
    cerr << "Error: couldn't place "<<shipNames[i]<<" (length "<<shipLengths[i]<<")"<<endl;
    return false;
}
board->placeShip(shipLengths[i]);
}

// All ships apparently placed ok.
return true;
*/

bool shootShot(char board[10][10] , int boardSize, int row, int col){
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

/* return whether or not a ship has been killed
bool checkLiveShips(){
    for(int i=0; i<numShips;i++){
        for(int len=0;len<length;
    }
}
*/

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
