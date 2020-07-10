/**
 * @author Stefan Brandle and Jonathan Geisler
 * @date August, 2004
 * Main driver for BattleShipsV3 testing.
 *
 *
 */

#include <iostream>
#include <iomanip>
#include <cctype>
#include <unistd.h>

// Next 2 to access and setup the random number generator.
#include <cstdlib>
#include <ctime>

// BattleShips project specific includes.
#include "BoardV3.h"
#include "AIContest.h"
#include "PlayerV2.h"
#include "conio.h"

// Include your player here
//    Professor's contestants
#include "CleanPlayerV2.h"
#include "SemiSmartPlayerV2.h"
#include "GamblerPlayerV2.h"
#include "LearningGambler.h"

// Change to include your player directly below
#include "GetgenPlayer.h"


PlayerV2* getPlayer( int playerId, int boardSize );
void playMatch( int player1Id, int player2Id, bool showMoves, bool showAllRounds );
int comparePlayers (const void * a, const void * b);
int selectPlayer( string playerNames[], int NumPlayers, int selectionNumber );

using namespace std;
using namespace conio;

float secondsPerMove = 1;
int boardSize;    // BoardSize
int totalGames = 0;
int totalCountedMoves = 0;
const int NumPlayers = 5;

int wins[NumPlayers][NumPlayers];
int playerIds[NumPlayers];
int lives[NumPlayers];
int winCount[NumPlayers];
int statsShotsTaken[NumPlayers];
int statsGamesCounted[NumPlayers];
string playerNames[NumPlayers] = { 
    "Clean Player",
    "Semi-Smart Player",
    "Gambler Player",
    "Learning Gambler",

    // Change to your player's name below
    "Getgen Player"
};

int main() {
    //bool silent = false;

    // Initialize various win statistics 
    for(int i=0; i<NumPlayers; i++) {
        statsShotsTaken[i] = 0;
        statsGamesCounted[i] = 0;
        winCount[i] = 0;
        lives[i] = NumPlayers/2;
        playerIds[i] = i;

        for(int j=0; j<NumPlayers; j++) {
            wins[i][j] = 0;
        }
    }

    // Seed (setup) the random number generator.
    // This should only happen once per program run.
    srand(time(NULL));

    // Now to get the board size.
    cout << "Welcome to the AI Bot tester." << endl << endl;
    cout << "What size board would you like? [Anything other than numbers 3-10 exits.] ";
    cin >> boardSize;
    
    // If have invalid board size input (non-number, or 0-2, or > 10).
    if ( !cin || boardSize < 3 || boardSize > 10 ) {
        cout << "Exiting" << endl;
        return 1;
    }

    // Find out how many times to test the AI.
    cout << "How many times should I test the game AI? ";
    cin >> totalGames;

    cout << "The displayed games for each AI match are played at the specified speed." << endl
         << "How many seconds per move? (E.g., 1, 0.5, 1.3) : ";
    cin >> secondsPerMove;

    // Find out whether to display all games
    cout << "Should all rounds be displayed (y|[N])? ";
    cin.ignore(100, '\n');
    char ans = cin.get();
    bool showAllRounds = ans=='Y' || ans=='y';

    // Find out which AI's are to play
    int player1Num = selectPlayer( playerNames, NumPlayers, 1 );
    int player2Num = selectPlayer( playerNames, NumPlayers, 2 );

    // And now it's show time!
    playMatch(player1Num, player2Num, true, showAllRounds);
    usleep(3000000);    // Pause 3 seconds to let viewers see stats

    cout << endl << endl;

    // Now calculate contest results
    qsort( playerIds, NumPlayers, sizeof(int), comparePlayers );

    // Add up the total wins per player
    for( int i=0; i<NumPlayers; i++ ) {
        for( int j=0; j<NumPlayers; j++ ) {
            winCount[i]+= wins[i][j];
        }
    }

    if( winCount[player1Num] > winCount[player2Num] ) {
        cout << setTextStyle( BOLD );
    }
    cout << setw(2) << player1Num << ": " << playerNames[player1Num] 
         << ", Wins=" << winCount[player1Num] << endl;

    if( winCount[player1Num] < winCount[player2Num] ) {
        cout << setTextStyle( BOLD );
    }
    cout << setw(2) << player2Num << ": " << playerNames[player2Num] 
         << ", Wins=" << winCount[player2Num] << endl;

    cout << resetAll () << endl;

    return 0;
}

