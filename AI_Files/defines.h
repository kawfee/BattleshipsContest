/**
 * defines.h: lists globally defined values.
 * @author Stefan Brandle, May 2004
 * @author Joey Gorski, August 2020
 */

#ifndef DEFINES_H        // Double inclusion protection
#define DEFINES_H

using namespace std;

    // Shot related
    const char SHOT = '@';
    const char MISS = '*';
    const char DUPLICATE_SHOT  = '!';
    const char HIT  = 'X';
    const char KILL = 'K';
    const char SHIP = 'S';
    const char WATER = '~';
    const char INVALID_SHOT = char(0);

    // Meta information -- win/lose/quit
    const char WIN  = 'W';
    const char LOSE = 'L';
    const char TIE  = 'T';
    const char QUIT = 'Q';
    const char PEEK = 'P';
    const char PLACE_SHIP = 'Z';

    const int MAX_BOARD_SIZE = 10;
    const int MAX_SHIP_SIZE = 5;
    const int MIN_SHIP_SIZE = 3;

    enum Direction { NONE=0, HORIZONTAL=1, VERTICAL=2 };

    // Struct information
    struct Player {
        string name;
        string author;
        int wins;
        int ties;
        int losses;
    };
    struct GameInfo {
        Player player1;
        Player player2;
        bool error;
    };
#endif
