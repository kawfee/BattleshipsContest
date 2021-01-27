/**
 * @author Stefan Brandle
 * @date April, 2004
 * Lab 15: CleanPlayer AI. Decent player with very simple logic.
 *
 * Please type in your name[s] here:
 */

#ifndef CLEANPLAYER_H		// Double inclusion protection
#define CLEANPLAYER_H

#include "PlayerV1.h"

using namespace std;

class CleanPlayer: public PlayerV1 {
    public:
	CleanPlayer( int boardSize );
	Message getMove( );

    private:
	// Local functions
	void getFollowUpShot( int& row, int& col );
	bool search( int& startRow, int& startCol, int rowDelta, int colDelta );
	void scan( int& row, int& col );
	bool isOnBoard( int row, int col );

	// Local data
        int scanRow;
        int scanCol;
        int maxShipSize;

};	// end of class CleanPlayer

#endif

