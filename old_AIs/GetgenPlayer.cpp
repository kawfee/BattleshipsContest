/*
 * @brief GetgenPlayer AI for battleships
 * @file GetgenPlayer.cpp
 * @author Stefan Brandle, Jonathan Geisler
 * @date September, 2004 Updated 2015 for multi-round play.
 *
 * This Battleships AI is very simple and does nothing beyond playing
 * a legal game. However, that makes it a good starting point for writing
 * a more sophisticated AI.
 */

#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <time.h>

#include "GetgenPlayer.h"

/**
 * @brief Constructor that initializes any inter-round data structures.
 * @param boardSize Indication of the size of the board that is in use.
 *
 * The constructor runs when the AI is instantiated (the object gets created)
 * and is responsible for initializing everything that needs to be initialized
 * before any of the rounds happen. The constructor does not get called 
 * before rounds; newRound() gets called before every round.
 */
GetgenPlayer::GetgenPlayer( int boardSize )
    :PlayerV2(boardSize)
{
    // Could do any initialization of inter-round data structures here.
    this->round=0;
    
    for(int i=0;i<MAX_BOARD_SIZE;i++){
        for(int j=0;j<MAX_BOARD_SIZE;j++){
            opponentShotStatBoard[i][j]=0;
            opponentShipStatBoard[i][j]=0;
        }
    }
}

/**
 * @brief Destructor placeholder.
 * If your code does anything that requires cleanup when the object is
 * destroyed, do it here in the destructor.
 */
GetgenPlayer::~GetgenPlayer( ) {}

/*
 * Private internal function that initializes a MAX_BOARD_SIZE 2D array of char to water.
 */
void GetgenPlayer::initializeBoard() {
    for(int i=0;i<MAX_BOARD_SIZE;i++){
        for(int j=0;j<MAX_BOARD_SIZE;j++){
            selfShotBoard[i][j]=WATER;
            selfShipBoard[i][j]=WATER;
        }
    }
}


/**
 * @brief Specifies the AI's shot choice and returns the information to the caller.
 * @return Message The most important parts of the returned message are 
 * the row and column values. 
 *
 * See the Message class documentation for more information on the 
 * Message constructor.
 */

Message GetgenPlayer::getMove() {
        int value=-1,bestValue=-1;
        int bestRow=0, bestCol=0;
        //average->get above average->take average of those->get above average->choose at random from the cream of the crop
                   // if((col%3==0 && row%3==0) || (col%3==1 && row%3==1) || (col%3==2 && row%3==2)) 

        for(int r=0; r<boardSize; r++){
            for(int c=0; c<boardSize; c++){
                value=calcValue(r, c);
                if(value>=bestValue){
                    bestValue=value;
                    bestRow=r;
                    bestCol=c;
                }
            }
        }
        
        Message result( SHOT, bestRow, bestCol, "Oof", None, 1 );
        return result;
}

/**
 * @brief Tells the AI that a new round is beginning.
 * The AI show reinitialize any intra-round data structures.
 */
void GetgenPlayer::newRound() {
    /* GetgenPlayer is too simple to do any inter-round learning. Smarter players 
     * reinitialize any round-specific data structures here.
     */
    this->initializeBoard();
    this->round++;
}

/**
 * @brief Gets the AI's ship placement choice. This is then returned to the caller.
 * @param length The length of the ship to be placed.
 * @return Message The most important parts of the returned message are 
 * the direction, row, and column values. 
 *
 * The parameters returned via the message are:
 * 1. the operation: must be PLACE_SHIP 
 * 2. ship top row value
 * 3. ship top col value
 * 4. a string for the ship name
 * 5. direction Horizontal/Vertical (see defines.h)
 * 6. ship length (should match the length passed to placeShip)
 */

/*
 0123456789
0----------
1----------
2----------
3----------
4----------
5----------
6----------
7----------
8----------
9XXXXX-----
*/





void GetgenPlayer::findSafest(int length, Direction& theDir, int& theRow, int& theCol) {
    int hitAmount = 0, minHit = 10000;
    bool broke=false;
    // horizontal
    for(int row=0; row<boardSize; row++) {
        for(int col=0; col<boardSize-length+1; col++) {
            hitAmount=0;
            broke=false;
            for(int c=0; c<length; c++) {
                if(col+c<boardSize && selfShipBoard[row][col+c] == WATER) {   //not past the board
                    hitAmount+=opponentShotStatBoard[row][col+c];
                }else{
                    broke=true;
                    break;
                }
            }
            if(!broke && hitAmount<minHit) {
                minHit = hitAmount;
                theRow = row;
                theCol = col;
                theDir = Horizontal;
            }
        }
    }
    // vertical
    for(int col=0; col<boardSize; col++) {
        for(int row=0; row<boardSize-length+1; row++) {
            hitAmount = 0;
            broke=false;
            for(int r=0; r<length; r++) {
                if(row+r<boardSize && selfShipBoard[row+r][col] == WATER) {   //not past the board
                    hitAmount+=opponentShotStatBoard[row+r][col];
                }else{
                    broke=true;
                    break;
                }
            }
            if(!broke && hitAmount<minHit) {
                minHit = hitAmount;
                theRow = row;
                theCol = col;
                theDir = Vertical;
            }
        }
    }
  /*
    cout<<"boardSize: "<<boardSize<<endl
        <<"theRow: "<<theRow<<endl
        <<"theCol: "<<theCol<<endl
        <<"theDir: "<<theDir<<endl
        <<"length: "<<length<<endl<<endl;
  */
}

