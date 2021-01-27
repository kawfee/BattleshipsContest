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
void shotReturned(json &msg, string clientID, char shotBoard[10][10]);
void shootShot(json &msg, char shotBoard[10][10], int boardSize);
void wipeBoards(char (&shipBoard)[10][10], char (&shotBoard)[10][10], int boardSize);
void sendGameVars(json &msg);
int socketConnect(int sock, const char *socket_name);
int socketOpen(const char *socket_name);
void socketClose(int sock);




// make some global structure for game vars to be accessed wherever they're needed
struct container{
    int shipLengths[6];
    int cellWeights[10][10];
    int lastRow=0;
    int lastCol=0;
    int shipMaxLength=3;
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
    int boardSize=10;
    char shipBoard[10][10];
    char shotBoard[10][10];

    // access things with gameVars.shipLengths for example
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            gameVars.cellWeights[i][j] = -1;
        }
    }
    for(int i=0;i<6;i++)  {
        gameVars.shipLengths[i] = 0;
    }

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
        for(int i = 0; i < 10; i++){
            for(int j = 0; j < 10; j++){
                gameVars.cellWeights[i][j] = -1;
            }
        }
        gameVars.lastRow = 0;
        gameVars.lastCol = 0;
        for(int i=0;i<6;i++)  {
            gameVars.shipLengths[i] = 0;
        }
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
    updateBoard(shipBoard, randRow, randCol, msg.at("length"), randDir, SHIP);
}









void doCalculatedShot(int shot[], char shotBoard[10][10], int boardSize);
void getFollowUpShot( int row, int col, int shot[], int boardSize, char board[10][10] );
bool search( int row, int col, int rowDelta, int colDelta, int shot[], int rangeLeft, int boardSize, char board[10][10]);
bool haveUnfinishedBusiness( int position[2], int boardSize, char board[10][10]);
void getNextProbe( int shot[], char board[10][10], int boardSize);
void printWeightedBoard( int board[10][10] );
void getMaxWeightCell( int board[10][10], int shot[2], int boardSize); 
int  calcCellWeight( int row, int col, int boardSize, char board[10][10] );
int  calcCellWeightVert( int row, int col, int length, char board[10][10], int boardSize);
int  calcCellWeightHoriz( int row, int col, int length, char board[10][10], int boardSize);
bool onBoard( int row, int col, int boardSize );
char getCellStatus( int row, int col, char board[10][10] ); 
void ensureMaxShipLength();
void reWeighBoard(int boardSize, char board[10][10]);





//shot logic shamelessly ripped from Dr Brandle and Dr Geisler--very little of it is modified, 
//and even then it's only modified to the point that it will work on a different backend.
void shootShot(json &msg, char shotBoard[10][10], int boardSize){
    int shot[] = {0,0};
    doCalculatedShot(shot, shotBoard, boardSize);
    msg.at("row") = shot[0];
    msg.at("col") = shot[1];
    return;
}

void doCalculatedShot(int shot[], char shotBoard[10][10], int boardSize){
    if( getCellStatus(gameVars.lastRow, gameVars.lastCol, shotBoard) == HIT ) {
	    getFollowUpShot( gameVars.lastRow, gameVars.lastCol, shot, boardSize, shotBoard);
    } else if( haveUnfinishedBusiness(shot, boardSize, shotBoard) ){
	    getFollowUpShot( shot[0], shot[1], shot, boardSize, shotBoard);
    } else {
        getNextProbe( shot, shotBoard, boardSize);
    }
    gameVars.lastRow = shot[0];
    gameVars.lastCol = shot[1];
    return;
    /*
        find where a ship might be and increase its spot's values.
        when you have a hit set a "terminate mode"
            start targeting in North, East, South, West order until you find another hit
            if you kill turn off terminate mode 
            else start going the opposite direction from the first hit recorded until you kill. 
            deal with ships in paralel somehow. 
        
        if you don't have terminate mode do a shot at a high value likely location.
        

        getShot():
            if getCellStatus(prevRow, prevCol) == HIT:
                getFollowUpShot()
            elif unfinishedBusiness():
                getFollowUpShot()
            else
                randShot();
    */
}

