/**
 * @author Stefan Brandle
 * @date April, 2004
 * Lab 15: CleanPlayer AI
 *
 * Please type in your name[s] here:
 */

#include "PlayerV1.h"
#include "CleanPlayer.h"

using namespace std;

CleanPlayer::CleanPlayer( int boardSize ):
    PlayerV1(boardSize)
{
    scanRow = 0;
    scanCol = 0;
    maxShipSize = 3;
}

/**
 * Gets the computer's shot choice. This is then returned to the caller.
 * @return int[] shot
 *  Position 0 of the int array should hold the row, position 1 the column.
 */
Message CleanPlayer::getMove( ) {
    int shotRow = scanRow;
    int shotCol = scanCol;

    if( board[scanRow][scanCol] == HIT ) {
	getFollowUpShot( shotRow, shotCol );
    } else {
	scan( scanRow, scanCol );
	shotRow = scanRow;
	shotCol = scanCol;
    }
    Message shotMessage( SHOT, shotRow, shotCol, "" );
    return shotMessage;
}

void CleanPlayer::getFollowUpShot( int& row, int& col ) {
    if( search(row, col, -1, 0) )		// Up
	return;
    else if ( search(row, col, 1, 0) )		// Down
	return;
    else if ( search(row, col, 0, 1) )		// Right
	return;
    else if ( search(row, col, 0, -1) ) 	// Left
	return;
    else 					// Help! Couldn't find anything to shoot at.
	scan( row, col );			// Should never need this, but put in for luck. :-)
}

bool CleanPlayer::search( int& row, int& col, int rowDelta, int colDelta ) 
{
    for( int range=1; range <= maxShipSize; range++) {
	int r=row+rowDelta*range;
	int c=col+colDelta*range;

	if( ! isOnBoard(r,c) )
	    return false;
        else if( board[r][c] == WATER ) {
	    row=r; col=c;
	    return true;
	}
	else if( board[r][c] == MISS || board[r][c] == KILL )
	    return false;
	else //	If it is a hit, just keep running through loop.
	    ;
    }
    return false;	// Guess we couldn't find anything.
}

bool CleanPlayer::isOnBoard( int row, int col ) {
    if( row>=0 && row<boardSize && col>=0 && col<boardSize )
        return true;
    else
        return false;
    // Or you could skip the if/else and just write
    //   return row>=0 && row<boardSize && col>=0 && col<boardSize;
}

void CleanPlayer::scan( int& row, int& col ) {
    scanCol = scanCol + maxShipSize;
    if( scanCol >= boardSize ) {
	scanCol = scanCol % boardSize;
	// if boardSize is multiple of column, could get caught going down columns. 
	// Adjust if needed.
	if( boardSize % maxShipSize == 0 ) {	
	    if( scanCol + 1 == maxShipSize ) {
		scanCol = 0;
	    } else {
		scanCol++;
	    }
	}
	scanRow++;
	if( scanRow >= boardSize ) {
	    scanRow = 0;
	}
    }
}

