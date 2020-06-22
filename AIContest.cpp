/**
 * @author Stefan Brandle and Jonathan Geisler
 * @date August, 2004
 * Main driver for BattleShipsV2 implementations.
 * Please type in your name[s] below:
 *
 *
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <unistd.h>
#include <cstdlib>

// BattleShips project specific includes.
#include "defines.h"
#include "Message.h"
#include "BoardV3.h"
#include "AIContest.h"
#include "conio.h"

using namespace std;
using namespace conio;

AIContest::AIContest( PlayerV2* player1, string player1Name, 
                      PlayerV2* player2, string player2Name,
                      int boardSize, bool silent )
{
    // Set up player 1
    this->player1 = player1;
    this->player1Board = new BoardV3(boardSize);
    this->player1Name = player1Name;
    this->player1Won = false;

    // Set up player 2
    this->player2 = player2;
    this->player2Board = new BoardV3(boardSize);
    this->player2Name = player2Name;
    this->player2Won = false;

    // General
    this->boardSize = boardSize;
    this->silent = silent;

    // Ship stuff
    shipNames[0] = "Submarine";
    shipNames[1] = "Destroyer";
    shipNames[2] = "Aircraft Carrier";
    shipNames[3] = "Destroyer 2";
    shipNames[4] = "Submarine 2";
    shipNames[5] = "Aircraft Carrier 2";
    
    numShips = boardSize-2;
    if( numShips > MAX_SHIPS ) {
        numShips = MAX_SHIPS;
    }

    for(int i=0; i<numShips; i++) {
        shipLengths[i] = random()%(MIN_SHIP_SIZE) + 3;
    }
}

AIContest::~AIContest() {
    delete this->player1Board;
    delete this->player2Board;
}

/**
 * Places the ships. 
 */
//bool AIContest::placeShips( PlayerV2* player, BoardV3* board, BoardV3* testingBoard ) {
bool AIContest::placeShips( PlayerV2* player, BoardV3* board ) {
    for( int i=0; i<numShips; i++ ) {
        Message loc = player->placeShip( shipLengths[i] );
        bool placedOk = board->placeShip( loc.getRow(), loc.getCol(), shipLengths[i], loc.getDirection() );
        if( ! placedOk ) {
            cerr << "Error: couldn't place "<<shipNames[i]<<" (length "<<shipLengths[i]<<")"<<endl;
            return false;
        }
    }

    // All ships apparently placed ok.
    return true;
}

void AIContest::showBoard(BoardV3* board, bool ownerView, string playerName,
                          bool fullRedraw, Side side, bool hLMostRecentShot, int hLRow, int hLCol ) {
    if( silent ) return;
    // 'fullRedraw' indicates full board draw or only an update.
    // 'side' (Left / Right) indicates where to draw the board

    // Line 1: board name
    // Line 3: board columns
    // Line 5-14: board rows
    // Line 16: Shot location
    // Line 17: shot results

    char ch;
    // Left board starts at column 1, right board at column 30
    int boardColOffset = side == Left ? 1 : 50;

    cout << gotoRowCol(1, boardColOffset)
         << playerName << flush;
    // Top row
    cout << gotoRowCol(3, boardColOffset) << " |";
    for(int count=0; count<boardSize; count++) {
        cout << count;
    }
    cout << flush;

    cout << gotoRowCol(4, boardColOffset) << flush;
    // Put out horizontal header line
    for(int col=0; col<boardSize+2; col++) {
        cout << '-';
    }
    cout << flush;

    for(int row=0; row<boardSize; row++) {
        cout << gotoRowCol(5+row, boardColOffset) << (char)(row+'A') << "|";
        for(int col=0; col<boardSize; col++) {
            if( ownerView == true ) {
                ch = board->getOwnerView(row,col);
            } else {
                ch = board->getOpponentView(row,col);
            }

            switch(ch) {
                case KILL: cout << fgColor(BLACK) << bgColor(LIGHT_RED); break;
                case HIT: cout << fgColor(BLACK) << bgColor(LIGHT_MAGENTA); break;
                case MISS: cout << fgColor(BLACK) << bgColor(GRAY); break;
                case WATER: cout << fgColor(BLACK) << bgColor(LIGHT_CYAN); break;
            }

            if( ch>= 'a' && ch<='k' ) {
                cout << setTextStyle( NEGATIVE_IMAGE ) << ch << resetAll() << flush;
            } else {
                if( hLMostRecentShot && hLRow==row && hLCol==col ) {
                    cout << setTextStyle( NEGATIVE_IMAGE ) << ch << setTextStyle( NEGATIVE_IMAGE ) << resetAll() << flush;
                } else {
                    cout << ch << resetAll() << flush;
                }
            }
        }
        cout << resetAll() << flush;
    }
    cout << resetAll() << endl;
}