//***********************************************
// MAIN SHOT ROUTINES ***************************
//***********************************************

// Gets a probing shot. Grabs a cell matching the current max board cell weight.
void getNextProbe( int shot[], char board[10][10], int boardSize) {
    reWeighBoard( boardSize, board );
    getMaxWeightCell( gameVars.cellWeights, shot, boardSize);
}

// Used to follow up on a wounded ship.
// The code needs to be improved to use weights to determine best
// followup locations.
void getFollowUpShot( int row, int col, int shot[], int boardSize, char board[10][10] ) {

    // Starting to play around with weights. Code place-holder.
    // int nextRow = 0, nextCol = 0;
    // int weightUp = calcCellWeight( row-1, col, boardSize, board);
    // Done playing around with weights.  :-)

    ensureMaxShipLength();

    // Check for viable shot UP
    if( search(row, col, -1, 0, shot, gameVars.shipMaxLength, boardSize, board) == true ) {
	    return;
    }
    // Ok, nothing [left] to shot at UP. But was there at least something UP?
    // If so, it may be worth checking in the opposite direction might be faster.
    else if( onBoard(row-1, col, boardSize) && getCellStatus(row-1, col, board) == HIT ) {
        if( search( row, col, 1, 0, shot, gameVars.shipMaxLength-1, boardSize, board) == true ) {
            return;
        }
    }
    // Check for viable shot to RIGHT
    if ( search( row, col, 0, 1, shot, gameVars.shipMaxLength, boardSize, board) == true ) {
	    return;
    } 
    // Ok, nothing [left] to shot at to RIGHT. But was there at least something to RIGHT?
    // If so, it may be worth checking in the opposite direction might be faster.
    else if( onBoard(row, col+1, boardSize && getCellStatus(row, col+1, board) == HIT )) {
        if( search( row, col, 0, -1, shot, gameVars.shipMaxLength-1, boardSize, board) == true ) {
            return;
        }
    }
    // At this point, check DOWN and to LEFT, but don't do reverse checks because 
    // anything there will have been covered above.
    if ( search( row, col, 1, 0, shot, gameVars.shipMaxLength, boardSize, board) == true ) {
	    return;
    } 
    if ( search( row, col, 0, -1, shot, gameVars.shipMaxLength, boardSize, board) == true ) {
	    return;
    } else {
	    getNextProbe( shot, board, boardSize);
	return;
    }
}

// Partner to getFollowUpShot. Assists with recursive hunting for parts of the ship.
bool search( int row, int col, int rowDelta, int colDelta, int shot[], int rangeLeft, int boardSize, char board[10][10]) {
    if( rangeLeft <= 0 ) return false;			// Out of jumps
    if( !onBoard(row, col, boardSize ) ) return false;		// Off board
    char result = getCellStatus( row, col, board );
    if( result == MISS || result == KILL ) {		// Nobody out here -- stop
	    return false;
    }
    else if( getCellStatus( row, col, board ) == WATER ) {	// Not shot at yet
        shot[0] = row; shot[1] = col;
        return true;
    } else if( search(row+rowDelta, col+colDelta, rowDelta, colDelta, shot, rangeLeft-1, boardSize, board) ) {
	    return true;
    }
    return false;
}

// Used to determine whether there is a known injured ship (hit while chasing a different
// ship) that needs to be killed off.
bool haveUnfinishedBusiness( int position[2], int boardSize, char board[10][10]) {
    for( int row=0; row<boardSize; row++ ) {
        for( int col=0; col<boardSize; col++ ) {
            if( getCellStatus( row, col, board ) == HIT ) {
                position[0] = row;
                position[1] = col;
                return true;
            }
        }
    }
    return false;
}

//***********************************************
// BOARD WEIGHING ROUTINES **********************
//***********************************************

// Recomputes board weights
void reWeighBoard(int boardSize, char board[10][10]) {
    for( int row=0; row<boardSize; row++ ) {
        for( int col=0; col<boardSize; col++ ) {
            gameVars.cellWeights[row][col] = calcCellWeight( row, col, boardSize, board);
        }
    }
}