void playMatch( int player1Id, int player2Id, bool showMoves, bool showAllRounds ) {
    PlayerV2 *player1, *player2;
    AIContest *game;
    int matchWins[2] = {0, 0};
    bool player1Won=false, player2Won=false;
    int player1Ties=0, player2Ties=0;

    player1 = getPlayer(player1Id, boardSize);
    player2 = getPlayer(player2Id, boardSize);

    bool silent = true;
    for( int count=0; count<totalGames; count++ ) {
        player1Won = false; player2Won = false;
        player1->newRound();
        player2->newRound();

        if( count==totalGames-1 ) {
            silent = false;
            game = new AIContest( player1, playerNames[player1Id], player2, playerNames[player2Id], boardSize, silent );
            game->play( secondsPerMove, totalCountedMoves, player1Won, player2Won );
        } 
        else {
            silent = showAllRounds==false;
            game = new AIContest( player1, playerNames[player1Id], player2, playerNames[player2Id], boardSize, silent );
            game->play( 0, totalCountedMoves, player1Won, player2Won );
        }

        if((player1Won && player2Won) || !(player1Won || player2Won)) {
            player1Ties++;
            player2Ties++;
            statsShotsTaken[player1Id] += totalCountedMoves;
            statsGamesCounted[player1Id]++;
            statsShotsTaken[player2Id] += totalCountedMoves;
            statsGamesCounted[player2Id]++;
        } else if( player1Won ) {
            matchWins[0]++;
            wins[player1Id][player2Id]++;
            statsShotsTaken[player1Id] += totalCountedMoves;
            statsGamesCounted[player1Id]++;
        } else if( player2Won ) {
            matchWins[1]++;
            wins[player2Id][player1Id]++;
            statsShotsTaken[player2Id] += totalCountedMoves;
            statsGamesCounted[player2Id]++;
        }
        delete game;
    }

    delete player1;
    delete player2;

    cout << endl << "********************" << endl;
    cout << playerNames[player1Id] << ": " << setTextStyle( NEGATIVE_IMAGE ) << "wins=" << matchWins[0] << resetAll()
         << " losses=" << totalGames-matchWins[0]-player1Ties 
         << " ties=" << player1Ties << endl;
    cout << playerNames[player2Id] << ": " << setTextStyle( NEGATIVE_IMAGE ) << "wins=" << matchWins[1] << resetAll()
         << " losses=" << totalGames-matchWins[1]-player2Ties 
         << " ties=" << player2Ties << endl;
    cout << "********************" << endl;
}

int comparePlayers (const void * a, const void * b) {
    int p1 = *(int*)a;
    int p2 = *(int*)b;

    if( lives[p1] > lives[p2] ) return -1;
    else if( lives[p1] < lives[p2] ) return 1;
    else {
        if( winCount[p1] > winCount[p2] ) return -1;
        else if( winCount[p1] < winCount[p2] ) return 1;
        else return 0;
    }
}

PlayerV2* getPlayer( int playerId, int boardSize ) {
    switch( playerId ) {
        // Professor provided
        default:
        case 0: return new CleanPlayerV2( boardSize );
        case 1: return new SemiSmartPlayerV2( boardSize );
        case 2: return new GamblerPlayerV2( boardSize );
        case 3: return new LearningGambler( boardSize );

        // Change for your player's AI below
        case 4: return new GetgenPlayer( boardSize );
    }
}

int selectPlayer( string playerNames[], int NumPlayers, int selectionNumber ) {
    // Find out which AI's are to play
    int num = -1;
    do {
        cout << endl << "Select player #" << selectionNumber << " from one of the following." << endl << endl;
        for(int i=0; i<NumPlayers; i++) {
            cout << i << "\t" << playerNames[i] << endl;
        }
        cout << endl << "Type in the number for AI #" << selectionNumber << " (choices are 0-" << NumPlayers-1 << "): ";
        cin >> num;
        if( num<0 || num>NumPlayers-1 ) {
            cout << "Invalid player number => " << num << endl;
        }
    } while ( num<0 || num>NumPlayers-1 );
    
    cout << endl << playerNames[num] << " selected for player #" << selectionNumber << endl << endl;

    return num;
}