Message GetgenPlayer::placeShip(int length) {
    int row=0, col=0, counter=0;
    bool roundNice=false;
    Direction dir = Direction(rand()%2+1);
    if(round<69){
        while( true ) {
            counter++;
            dir = Direction(rand()%2+1);
            if(dir == Horizontal) {
                row = rand()%boardSize;
                col = rand()%(boardSize-length+1);
            } else {
                col = rand()%boardSize;
                row = rand()%(boardSize-length+1);
            }
            if( canPlaceShip( row,col,dir,length,roundNice ) ) {
                markShip(row,col,dir,length);
                
                char shipName[10];
                snprintf(shipName, sizeof shipName, "Ship%d", numShipsPlaced);
                numShipsPlaced++;

                // parameters = mesg type (PLACE_SHIP), row, col, a string, direction (Horizontal/Vertical)
                Message ship(PLACE_SHIP,row,col,shipName,dir,length);
                return ship;
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
                markShip(row,col,dir,length);
            
                char shipName[10];
                snprintf(shipName, sizeof shipName, "Ship%d", numShipsPlaced);
                numShipsPlaced++;
            
                // parameters = mesg type (PLACE_SHIP), row, col, a string, direction (Horizontal/Vertical)
                Message ship(PLACE_SHIP,row,col,shipName,dir,length);
                return ship;
            }
            if(counter>69) {
                roundNice = true;
            }
        }
    }
}

/**
 * @brief Updates the AI with the results of its shots and where the opponent is shooting.
 * @param msg Message specifying what happened + row/col as appropriate.
 */
void GetgenPlayer::update(Message msg) {
    
    switch(msg.getMessageType()) {
        case HIT:
            selfShotBoard[msg.getRow()][msg.getCol()]=msg.getMessageType();
            opponentShipStatBoard[msg.getRow()][msg.getCol()]+=1;
            break;
        case KILL:
            selfShotBoard[msg.getRow()][msg.getCol()]=msg.getMessageType();
            break;
        case MISS:
            selfShotBoard[msg.getRow()][msg.getCol()]=msg.getMessageType();
            break;
        case WIN:
            break;
        case LOSE:
            break;
        case TIE:
            break;
        case OPPONENT_SHOT:
            opponentShotStatBoard[msg.getRow()][msg.getCol()]+=1; 
            break;
    }
}


int GetgenPlayer::calcValue(int row,int col){
    int val = 0;
    int valIncrease=50;
    if(selfShotBoard[row][col]!= WATER){
        return -1;
    }
    for(int c=col; c>=0 && c>col-6; c--){
        char res = selfShotBoard[row][c];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }
    for(int c=col; c<boardSize && c<col+6; c++){
        char res = selfShotBoard[row][c];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }

    for(int r=row; r>=0 && r>row-6; r--){
        char res = selfShotBoard[r][col];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }
    for(int r=row; r<boardSize && r<row+6; r++){
        char res = selfShotBoard[r][col];
        if(res==MISS || res==KILL){
            break;
        }
        if(res==HIT){
            val+=valIncrease;
        }else{
            val++;
        }
    }

    return val + opponentShipStatBoard[row][col]/30;
}

bool GetgenPlayer::canPlaceShip(int row,int col,Direction dir,int length, bool canTouch){
    if(dir == Horizontal) {
        if(row<0 || row >= boardSize) return false;
        if(col<0 || col >  boardSize-length) return false;
        if(!canTouch && col>0 && selfShipBoard[row][col-1]==SHIP) return false;
        for(int c=col;c<col+length;c++){
            if(selfShipBoard[row][c]!=WATER) return false;
            if(!canTouch && selfShipBoard[row-1][c]==SHIP) return false;
            if(!canTouch && selfShipBoard[row+1][c]==SHIP) return false;
        }
        if(!canTouch && col+length+1<boardSize && selfShipBoard[row][col+length+1]==SHIP) return false;

    } else if(dir == Vertical) {
        if(col<0 || col >=  boardSize) return false;
        if(row<0 || row > boardSize-length) return false;
        if(!canTouch && row>0 && selfShipBoard[row-1][col]==SHIP) return false;
        for(int r=row;r<row+length;r++){
            if(selfShipBoard[r][col]!=WATER) return false;
            if(!canTouch && selfShipBoard[r][col+1]==SHIP) return false;
            if(!canTouch && selfShipBoard[r][col-1]==SHIP) return false;
        }
        if(!canTouch && row+length+1<boardSize && selfShipBoard[row+length+1][col]==SHIP) return false;
    }
    return true;
}

void GetgenPlayer::markShip(int row,int col,Direction dir,int length){
    if(dir == Horizontal) {
        for(int c=col;c<col+length;c++){
            selfShipBoard[row][c]=SHIP;
        }
    } else if(dir == Vertical) {
        for(int r=row;r<row+length;r++){
            selfShipBoard[r][col]=SHIP;
        }
    }
}
































