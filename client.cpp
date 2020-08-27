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
#include "defines.h"

using namespace std;
using json=nlohmann::json;

void messageHandler(json &msg, string &clientID, int &round, char shipBoard[10][10], char shotBoard[10][10], int boardSize);
void updateBoard(char board[10][10], int row, int col, int length, Direction dir, char newChar);
void placeShip(json &msg, char shipBoard[10][10], int boardSize);
void shootShot(json &msg, char shotBoard[10][10], int boardSize);



int main(int argc, char *argv[]){
    srand(getpid());
    string clientID = to_string(getpid());

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
        cout<<"Error connecting client to socket!"<<endl;
    }else{
    cout << "Connected client to the server!" << endl;
    }

    //declare variables for during the game
    int round=0;
    int boardSize=10;
    char shipBoard[10][10];
    char shotBoard[10][10];

    //populate boards
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            shipBoard[row][col]=WATER;
            shotBoard[row][col]=WATER;
        }
    }
    while(1){
        round++;

        //read
        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        recv(clientSd, (char*)&buffer, sizeof(buffer), 0);

        string tempStr="";
        tempStr.append(buffer);
        //strcpy(tempStr, buffer);

        json msg=json::parse(tempStr);

        messageHandler(msg, clientID, round, shipBoard, shotBoard, boardSize);

        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        if( msg.dump().length()+1 > 1500 ){
            cout << "UNSUPPORTED LENGTH REACHED BY: msg" << endl;
        }
        strcpy(buffer, msg.dump().c_str());
        send(clientSd, (char*)&buffer, strlen(buffer), 0);
    }

    close(clientSd);
    cout << "Connection closed for client " << clientID << endl;

    return 0;
}

void messageHandler(json &msg, string &clientID, int &round, char shipBoard[10][10], char shotBoard[10][10], int boardSize){
    msg.at("client") = clientID;
    msg.at("count") = round;

    if(msg.at("messageType")=="placeShip"){
        placeShip(msg, shipBoard, boardSize);
    }else if(msg.at("messageType")=="shootShot"){
        shootShot(msg, shotBoard, boardSize);
    }else if(msg.at("messageType")=="shipDead"){
        //shipDied(msg, shotBoard, boardSize);
    }
}

void placeShip(json &msg, char shipBoard[10][10], int boardSize){
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            if(shipBoard[row][col]==WATER){
                msg.at("row") = row;
                msg.at("col") = col;
                msg.at("dir") = HORIZONTAL;
                for(int len=0;len<msg.at("length");len++){
                    if(shipBoard[row][col]!=WATER){
                        msg.at("dir") = VERTICAL;
                        updateBoard(shipBoard, row, col, msg.at("length"), VERTICAL, SHIP);
                        return;
                    }
                }
                updateBoard(shipBoard, row, col, msg.at("length"), HORIZONTAL, SHIP);
                return;
            }
        }
    }
}

void shootShot(json &msg, char shotBoard[10][10], int boardSize){
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            if(shotBoard[row][col]==WATER){
                msg.at("row") = row;
                msg.at("col") = col;
                updateBoard(shotBoard, row, col, 1, NONE, SHOT);
                return;
            }
        }
    }
}

void updateBoard(char board[10][10], int row, int col, int length, Direction dir, char newChar){
    if(dir==HORIZONTAL){
        for(int len=0;len<length;len++){
            board[row][col+len]=newChar;
        }
    }else if(dir==VERTICAL){
        for(int len=0;len<length;len++){
            board[row+len][col]=newChar;
        }
    }else if(dir==NONE){
        board[row][col]=newChar;
    }
}
