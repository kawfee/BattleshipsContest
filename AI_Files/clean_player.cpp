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
#include "socket_defs.h"

using namespace std;
using json=nlohmann::json;

void messageHandler(json &msg, string &clientID, int &round);
void updateBoard(char board[10][10], int row, int col, int length, Direction dir, char newChar);
void placeShip(json &msg);
void shootShot(json &msg);
void shotReturned(json &msg, string clientID);
void wipeBoards();
void sendGameVars(json &msg);
int  socketConnect(int sock, const char *socket_name);
int  socketOpen(const char *socket_name);
void socketClose(int sock);


struct container{
    int  boardSize=10;
    int  shipLengths[6];
    char shotBoard[10][10];
    char shipBoard[10][10];
    int  scanRow=0;
    int  scanCol=0;
    int  maxShipSize = 4;
};
container gameVars;

int main(int argc, char *argv[]){
    auto waste=0;

    srand(getpid());
    string clientID = "clean_player";

    //setup a socket and connection tools
    const char *path = "./serversocket";
    int clientSd = socketOpen(path);

    //create a message buffer
    char buffer[1500];

    //declare variables for during the game
    int round=0;

    //populate boards
    wipeBoards();
    for(int i=0;i<6;i++)  {
        gameVars.shipLengths[i] = 0;
    }
    
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

        messageHandler(msg, clientID, round);

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

void messageHandler(json &msg, string &clientID, int &round){
    if(msg.at("messageType")=="setupGame"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        sendGameVars(msg);
    }else if(msg.at("messageType")=="matchOver"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        wipeBoards();
        for(int i=0;i<6;i++)  {
            gameVars.shipLengths[i] = 0;
        }
    }else if(msg.at("messageType")=="placeShip"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        placeShip(msg);
    }else if(msg.at("messageType")=="shootShot"){
        msg.at("client") = clientID;
        msg.at("count") = round;
        shootShot(msg);
    }else if (msg.at("messageType")=="shotReturn"){
        shotReturned(msg, clientID);
    }else if(msg.at("messageType")=="shipDied"){
        updateBoard(gameVars.shipBoard, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), KILL);
    }else if (msg.at("messageType")=="killedShip"){
        updateBoard(gameVars.shotBoard, msg.at("row"), msg.at("col"), msg.at("length"), msg.at("dir"), KILL);
    }
    
}

void wipeBoards(){
    for(int row=0;row<gameVars.boardSize;row++){
        for(int col=0;col<gameVars.boardSize;col++){
            gameVars.shipBoard[row][col]=WATER;
            gameVars.shotBoard[row][col]=WATER;
        }
    }
}

void placeShip(json &msg){
    int shipLength = msg.at("length");
    for(int i=0;i<6;i++){
        if(gameVars.shipLengths[i]==0){
            gameVars.shipLengths[i]=shipLength;
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
                if(gameVars.shipBoard[randRow][randCol+len]!=WATER){
                    goodShip = false;
                }
            } else{
               if(gameVars.shipBoard[randRow+len][randCol]!=WATER){
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
    updateBoard(gameVars.shipBoard, randRow, randCol, msg.at("length"), randDir, SHIP);
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

void shotReturned(json &msg, string clientID){
    if(msg.at("client") == clientID){
        int tempRow = msg.at("row");
        int tempCol = msg.at("col");
        string tempResult = msg.at("str");
        if(tempResult.c_str()[0] == HIT){
            gameVars.shotBoard[tempRow][tempCol]=HIT;
        }else if(tempResult.c_str()[0] == MISS){
            gameVars.shotBoard[tempRow][tempCol]=MISS;
        }
    }else{
        //do nothing
        //unless... ?
    }
}

void sendGameVars(json &msg){
    msg.at("str") = "Joey Gorski"; // Your author name(s) here
}


void shootShot(json &msg);
void getMove(int &shotRow, int &shotCol);
void getFollowUpShot(int &row, int &col);
bool search(int &row, int &col, int rowDelta, int colDelta);
bool isOnBoard( int row, int col );
void scan(int &row, int &col);
void ensureMaxShipLength();

void shootShot(json &msg){
    int shotRow=0, shotCol=0;
    getMove(shotRow, shotCol);
    msg.at("row") = shotRow;
    msg.at("col") = shotCol;
    updateBoard(gameVars.shotBoard, shotRow, shotCol, 1, NONE, SHOT);
}

void getMove(int &shotRow, int &shotCol){
    shotRow=gameVars.scanRow;
    shotCol=gameVars.scanCol;

    if(gameVars.shotBoard[gameVars.scanRow][gameVars.scanCol]==HIT){
        getFollowUpShot(shotRow, shotCol);
    }else{
        scan(gameVars.scanRow, gameVars.scanCol);
        shotRow=gameVars.scanRow;
        shotCol=gameVars.scanCol;
    }
}

void getFollowUpShot(int &row, int &col){
    ensureMaxShipLength();
    if(search(row, col, -1, 0)){
        return;
    }else if(search(row, col, 1, 0)){
        return;
    }else if(search(row, col, 0, 1)){
        return;
    }else if(search(row, col, 0, -1)){
        return;
    }else{
        scan(row, col);
    }
}

bool search(int &row, int &col, int rowDelta, int colDelta){
    for(int range=1; range<=gameVars.maxShipSize; range++){
		int r=row+rowDelta*range;
		int c=col+colDelta*range;

		if( ! isOnBoard(r,c)){
			return false;
		}else if( gameVars.shotBoard[r][c] == WATER ) {
			row=r; col=c;
			return true;
		}else if( gameVars.shotBoard[r][c] == MISS || gameVars.shotBoard[r][c] == KILL ){
			return false;
		}else{ 
            //	If it is a hit, just keep running through loop.
		}
    }
    return false;	// Guess we couldn't find anything.
}

bool isOnBoard( int row, int col ) {
    if( row>=0 && row<gameVars.boardSize && col>=0 && col<gameVars.boardSize ){
        return true;
    }else{
        return false;
    }
}

void scan(int &row, int &col){
    gameVars.scanCol = gameVars.scanCol + gameVars.maxShipSize;
    if( gameVars.scanCol >= gameVars.boardSize ) {
	    gameVars.scanCol = gameVars.scanCol % gameVars.boardSize;
        // if boardSize is multiple of column, could get caught going down columns. 
        // Adjust if needed.
        if( gameVars.boardSize % gameVars.maxShipSize == 0 ) {	
            if( gameVars.scanCol + 1 == gameVars.maxShipSize ) {
                gameVars.scanCol = 0;
            } else {
                gameVars.scanCol++;
            }
        }
        gameVars.scanRow++;
        if( gameVars.scanRow >= gameVars.boardSize ) {
            gameVars.scanRow = 0;
        }
    }
}

void ensureMaxShipLength(){
    for(int i=0;i<6;i++){
        if(gameVars.shipLengths[i]>gameVars.maxShipSize){
            gameVars.maxShipSize=gameVars.shipLengths[i];
        }
    }
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