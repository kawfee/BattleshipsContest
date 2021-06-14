/**
 * @authors Stefan Brandle & Jonathan Geisler
 * @date May, 2004
 *
 */

#ifndef ORIGGAMBLERPLAYER_H
#define ORIGGAMBLERPLAYER_H

#include "PlayerV1.h"

using namespace std;

class OrigGamblerPlayer: public PlayerV1 {
  public:
    OrigGamblerPlayer( int boardSize );
    Message getMove();

    // Note: making assumptions about ship size.
  private:
    static const int SHIP_MAX_LENGTH = 4;
    static const int SHIP_MIN_LENGTH = 3;
    int lastRow, lastCol;
    int shot[2];
    int cellWeights[MaxBoardSize][MaxBoardSize];

    void getShot(int shot[]);
    void getNextProbe( int shot[] );
    void getFollowUpShot( int row, int col, int shot[] );
    bool search( int row, int col, int rowDelta, int colDelta, int shot[], int rangeLeft );
    bool haveUnfinishedBusiness( int position[2] );
    void reWeighBoard( int board[][MaxBoardSize] );
    void printWeightedBoard( int board[][MaxBoardSize] );
    void getMaxWeightCell( int board[][MaxBoardSize], int shot[2] );
    int calcCellWeight( int row, int col );
    int calcCellWeightVert( int row, int col, int length );
    int calcCellWeightHoriz( int row, int col, int length );
    bool onBoard( int row, int col );
    char getCellStatus( int row, int col );

};

#endif