// Prints board weights in grid format
void printWeightedBoard(int boardSize, int board[10][10]) {
    int value;
    for( int row=0; row<boardSize; row++ ) {
        for( int col=0; col<boardSize; col++ ) {
            value = board[row][col];
            if( value < 10 )
                cout << "  ";
            else
                cout << " ";
            cout << value;
        }
        cout << endl;
    }
}

// Finds a cell with the max highest weighting on the board.
// Given the current code, that will be the one with the
// lowest row/col pair of all cells set at the max weight.
void getMaxWeightCell( int board[10][10], int shot[2], int boardSize) {
    int maxWeight=-1;
    for( int row=0; row<boardSize; row++ ) {
        for( int col=0; col<boardSize; col++ ) {
            if( board[row][col] > maxWeight ) {
                shot[0] = row;
                shot[1] = col;
                maxWeight = board[row][col];
            }
        }
    }
}

// Used to calculate what a cell's weight should be.
int calcCellWeight( int row, int col, int boardSize, char board[10][10] ) {
    // Do a bit of seatbelt checking -- off-board => -1
    if( ! onBoard(row, col, boardSize) ) return -1;

    ensureMaxShipLength();
    int weight = 0;
    for( int length=3; length<=gameVars.shipMaxLength; length++ ) {
        weight += calcCellWeightVert( row, col, length, board, boardSize);
        weight += calcCellWeightHoriz( row, col, length, board, boardSize);
    }

    return weight;
}

// Used to calculate a cell's vertical weight for various possible ship lengths.
// Assistant to calcCellWeight( int, int ) above.
int calcCellWeightVert( int row, int col, int length, char board[10][10], int boardSize ) {
    int weight = 0;
    char status;

    // Do vertical calculation
    int low=row; 
    while( low>=0 && row-low<length ) {
        status = getCellStatus( low, col, board );
        if( status == KILL || status == MISS )
            break;	// Blocked
        low--;
    }
    
    int high=row;
    while( high<boardSize && high-row<length ) {
        status = getCellStatus( high, col, board );
        if( status == KILL || status == MISS )
            break;	// Blocked
        high++;
    }
    if( high-low >= length ) {	// A ship could fit here
	    weight += high-low - length;
    }

    return weight;
}

// Used to calculate a cell's horizontal weight for various possible ship lengths.
// Assistant to calcCellWeight( int, int ) above.
int calcCellWeightHoriz( int row, int col, int length, char board[10][10], int boardSize) {
    int weight = 0;
    char status;

    // Do horizontal calculation
    int low=col, high=col;
    while( low>=0 && col-low<length ) {
        status = getCellStatus( row, low, board );
        if( status == KILL || status == MISS ) 
            break;	// Blocked
        low--;
    }
    while( high<boardSize && high-col<length ) {
        status = getCellStatus( row, high, board );
        if( status == KILL || status == MISS )
            break;	// Blocked
        high++;
    }
    if( high-low >= length ) {	// A ship could fit here
	    weight += high-low - length;
    }

    return weight;
}

//***********************************************
// MISCELLANEOUS OTHER ROUTINES *****************
//***********************************************

bool onBoard( int row, int col, int boardSize ) {
    return row>=0 && col>=0 && row<boardSize && col<boardSize; 
}

void ensureMaxShipLength(){
    for(int i=0;i<6;i++){
        if(gameVars.shipLengths[i]>gameVars.shipMaxLength){
            gameVars.shipMaxLength=gameVars.shipLengths[i];
        }
    }
}

// Think C/C++ macro. Just to avoid typing the whole mess in
// every time!
char getCellStatus( int row, int col, char board[10][10] ) {
    return board[row][col];
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
        int tempRow = msg.at("row");
        int tempCol = msg.at("col");
        string tempResult = msg.at("str");
        if(tempResult.c_str()[0] == HIT){
            shotBoard[tempRow][tempCol]=HIT;
        }else if(tempResult.c_str()[0] == MISS){
            shotBoard[tempRow][tempCol]=MISS;
        }
    }else{
        //do nothing
        //unless... ?
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