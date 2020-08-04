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

#define TRUE   1
#define FALSE  0
#define PORT 54321

using namespace std;
using json = nlohmann::json;

void setupServer(int &max_clients, int (&client_socket)[30], int &master_socket, int &opt, sockaddr_in &address, int &i, int &addrlen);

int main(int argc , char *argv[]){
    //create the variables
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
        max_clients = 2 , activity, i , valread , sd;
    int max_sd;
    int sdTurn = 0;
    struct sockaddr_in address;

    int dConnect = false;

    char buffer[1500];

    //set of socket descriptors
    fd_set readfds;

    //base json object
    json msg = { {"count", 0}, {"client", "none"} };

    setupServer(max_clients, client_socket, master_socket, opt, address, i, addrlen);

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //socket descriptor
        sd = client_socket[sdTurn];
        std::cout << "The SD for this turn is: " << sd << '\n';

        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET( sd , &readfds);

        //highest file descriptor number, need it for the select function
        if(sd > max_sd)
            max_sd = sd;

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely

        struct timeval tv = {0, 500000}; // Half a second
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("timed out");
            return activity;
        }
        //MASTER_SOCKET IS TOUCHED
        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , sid is %d , ip is : %s , port : %d  \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            //send new connection greeting message
            if( send(new_socket, msg.dump().c_str(), strlen(msg.dump().c_str()), 0) != int(strlen(msg.dump().c_str())) )
            {
                perror("send");
            }

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
        }

        //CHILD_SOCKET IS TOUCHED
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1499)) == 0 || dConnect !=0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Client %d disconnected , ip %s , port %d \n" ,
                          sd, inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                    if ( sdTurn == 0){
                        sdTurn=1;
                    }
                    else if ( sdTurn == 1){
                        sdTurn=0;
                    }

                    dConnect++;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';

                    /*
                    take buffer and save it somewhere
                    modify msg
                    send modified msg

                    */

                    char oldBuffer[1500];
                    strcpy(oldBuffer, buffer);
                    string clientStr;
                    clientStr.append(oldBuffer);
                    json clientResponse = json::parse(clientStr);

                    msg.at("client") = clientResponse.at("client");
                    msg.at("count") = clientResponse.at("count");

                    strcpy(buffer, msg.dump().c_str());

                    cout << "Test Print: \nsid = "
                         << sd
                         << " \ncurrent sdTurn = "
                         << sdTurn
                         << " : \nbuffer = "
                         << oldBuffer
                         << " : \nmsg = \n"
                         << msg.dump(4)
                         << endl;

                    send(sd , buffer , strlen(buffer) , 0 );

                    if (sdTurn == 0){
                        sdTurn=1;
                    }
                    else if (sdTurn == 1){
                        sdTurn=0;
                    }
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
