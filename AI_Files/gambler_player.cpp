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

#include "../json.hpp"
#include "../defines.h"
#include "../socket_defs.h"

using namespace std;
using json=nlohmann::json;

void messageHandler(json &msg, string &clientID, int &round, char (&shipBoard)[10][10], char (&shotBoard)[10][10], int boardSize);
void updateBoard(char board[10][10], int row, int col, int length, Direction dir, char newChar);
void placeShip(json &msg, char shipBoard[10][10], int boardSize);
void shootShot(json &msg, char shotBoard[10][10], int boardSize);
void shotReturned(json &msg, string clientID, char shotBoard[10][10]);
void wipeBoards(char (&shipBoard)[10][10], char (&shotBoard)[10][10], int boardSize);
void doCalculatedShot(int &returnRow, int &returnCol, int boardSize, char shotBoard[10][10]);
void sendGameVars(json &msg);
int socketConnect(int sock, const char *socket_name);
int socketOpen(const char *socket_name);
void socketClose(int sock);

int shipLengths[6] = {0,0,0,0,0,0};


int main(int argc, char *argv[]){
    auto waste=0;

    srand(getpid());
    string clientID = "gambler_player";

    //setup a socket and connection tools
    const char *path = "./serversocket";
    int clientSd = socketOpen(path);

    //create a message buffer
    char buffer[1500];

    //declare variables for during the game
    int round=0;
    int boardSize=10;
    char shipBoard[10][10];
    char shotBoard[10][10];

    //populate boards
    wipeBoards(shipBoard, shotBoard, boardSize);
    
    while(1){
        round++;

        //read
        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        //recv(clientSd, (char*)&buffer, sizeof(buffer), 0);
        waste=read( clientSd , buffer, 1499);

        string tempStr="";
        tempStr.append(buffer);
        //strcpy(tempStr, buffer);

        json msg=json::parse(tempStr);

        messageHandler(msg, clientID, round, shipBoard, shotBoard, boardSize);

        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        if( msg.dump().length()+1 > 1500 ){
            cerr << "UNSUPPORTED LENGTH REACHED BY: msg" << endl;
        }
        strcpy(buffer, msg.dump().c_str());
        send(clientSd, (char*)&buffer, strlen(buffer), 0);
    }

    close(clientSd);
    cerr << "Connection closed for client " << clientID << endl;

    if(waste){
        //do nothing--this is just to stop a warning from popping up.
    }
    return 0;
}

void messageHandler(json &msg, string &clientID, int &round, char (&shipBoard)[10][10], char (&shotBoard)[10][10], int boardSize){
    if(msg.at("messageType")=="setupGame"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        sendGameVars(msg);
    }else if(msg.at("messageType")=="matchOver"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        wipeBoards(shipBoard, shotBoard, boardSize);
    }else if(msg.at("messageType")=="placeShip"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        placeShip(msg, shipBoard, boardSize);
    }else if(msg.at("messageType")=="shootShot"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        shootShot(msg, shotBoard, boardSize);
    }else if (msg.at("messageType")=="shotReturn"){
        shotReturned(msg, clientID, shotBoard);
    }else if(msg.at("messageType")=="shipDied"){
        updateBoard(shipBoard, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), KILL);
    }else if (msg.at("messageType")=="killedShip"){
        updateBoard(shotBoard, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), KILL);
    }
    
}

void wipeBoards(char (&shipBoard)[10][10], char (&shotBoard)[10][10], int boardSize){
    for(int row=0;row<boardSize;row++){
        for(int col=0;col<boardSize;col++){
            shipBoard[row][col]=WATER;
            shotBoard[row][col]=WATER;
        }
    }
}

void placeShip(json &msg, char shipBoard[10][10], int boardSize){
    int shipLength = msg.at("length");
    for(int i=0;i<6;i++){
        if(shipLengths[i]==0){
            shipLengths[i]=shipLength;
            break;
        }
    }
    int randBorder = 10 - shipLength;
    int randCol = rand() % randBorder;
    int randRow = rand() % randBorder; 
    Direction randDir = Direction(rand() % 2 + 1);
    bool goodShip = false;
    while(true){
        goodShip = true;
        for(int len=0; len<shipLength; len++){
            if(randDir == HORIZONTAL){
                if(shipBoard[randRow][randCol+len]!=WATER){
                    goodShip = false;
                }
            } else{
               if(shipBoard[randRow+len][randCol]!=WATER){
                    goodShip = false;
                } 
            }
        }
        if(goodShip){
           break; 
        }
        else {
            randCol = rand() % randBorder;
            randRow = rand() % randBorder;
            randDir = Direction(rand() % 2 + 1);
        }
    }
    msg.at("row") = randRow;
    msg.at("col") = randCol;
    msg.at("dir") = randDir;
    //cout << "SHIP DATA: ROW: " << randRow << " COL: " << randCol << " DIR: " << randDir << endl;
    updateBoard(shipBoard, randRow, randCol, msg.at("length"), randDir, SHIP);
}

