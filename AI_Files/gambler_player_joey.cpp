/**
 * @author Matthew Bouch, Joey Gorski, Stefan Brandle, Jonathan Geisler
 * @date January, 2021
 * 
 * 
 * 
 */

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
bool canPlaceShip(int row,int col,Direction dir,int length, bool canTouch);
void findSafest(int length, Direction& theDir, int& theRow, int& theCol);
void shootShot(json &msg);
void shotReturned(json &msg, string clientID);
void wipeBoards();
void sendGameVars(json &msg);
int  socketConnect(int sock, const char *socket_name);
int  socketOpen(const char *socket_name);
void socketClose(int sock);


struct container{
    int  round=0;
    int  boardSize=10;
    int  shipLengths[6];
    char shotBoard[10][10];
    int  opponentShotStatBoard[10][10];
    int  percentageBoard[10][10];
    char shipBoard[10][10];
    int  scanRow=0;
    int  scanCol=0;
    int  maxShipSize = 4;
};
container gameVars;

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

    //populate boards
    wipeBoards();
    for(int i=0;i<6;i++)  {
        gameVars.shipLengths[i] = 0;
    }
    
    while(1){
        round++;

        //read
        memset(&buffer, 0, sizeof(buffer));//clear the buffer

        waste=read( clientSd , buffer, 1499);

        string tempStr="";
        tempStr.append(buffer);

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
        gameVars.round=round;
        wipeBoards();
        for(int i=0;i<6;i++)  {
            gameVars.shipLengths[i] = 0;
        }
        gameVars.scanRow=0;
        gameVars.scanCol=0;
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
            gameVars.percentageBoard[row][col]=0;
            gameVars.shotBoard[row][col]=WATER;
        }
    }
}

void placeShip(json &msg){
    int length = msg.at("length");
    for(int i=0;i<6;i++){
        if(gameVars.shipLengths[i]==0){
            gameVars.shipLengths[i]=length;
            break;
        }
    }

    int row=0, col=0, counter=0;
    bool roundNice=false;
    Direction dir = Direction(rand()%2+1);
    if(gameVars.round<69){
        while( true ) {
            counter++;
            dir = Direction(rand()%2+1);
            if(dir == HORIZONTAL) {
                row = rand()%gameVars.boardSize;
                col = rand()%(gameVars.boardSize-length+1);
            } else {
                col = rand()%gameVars.boardSize;
                row = rand()%(gameVars.boardSize-length+1);
            }
            if( canPlaceShip( row,col,dir,length,roundNice ) ) {
                msg.at("row") = row;
                msg.at("col") = col;
                msg.at("dir") = dir;
                updateBoard(gameVars.shipBoard, row, col, length, dir, SHIP);
            }
            if(counter>69){
                roundNice=true;
            }
        }
    } else {
        while(true) {
            counter++;
            findSafest(length, dir, row, col);
            if( canPlaceShip( row,col, dir, length, roundNice) ) {
                msg.at("row") = row;
                msg.at("col") = col;
                msg.at("dir") = dir;
                updateBoard(gameVars.shipBoard, row, col, length, dir, SHIP);
            }
            if(counter>69) {
                roundNice = true;
            }
        }
    }
}

bool canPlaceShip(int row,int col,Direction dir,int length, bool canTouch){
    if(dir == HORIZONTAL) {
        if(row<0 || row >= gameVars.boardSize) return false;
        if(col<0 || col >  gameVars.boardSize-length) return false;
        if(!canTouch && col>0 && gameVars.shotBoard[row][col-1]==SHIP) return false;
        for(int c=col;c<col+length;c++){
            if(gameVars.shotBoard[row][c]!=WATER) return false;
            if(!canTouch && gameVars.shotBoard[row-1][c]==SHIP) return false;
            if(!canTouch && gameVars.shotBoard[row+1][c]==SHIP) return false;
        }
        if(!canTouch && col+length+1<gameVars.boardSize && gameVars.shotBoard[row][col+length+1]==SHIP) return false;

    } else if(dir == VERTICAL) {
        if(col<0 || col >=  gameVars.boardSize) return false;
        if(row<0 || row > gameVars.boardSize-length) return false;
        if(!canTouch && row>0 && gameVars.shotBoard[row-1][col]==SHIP) return false;
        for(int r=row;r<row+length;r++){
            if(gameVars.shotBoard[r][col]!=WATER) return false;
            if(!canTouch && gameVars.shotBoard[r][col+1]==SHIP) return false;
            if(!canTouch && gameVars.shotBoard[r][col-1]==SHIP) return false;
        }
        if(!canTouch && row+length+1<gameVars.boardSize && gameVars.shotBoard[row+length+1][col]==SHIP) return false;
    }
    return true;
}

