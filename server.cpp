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
void childSend(char (&buffer)[1500], int &sd, int &sdTurn);
void messageHandler(json msg, string currentClient, char (&c1Board)[10][10] , char (&c2Board)[10][10]);
void placeShip(char (&board)[10][10] , int row, int col);

int main(int argc , char *argv[]){
    //create the server variables
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients=2 , activity, i , valread , sd, max_sd,
        sdTurn=0, dConnect=0, countConnected=0;
    struct sockaddr_in address;
    char buffer[1500];
    string clientStr, currentClient;
    //set of socket descriptors
    fd_set readfds;
    json msg = {
        {"messageType", "placeShip"},
        {"row", -1},
        {"col", -1},
        {"str", ""},
        {"dir", HORIZONTAL},
        {"length", 3},
        {"client", "none"},
        {"count", 0}
    };
    json clientResponse;

    //create the game variables here
    char c1Board[10][10];
    char c2Board[10][10];

    //populate the boards
    for(int row=0;row<10;row++){
        for(int col=0;col<10;col++){
            c1Board[row][col] = WATER;
            c2Board[row][col] = WATER;
        }
    }

    setupServer(max_clients, client_socket, master_socket, opt, address, i, addrlen);

    auto c1 = Popen({"./client"});
    auto c2 = Popen({"./client"});

    while(TRUE)
    {
        //prepare the sockets for liftoff
        prepSockets(readfds, master_socket, max_sd, sd, sdTurn, client_socket, countConnected);

        sd = client_socket[sdTurn];

        if (sdTurn == 0){
            currentClient = "client1";
        }
        else{
            currentClient = "client2";
        }

        //send message
        if(countConnected >= 2){
            strcpy(buffer, msg.dump().c_str());
            send(sd , buffer , strlen(buffer) , 0 );
        }

        //wait at the sockets for a change,
        //if it takes longer than timeval tv the process is killed.
        struct timeval tv = {0, 500000}; // Half a second
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

        cout << "Activity: " << activity << endl;

        // Handles a timeout error from the above select statement
        if ((activity <= 0) && (errno!=EINTR))
        {
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
            if ((valread = read( sd , buffer, 1499)) == 0 || dConnect !=0){
                //get the details of the disconnected client and print them
                childDisconnect(sd, address, addrlen, client_socket, sdTurn, dConnect);
            }

            //Echo back the message that came in
            else{
                //this else statement is where the data is processed for the game
                childParse(clientStr, clientResponse, valread, buffer);

                // Sets data to send to client
                msg.at("client") = clientResponse.at("client");
                msg.at("count") = clientResponse.at("count");

                messageHandler(msg, currentClient, c1Board, c2Board);

                if(msg.at("count")>=50){ //kill the game at round 50
                    //client_socket[0] and [1] are the two sd values
                    childDisconnect(client_socket[0], address, addrlen, client_socket, sdTurn, dConnect);
                    childDisconnect(client_socket[1], address, addrlen, client_socket, sdTurn, dConnect);

                    dConnect = 2;
                    cout << "dConnect is equal to: " << dConnect << endl;

                    c1.kill();
                    c2.kill();

                    cout << "Clients killed successfully"
                         << endl;

                    break;
                }

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
                for(int row=0;row<10;row++){
                    for(int col=0;col<10;col++){
                        std::cout << c1Board[row][col];
                    }
                    cout << endl;
                }
                std::cout << "c2Board" << std::endl;
                for(int row=0;row<10;row++){
                    for(int col=0;col<10;col++){
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
    client_socket[sdTurn] = 0;
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

void messageHandler(json msg, string currentClient, char (&c1Board)[10][10] , char (&c2Board)[10][10]){
    if(currentClient=="client1"){
        if(msg.at("messageType") == "placeShip"){
            placeShip(c1Board, msg.at("row"), msg.at("col"));
        }
    }else if(currentClient=="client2"){
        if(msg.at("messageType") == "placeShip"){
            placeShip(c2Board, msg.at("row"), msg.at("col"));
        }
    }else{
        cerr << "Invalid client input" << endl;
        return;
    }
}

void placeShip(char (&board)[10][10] , int row, int col){
    board[row][col] = 'S';
    std::cout << "This is placeShip: " << board[row][col] << std::endl;
    std::cout << "This is board:" << std::endl;
    for(int row=0;row<10;row++){
        for(int col=0;col<10;col++){
            std::cout << board[row][col];
        }
        cout << endl;
    }
}
