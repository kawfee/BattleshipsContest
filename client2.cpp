#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <cstring>

#include "json.hpp"


using namespace std;
using json=nlohmann::json;

//Client side

int main(int argc, char *argv[]){

    //set random clientID
    string clientID = "Client2";

    const char *serverIp = "localhost"; int port = 54321;

    //create a message buffer
    char buffer[1500];

    //setup a socket and connection tools
    struct hostent* host = gethostbyname(serverIp);
    sockaddr_in sendSockAddr;

    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));

    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);

    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0){
        cout<<"Error connecting to socket!"<<endl;
    }else{
    cout << "Connected to the server!" << endl;
    }

    int round=0;
    while(1){
        round++;

        //read
        cout << "Awaiting server response..." << endl;
        memset(&buffer, 0, sizeof(buffer));//clear the buffer
        recv(clientSd, (char*)&buffer, sizeof(buffer), 0);
        if(!strcmp(buffer, "exit")) {
            cout << "Server has quit the session" << endl;
            break;
        }
        cout << "Server: " << buffer << endl;

        //write
        cout << "Client: ";

        string tempStr="";
        tempStr.append(buffer);
        //strcpy(tempStr, buffer);

        json msg=json::parse(tempStr);

        msg.at("client") = clientID;
        msg.at("count") = round;

        //std::cout << "Testing JSON contents: " <<msg[0] << '\n';

        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        if( msg.dump().length()+1 > 1500 ){
            cout << "UNSUPPORTED LENGTH REACHED BY: msg" << endl;
        }
        strcpy(buffer, msg.dump().c_str());
        send(clientSd, (char*)&buffer, strlen(buffer), 0);
    }

    close(clientSd);
    cout << "Connection closed" << endl;

    return 0;
}