void findSafest(int length, Direction& theDir, int& theRow, int& theCol) {
    int hitAmount = 0, minHit = 10000;
    bool broke=false;
    // horizontal
    for(int row=0; row<gameVars.boardSize; row++) {
        for(int col=0; col<gameVars.boardSize-length+1; col++) {
            hitAmount=0;
            broke=false;
            for(int c=0; c<length; c++) {
                if(col+c<gameVars.boardSize && gameVars.shotBoard[row][col+c] == WATER) {   //not past the board
                    hitAmount+=gameVars.opponentShotStatBoard[row][col+c];
                }else{
                    broke=true;
                    break;
                }
            }
            if(!broke && hitAmount<minHit) {
                minHit = hitAmount;
                theRow = row;
                theCol = col;
                theDir = HORIZONTAL;
            }
        }
    }
    // VERTICAL
    for(int col=0; col<gameVars.boardSize; col++) {
        for(int row=0; row<gameVars.boardSize-length+1; row++) {
            hitAmount = 0;
            broke=false;
            for(int r=0; r<length; r++) {
                if(row+r<gameVars.boardSize && gameVars.shotBoard[row+r][col] == WATER) {   //not past the board
                    hitAmount+=gameVars.opponentShotStatBoard[row+r][col];
                }else{
                    broke=true;
                    break;
                }
            }
            if(!broke && hitAmount<minHit) {
                minHit = hitAmount;
                theRow = row;
                theCol = col;
                theDir = VERTICAL;
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

void shotReturned(json &msg, string clientID){
    if(msg.at("client") == clientID){
        int tempRow = msg.at("row");
        int tempCol = msg.at("col");
        string tempResult = msg.at("str");
        gameVars.shotBoard[tempRow][tempCol]=tempResult.c_str()[0];
    }else{
        //do nothing
        //unless... ?
        int tempRow = msg.at("row");
        int tempCol = msg.at("col");
        gameVars.opponentShotStatBoard[tempRow][tempCol]++;
    }
}

void sendGameVars(json &msg){
    msg.at("str") = "Matthew Bouch and Joey Gorski"; // Your author name(s) here
}






void shootShot(json &msg);
void getMove(int &shotRow, int &shotCol);
int calcValue(int row, int col);




void shootShot(json &msg){
    int shotRow=0, shotCol=0;
    getMove(shotRow, shotCol);
    while(gameVars.shotBoard[shotRow][shotCol]!=WATER){
        getMove(shotRow, shotCol);
    }
    msg.at("row") = shotRow;
    msg.at("col") = shotCol;
    //updateBoard(gameVars.shotBoard, shotRow, shotCol, 1, NONE, SHOT);
}

void getMove(int &shotRow, int &shotCol){
    shotRow=gameVars.scanRow;
    shotCol=gameVars.scanCol;


    int value=-1,bestValue=-1;
    int bestRow=0, bestCol=0;

    for(int r=0; r<gameVars.boardSize; r++){
        for(int c=0; c<gameVars.boardSize; c++){
            value=calcValue(r, c);
            if(value>=bestValue){
                bestValue=value;
                bestRow=r;
                bestCol=c;
            }
        }
    }

    shotRow=bestRow;
    shotCol=bestCol;
}

/*
    gameVars{
        int  boardSize=10;
        int  shipLengths[6];
        char shotBoard[10][10];
        int  percentageBoard[10][10];
        char shipBoard[10][10];
        int  scanRow=0;
        int  scanCol=0;
        int  maxShipSize = 4;
    };
*/

//shotBoard
//
int calcValue(int row, int col){
    int val = 0;
    int valIncrease=50;
    if(gameVars.shotBoard[row][col]!= WATER){
        return -1;
    }
    for(int c=col; c>=0 && c>col-4; c--){
        char res = gameVars.shotBoard[row][c];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }
    for(int c=col; c<gameVars.boardSize && c<col+4; c++){
        char res = gameVars.shotBoard[row][c];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }

    for(int r=row; r>=0 && r>row-4; r--){
        char res = gameVars.shotBoard[r][col];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }
    for(int r=row; r<gameVars.boardSize && r<row+4; r++){
        char res = gameVars.shotBoard[r][col];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }

    // ONLY FOR LEARNING GAMBLER
    // return val + opponentShipStatBoard[row][col]/30;
    return val;
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