// Clears the screen.
void AIContest::clearScreen() {
    if( silent ) return;
    cout << clrscr() << flush;
}

void AIContest::snooze( float seconds ) {
    // usleep() takes argument in microseconds, so need to convert seconds to microseconds.
    long sleepTime = long(1000000 * seconds);
    usleep(sleepTime);
}

void AIContest::updateAI(PlayerV2 *player, BoardV3 *board, int hitRow, int hitCol) {
    Message killMsg( KILL, -1, -1, "");
    char shipMark = board->getShipMark(hitRow, hitCol);

    for(int row=0; row<boardSize; row++) {
        for(int col=0; col<boardSize; col++) {
            if(board->getShipMark(row,col) == shipMark) {
                killMsg.setRow(row);
                killMsg.setCol(col);
                player->update(killMsg);
            }
        }
    }
}

bool AIContest::processShot(string playerName, PlayerV2 *player, BoardV3 *board, 
                           Side side, int row, int col, PlayerV2 *otherPlayer) 
{
    bool won = false;
    int resultsRow = 16;
    int shotColOffset = side == Right ? 1 : 50;
    int boardColOffset = side == Left ? 1 : 50;
    // Wipe any previous contents clean first
    if( !silent ) cout << gotoRowCol( resultsRow, shotColOffset) << playerName 
                       << "'s shot: [" <<row<< "," <<col<< "]" << endl;
    Message msg = board->processShot( row, col );
    // Hack because board doesn't set these properly.
    msg.setRow(row);
    msg.setCol(col);

    switch( msg.getMessageType() ) {
        case MISS:
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << setw(30) << "";
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << "Miss" << flush;
            player->update(msg);
            break;

        case HIT:
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << setw(30) << "";
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << "Hit" << flush;
            player->update(msg);
            break;

        case KILL:
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << setw(30) << "";
            if( !silent ) cout << gotoRowCol( resultsRow+1, shotColOffset ) << "It's a KILL! " << msg.getString() << flush;
            // Notify that is a hit
            msg.setMessageType(HIT);
            player->update(msg);
            // Probably not needed, but just to be safe ...
            msg.setMessageType(KILL);
            // and notify that all segments of ship are now a KILL
            updateAI(player, board, row, col);

            // Chance to win after every kill. Check
            won = board->hasWon();
            break;

        case DUPLICATE_SHOT:
            if( !silent ) {
                cout << fgColor(RED) << bgColor(WHITE);
                    cout << gotoRowCol( resultsRow+1, boardColOffset ) << setw(30) << "";
                    cout << gotoRowCol( resultsRow+1, boardColOffset ) << "You already shot there.";
                cout << resetAll() << flush;
            }
            player->update(msg);
            break;

        case INVALID_SHOT:
            if( !silent ) {
                cout << fgColor(RED) << bgColor(WHITE);
                cout << gotoRowCol( resultsRow+1, boardColOffset ) << setw(30) << "";
                    cout << gotoRowCol( resultsRow+1, boardColOffset ) << playerName << "Invalid coordinates: [row="<<row<<
                         ", col="<<col<<"]" << flush;
                cout << resetAll() << flush;
            }
            player->update(msg);
            break;

        default:
            if( !silent ) {
                cout << fgColor(RED) << bgColor(WHITE);
                cout << gotoRowCol( resultsRow+1, boardColOffset ) << setw(30) << "";
                    cout << gotoRowCol( resultsRow+1, boardColOffset ) << "Invalid return from processShot: "
                                   << msg.getMessageType() << "(" << msg.getString() << ")";
                cout << resetAll() << flush;
            }
            player->update(msg);
            break;
    }

    // Notify the other player of the shot
    msg.setMessageType(OPPONENT_SHOT);
    otherPlayer->update(msg);

    return won;
}

