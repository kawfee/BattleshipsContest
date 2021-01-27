/**
 * @authors Stefan Brandle & Jonathan Geisler
 * @date May, 2004
 *
 */

#include <iostream>

#include "OrigGamblerPlayer.h"
#include "Message.h"

using namespace std;

OrigGamblerPlayer::OrigGamblerPlayer( int boardSize ):
    PlayerV1(boardSize)
{
    lastRow = 0;
    lastCol = 0;

    reWeighBoard( cellWeights );
}

Message OrigGamblerPlayer::getMove() {
    getShot(shot);
    Message msg(SHOT, shot[0], shot[1], "Gamble shot");
    return msg;
}

void OrigGamblerPlayer::getShot(int shot[]) {
    if( getCellStatus(lastRow, lastCol) == HIT ) {
	getFollowUpShot( lastRow, lastCol, shot );
    } else if( haveUnfinishedBusiness(shot) ){
	getFollowUpShot( shot[0], shot[1], shot );
    } else {
	getNextProbe( shot );
	lastRow = shot[0];
	lastCol = shot[1];
    }
}

//***********************************************
// MAIN SHOT ROUTINES ***************************
//***********************************************

// Gets a probing shot. Grabs a cell matching the current max board cell weight.
void OrigGamblerPlayer::getNextProbe( int shot[] ) {
    reWeighBoard( cellWeights );
    getMaxWeightCell( cellWeights, shot );
}

// Used to follow up on a wounded ship.
// The code needs to be improved to use weights to determine best
// followup locations.
void OrigGamblerPlayer::getFollowUpShot( int row, int col, int shot[] ) {

    // Starting to play around with weights. Code place-holder.
    int nextRow = 0, nextCol = 0;
    int weightUp = calcCellWeight( row-1, col );
    // Done playing around with weights.  :-)

    // Check for viable shot UP
    if( search(row, col, -1, 0, shot, SHIP_MAX_LENGTH) == true ) {
	return;
    }
    // Ok, nothing [left] to shot at UP. But was there at least something UP?
    // If so, it may be worth checking in the opposite direction might be faster.
    else if( onBoard(row-1, col) && getCellStatus(row-1, col) == HIT ) {
	if( search( row, col, 1, 0, shot, SHIP_MAX_LENGTH-1) == true ) {
	    return;
	}
    }
    // Check for viable shot to RIGHT
    if ( search( row, col, 0, 1, shot, SHIP_MAX_LENGTH ) == true ) {
	return;
    } 
    // Ok, nothing [left] to shot at to RIGHT. But was there at least something to RIGHT?
    // If so, it may be worth checking in the opposite direction might be faster.
    else if( onBoard(row, col+1) && getCellStatus(row, col+1) == HIT ) {
	if( search( row, col, 0, -1, shot, SHIP_MAX_LENGTH-1) == true ) {
	    return;
	}
    }
    // At this point, check DOWN and to LEFT, but don't do reverse checks because 
    // anything there will have been covered above.
    if ( search( row, col, 1, 0, shot, SHIP_MAX_LENGTH ) == true ) {
	return;
    } 
    if ( search( row, col, 0, -1, shot, SHIP_MAX_LENGTH ) == true ) {
	return;
    } else {
	getNextProbe( shot );
	return;
    }
}

// Partner to getFollowUpShot. Assists with recursive hunting for parts of the ship.
bool OrigGamblerPlayer::search( int row, int col, int rowDelta, int colDelta, int shot[], int rangeLeft ) {
    if( rangeLeft <= 0 ) return false;			// Out of jumps
    if( !onBoard(row, col) ) return false;		// Off board
    char result = getCellStatus( row, col );
    if( result == MISS || result == KILL ) {		// Nobody out here -- stop
	return false;
    }
    else if( getCellStatus( row, col ) == WATER ) {	// Not shot at yet
	shot[0] = row; shot[1] = col;
	return true;
    } else if( search(row+rowDelta, col+colDelta, rowDelta, colDelta, shot, rangeLeft-1) ) {
	return true;
    }
    return false;
}

// Used to determine whether there is a known injured ship (hit while chasing a different
// ship) that needs to be killed off.
bool OrigGamblerPlayer::haveUnfinishedBusiness( int position[2] ) {
    for( int row=0; row<boardSize; row++ ) {
	for( int col=0; col<boardSize; col++ ) {
	    if( getCellStatus( row, col ) == HIT ) {
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
void OrigGamblerPlayer::reWeighBoard( int board[][MaxBoardSize] ) {
    for( int row=0; row<boardSize; row++ ) {
	for( int col=0; col<boardSize; col++ ) {
	    board[row][col] = calcCellWeight( row, col );
	}
    }
}

// Prints board weights in grid format
void OrigGamblerPlayer::printWeightedBoard( int board[][MaxBoardSize] ) {
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
void OrigGamblerPlayer::getMaxWeightCell( int board[][MaxBoardSize], int shot[2] ) {
    int maxRow=0, maxCol=0, maxWeight=-1;
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
int OrigGamblerPlayer::calcCellWeight( int row, int col ) {
    // Do a bit of seatbelt checking -- off-board => -1
    if( ! onBoard(row, col) ) return -1;

    int weight = 0;
    for( int length=SHIP_MIN_LENGTH; length<=SHIP_MAX_LENGTH; length++ ) {
	weight += calcCellWeightVert( row, col, length );
	weight += calcCellWeightHoriz( row, col, length );
    }

    return weight;
}

// Used to calculate a cell's vertical weight for various possible ship lengths.
// Assistant to calcCellWeight( int, int ) above.
int OrigGamblerPlayer::calcCellWeightVert( int row, int col, int length ) {
    int weight = 0;
    char status;

    // Do vertical calculation
    int low=row; 
    while( low>=0 && row-low<length ) {
	status = getCellStatus( low, col );
	if( status == KILL || status == MISS )
	    break;	// Blocked
	low--;
    }
    
    int high=row;
    while( high<boardSize && high-row<length ) {
	status = getCellStatus( high, col );
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
int OrigGamblerPlayer::calcCellWeightHoriz( int row, int col, int length ) {
    int weight = 0;
    char status;

    // Do horizontal calculation
    int low=col, high=col;
    while( low>=0 && col-low<length ) {
	status = getCellStatus( row, low );
	if( status == KILL || status == MISS ) 
	    break;	// Blocked
	low--;
    }
    while( high<boardSize && high-col<length ) {
	status = getCellStatus( row, high );
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

bool OrigGamblerPlayer::onBoard( int row, int col ) {
    return row>=0 && col>=0 && row<boardSize && col<boardSize; 
}

// Think C/C++ macro. Just to avoid typing the whole mess in
// every time!
char OrigGamblerPlayer::getCellStatus( int row, int col ) {
    return board[row][col];
}