void shootShot(json &msg, char shotBoard[10][10], int boardSize){
    int row=0, col=0;
    doCalculatedShot(row, col, boardSize, shotBoard);
    msg.at("row") = row;
    msg.at("col") = col;
    updateBoard(shotBoard, row, col, 1, NONE, SHOT);
    return;
    // for(int row=0;row<boardSize;row++){
    //     for(int col=0;col<boardSize;col++){
    //         if(shotBoard[row][col]==WATER){
    //             msg.at("row") = row;
    //             msg.at("col") = col;
    //             updateBoard(shotBoard, row, col, 1, NONE, SHOT);
    //             return;
    //         }
    //     }
    // }
}

void doCalculatedShot(int &returnRow, int &returnCol, int boardSize, char shotBoard[10][10]){ //shipLengths
    int ship=0;
    int percentageBoard[10][10];

    // FIX ME PLEASE I'M TRASH

    while(ship<6){
        int len=shipLengths[ship];

        for(int row=0; row<boardSize-len; row++){
            for(int col=0; col<boardSize; col++){
                for(int shipLen=0; shipLen<len; shipLen++){
                    percentageBoard[row+shipLen][col]++;
                    if(shotBoard[row][col]==HIT){
                        if(col<boardSize-1)         percentageBoard[row+shipLen][col+1]+=1000;
                        if(row+shipLen<boardSize-1) percentageBoard[row+shipLen+1][col]+=1000;
                        if(col>0)                   percentageBoard[row+shipLen][col-1]+=1000;
                        if(row+shipLen>0)           percentageBoard[row+shipLen-1][col]+=1000;
                    }
                }
            }
        }
        for(int row=0; row<boardSize; row++){
            for(int col=0; col<boardSize-len; col++){
                for(int shipLen=0; shipLen<len; shipLen++){
                    percentageBoard[row][col+shipLen]++;
                    if(shotBoard[row][col]==HIT){
                        if(col+shipLen<boardSize-1) percentageBoard[row][col+shipLen+1]+=1000;
                        if(row<boardSize-1)         percentageBoard[row+1][col+shipLen]+=1000;
                        if(col+shipLen>0)           percentageBoard[row][col+shipLen-1]+=1000;
                        if(row>0)                   percentageBoard[row-1][col+shipLen]+=1000;
                    }
                }
            }
        }

        ship++;
    }
    
    int bestScore = -1;
    for(int row=0; row < boardSize; row++){
        for(int col=0; col<boardSize; col++){
            if((percentageBoard[row][col] > bestScore) && (shotBoard[row][col] == WATER)){
                bestScore = percentageBoard[row][col];
                returnRow = row;
                returnCol = col;
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

void shotReturned(json &msg, string clientID, char shotBoard[10][10]){
    // Do something with the message data here. 
    //cout << msg.dump(4) << endl;

    if(msg.at("client") == clientID){
        // do nothing
    }else{
        //updateBoard(board[10][10], row, col, length, dir, newChar)
        int tempRow = msg.at("row");
        int tempCol = msg.at("col");
        string tempResult = msg.at("str");
        if(tempResult.c_str()[0] == HIT){
            //cout << "Hello World" << endl;
            shotBoard[tempRow][tempCol]=HIT;
        }else if(tempResult.c_str()[0] == MISS){
            //cout << "Hello World2" << endl;
            shotBoard[tempRow][tempCol]=MISS;
        }
    }
}

void sendGameVars(json &msg){
    msg.at("str") = "Joey Gorski and Matthew Bouch"; // Your author name(s) here
}

























int socketConnect(int sock, const char *socket_name){
    struct sockaddr_un address;

    memset(&address, 0x00, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socket_name, SOCKET_NAME_MAX_LEN-1);

    return connect(sock, (struct sockaddr *)&address, sizeof(address));
}

int socketOpen(const char *socket_name){
    int res;

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    res = socketConnect(sock, socket_name);
    if (res < 0) {
        close(sock);
        return res;
    }
    
    return sock;
}

void socketClose(int sock){
    if (sock > 0)
        close(sock);
}