void AIContest::play( float secondsDelay, int& totalMoves, bool& player1Won, bool& player2Won ) {
    int maxShots = boardSize*boardSize*2;
    totalMoves = 0;
    clearScreen();

    //showBoard(playerBoard, true, "Sneak peek at player's board");
    //BoardV3 testingBoard1(boardSize);
    //if( ! placeShips(player1, player1Board, testingBoard1) ) {
    if( ! placeShips(player1, player1Board) ) {
        //if( ! silent ) {
        cout << endl;
        cout << player1Name << " placed ship in invalid location and forfeits game." << endl;
        cout << endl;
        snooze( secondsDelay*4 );
        //}
        player2Won = true;
    }

    if( ! placeShips(player2, player2Board) ) {
        //if( ! silent ) {
        cout << endl;
        cout << player2Name << " placed ship in invalid location and forfeits game." << endl;
        cout << endl;
        snooze( secondsDelay*4 );
        //}
        player1Won = true;
    }


    while ( !(player1Won || player2Won) && totalMoves < maxShots ) {

        Message shot1 = player1->getMove();
        player1Won = processShot(player1Name, player1, player2Board, Left, shot1.getRow(), shot1.getCol(), player2);

        Message shot2 = player2->getMove();
        player2Won = processShot(player2Name, player2, player1Board, Right, shot2.getRow(), shot2.getCol(), player1);

        if( ! silent ) {
            showBoard(player1Board, false, player1Name + "'s Board", false, Left, true, shot2.getRow(), shot2.getCol());
        }
        if( ! silent ) {
            showBoard(player2Board, false, player2Name + "'s Board", false, Right, true, shot1.getRow(), shot1.getCol());
        }

        totalMoves++;

        if( ! silent ) {
            if( secondsDelay > 0 ) {    // Slows program if call to sleep 0.
                snooze( secondsDelay );
            }
        }

    }

    if( ! silent ) {
        clearScreen();
        showBoard(player1Board, true, "Final status of " + player1Name + "'s board", 
                  true, Left, false, -1, -1);
        cout << endl;
        showBoard(player2Board, true, "Final status of " + player2Name + "'s board", 
                  true, Right, false, -1, -1);
        cout << endl;
    }

    if( player1Won && player2Won ) {
        cout << "The game was a tie. Both players sunk all ships." << endl;
        Message msg(TIE);
        player1->update(msg);
        player2->update(msg);
    } else if( player1Won ) {
        cout << player1Name << " won." << endl;
        Message msg(WIN);
        player1->update(msg);
        msg.setMessageType(LOSE);
        player2->update(msg);
    } else if( player2Won ) {
        cout << player2Name << " won." << endl;
        Message msg(WIN);
        player2->update(msg);
        msg.setMessageType(LOSE);
        player1->update(msg);
    } else {   // both timed out -- neither won
        cout << "The game was a tie. Neither player sunk all ships." << endl;
        Message msg(LOSE);
        player1->update(msg);
        player2->update(msg);
    }

    cout << "--- (Moves = " << totalMoves << ", percentage of board shot at = " <<
                  (100.0*(float)totalMoves)/(boardSize*boardSize) << "%.)" << endl;
    cout << endl;
    if( ! silent ) {
        snooze( 5 );
    }
}
