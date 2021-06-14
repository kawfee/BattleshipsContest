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
void shootShot(json &msg);
void shotReturned(json &msg, string clientID);
void wipeBoards();
void sendGameVars(json &msg);
int  socketConnect(int sock, const char *socket_name);
int  socketOpen(const char *socket_name);
void socketClose(int sock);

void updateImpossibles();

struct container{
    int  boardSize=10;
    int  shipLengths[6];
    char shotBoard[10][10];
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
        gameVars.shotBoard[tempRow][tempCol]=tempResult.c_str()[0];
    }else{
        //do nothing
        //unless... ?
    }
}

void sendGameVars(json &msg){
    msg.at("str") = "Matthew Bouch and Joey Gorski"; // Your author name(s) here
}






void shootShot(json &msg);
void getMove(int &shotRow, int &shotCol);
void getFollowUpShot(int &row, int &col);
bool search(int &row, int &col, int rowDelta, int colDelta);
bool isOnBoard( int row, int col );
void findTarget(int &row, int &col);
bool findUnkilledShips(int &theRow, int &theCol);
void ensureMaxShipLength();
bool isValid(int row, int col);





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

    if(gameVars.shotBoard[shotRow][shotCol]==HIT){
        getFollowUpShot(shotRow, shotCol);
    }else if(findUnkilledShips(gameVars.scanRow, gameVars.scanCol)){
        getFollowUpShot(gameVars.scanRow, gameVars.scanCol);    
        shotRow=gameVars.scanRow;
        shotCol=gameVars.scanCol;
    }else{
        findTarget(gameVars.scanRow, gameVars.scanCol);
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
        findTarget(row, col);
    }
}

bool search(int &row, int &col, int rowDelta, int colDelta){
    for(int range=1; range<=gameVars.maxShipSize+1; range++){
		int r=row+rowDelta*range;
		int c=col+colDelta*range;

		if( ! isOnBoard(r,c)){
			return false;
		}else if( gameVars.shotBoard[r][c] == WATER ) {
			row=r; col=c;
			return true;
		}else if(   
                       gameVars.shotBoard[r][c] == MISS 
                    || gameVars.shotBoard[r][c] == KILL 
                    || gameVars.shotBoard[r][c] == DUPLICATE_HIT 
                    || gameVars.shotBoard[r][c] == DUPLICATE_SHOT 
                ){
			return false;
		}
    }
    return false;
}

bool isOnBoard( int row, int col ) {
    if( row>=0 && row<gameVars.boardSize && col>=0 && col<gameVars.boardSize ){
        return true;
    }else{
        return false;
    }
}


/*
    struct container{
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

void findTarget(int &targetRow, int &targetCol){
    bool efficientShot[10][10];
    for(int i=0; i<gameVars.boardSize;i++){
        for(int j=0; j<gameVars.boardSize;j++){
            //gameVars.percentageBoard[i][j] = 0;
            if( (i%3) - (j%3) == 0 ) {
                efficientShot[i][j] = true;
            } else {
                efficientShot[i][j] = false;
            }
        }
    }
    int bestRow = 0;
    int bestCol = 0;
    int largestChance = 0;
    bool whileTrue = true;
        for(int row = 0; row < gameVars.boardSize; row++){
            for(int col = 0; col < gameVars.boardSize; col++){
                if(gameVars.shotBoard[row][col]!=WATER){
                    //gameVars.percentageBoard[row][col]-=200;
                }
                for(int i = 0; i < 2; i++){
                    whileTrue = true;
                    // Horizontal
                    if( i == 0){
                        /*for(int addCols = 0; addCols < 3; addCols++){
                            if( isValid(row, (col + addCols)) == true ){
                                gameVars.percentageBoard[row][(col + addCols)]++;
                            }
                        }*/
                        for(int addCols = 0; addCols < 3; addCols++){
                            if( isValid(row, (col + addCols)) == false ){
                                whileTrue = false;
                            }
                        }
                        if (whileTrue == true){
                            for(int addCols = 0; addCols < 3; addCols++){
                                gameVars.percentageBoard[row][(col + addCols)]++;
                            }
                        }
                    }
                    // Vertical
                    else{
                        /*for(int addRows = 0; addRows < 3; addRows++){
                            if( isValid((row + addRows), col) == true){
                                gameVars.percentageBoard[(row + addRows)][col]++;
                            }
                        }*/
                        for(int addRows = 0; addRows < 3; addRows++){
                            if( isValid((row + addRows), col) == false){
                                whileTrue = false;
                            }
                        }
                        if (whileTrue == true){
                            for(int addRows = 0; addRows < 3; addRows++){
                                gameVars.percentageBoard[(row + addRows)][col]++;
                            }
                        }
                    }
                }
            }
        }
        for(int row = 0; row < gameVars.boardSize; row++){
            for(int col = 0; col < gameVars.boardSize; col++){
                if ( gameVars.percentageBoard[row][col] >= largestChance && efficientShot[row][col]){
                    largestChance = gameVars.percentageBoard[row][col];
                    bestRow = row;
                    bestCol = col;
                }
            }
        }
        targetRow = bestRow;
        targetCol = bestCol;
    
}


void updateImpossibles() {
    bool horizPass=true;
    bool vertPass=true;
    for (int row = 0; row<gameVars.boardSize; row++) {
        for (int col = 0; col<gameVars.boardSize; col++) {
            if ( (! isValid(row-1, col)) && ((! isValid(row+2,col)) || (!isValid(row+1, col))) ) {
                vertPass = false;
            }
            if ( (! isValid(row, col-1)) && ((! isValid(row, col+2)) || (!isValid(row, col+1))) ) {
                horizPass = false;
            }
            if ( (! vertPass) && (! horizPass) ) {
                gameVars.shotBoard[row][col] = MISS;
                gameVars.percentageBoard[row][col] = -1;
            }
            vertPass = true;
            horizPass = true;
        }
    }
}

bool isValid(int row, int col) {
    if ( ( row >= 0 && row < gameVars.boardSize) && ( col >= 0 && col < gameVars.boardSize)){
        if( gameVars.shotBoard[row][col] == WATER ){
            return true;
        }
    }
    return false;
}

bool findUnkilledShips(int &theRow, int &theCol){
    for(int row=0;row<gameVars.boardSize;row++){
        for(int col=0;col<gameVars.boardSize;col++){
            if(gameVars.shotBoard[row][col]==HIT || gameVars.shotBoard[row][col]==DUPLICATE_HIT){
                theRow=row;
                theCol=col;
                return true;
            }
        }
    }
    return false;
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