/**
 * @author Stefan Brandle and Jonathan Geisler
 * @date August, 2004
 * Main driver for BattleShipsV1 implementations.
 * Please type in your name[s] below:
 *
 *
 */

#ifndef AICONTEST_H
#define AICONTEST_H

#include <string>

// BattleShips project specific includes.
#include "Message.h"
#include "BoardV3.h"
#include "PlayerV2.h"

using namespace std;

class AIContest {
  public:
    AIContest( PlayerV2* player1, string player1Name, PlayerV2* player2, string player2Name, int boardSize, bool silent );
    ~AIContest();
    void play( float secondsDelay, int& totalMoves, bool& player1Won, bool& player2Won );

  protected:
    enum Side { Left=1, Right=2 };
    bool placeShips( PlayerV2* player, BoardV3* board);
    void showBoard(BoardV3* board, bool ownerView, string playerName, bool fullRedraw, Side side, bool hLMostRecentShot, int hLRow, int hLCol );
    void clearScreen();
    void updateAI(PlayerV2 *player, BoardV3 *board, int hitRow, int hitCol);
    void snooze(float seconds);
    bool processShot(string playerName, PlayerV2 *player, BoardV3 *board, Side side, int row, int col, PlayerV2* otherPlayer);

    // Data
    PlayerV2 *player1;
    PlayerV2 *player2;
    BoardV3 *player1Board;
    BoardV3 *player2Board;
    string player1Name;
    string player2Name;
    int boardSize;
    bool silent;
    bool player1Won;
    bool player2Won;
    int NumShips;
    static const int MAX_SHIPS = 6;
    string shipNames[MAX_SHIPS];
    int shipLengths[MAX_SHIPS];
    int numShips;
};

#endif
