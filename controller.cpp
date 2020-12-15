#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <regex>

#include "subprocess.hpp"
#include "server.cpp"
#include "defines.h"


using namespace std;
using namespace subprocess;


int main(){
    srand(time(NULL));

    // Edit how many players here
    const int playerCount = 50; // We assume size 50 as a max here but make this bigger if tournament requires it.

    /*
        make a list of all AI executable files
        call runGame() for those files and do tourney cr-arbage
    */
    Player players[playerCount];
    system("ls ./client_Ais > ais.txt");
    int size = 0;
    string line;
    ifstream myfile ("ais.txt");
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            players[size].name = line;
            players[size].author = "";
            players[size].wins = 0;
            players[size].ties = 0;
            players[size].losses = 0;
            size++;
        }
        myfile.close();
    }
    
    regex int_expr("^[0-9]+$");
    regex doub_expr("^[0-9]*+\.[0-9]+$");

    string gameType = "";
    int numGames    = 500;
    int boardSize   = 10;
    int watchAll    = 1;
    int runChoice   = 1;
    double delay    = 0.3;
    string temp     = "";

    int aiChoiceOne;
    int aiChoiceTwo;

    cout << "What type of game do you want? [(1:test), 2:competition] ";
    getline(cin, gameType);
    if(gameType == "1" || gameType == ""){
        //if 2 player ask for the starting values
                // - flow or stutter step games
                // - which AIs will battle
        cout << "How many games should the AI play? (default: 500) ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            numGames = stoi(temp);    
        }

        cout << "How size should the board be? (default: 10) ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            boardSize = stoi(temp);    
        }
        
        cout << "Watch all games or only the last? [(1:last), 2:all] ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            watchAll = stoi(temp);    
        }


        cout << "Should the game run via time delay or directed input? [(1:delay), 2:input] ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            runChoice = stoi(temp);    
        }

        if (runChoice == 1){
            cout << "How long should the delay between actions be? (default: 0.3) ";
            getline(cin, temp);
            if (regex_match(temp, doub_expr) || regex_match(temp, int_expr)){
                delay = stod(temp);    
            }
            cout << "delay: " << delay << endl;
        }
        
        cout << endl;
        for(int i=0;i<size;i++){
            cout << "[" << i << "] " << players[i].name << endl;
        }
        cout << endl << "Choose an AI to battle: ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            aiChoiceOne = stoi(temp);    
        }

        cout << "Choose another AI to battle: ";
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            aiChoiceTwo = stoi(temp);    
        }

        string clientNameOne = players[aiChoiceOne].name;
        string clientNameTwo = players[aiChoiceTwo].name;

        cout << "clientNameOne: " << clientNameOne << endl
            << "clientNameTwo: " << clientNameTwo << endl;


        cout << runGame(numGames, players[aiChoiceOne], players[aiChoiceTwo], boardSize) << endl;
        cout << "^^^ Game returned result" << endl;

        cout << players[aiChoiceOne].name 
             <<" RECORD W-L-T: " 
             << players[aiChoiceOne].wins << " " 
             << players[aiChoiceOne].losses << " " 
             << players[aiChoiceOne].ties 
             << endl;
        cout << players[aiChoiceTwo].name 
             << " RECORD W-L-T: " 
             << players[aiChoiceTwo].wins << " " 
             << players[aiChoiceTwo].losses << " " 
             << players[aiChoiceTwo].ties 
             << endl;
    }
    return 0;
    
    //start tournament
    /* 
        int remainingPlayers=size;
        Player winners[size/2+1];
        Player losers[size/2+1];
        while(remainingPlayers>1){
            Player match[2];
            int choice=abs(rand()*100)%size;
            string str1=movePlayer(choice, players, match, size);
            
            choice=abs(rand()*100)%size;
            string str2=movePlayer(choice, players, match, size);

            cout << str1 << " VS " << str2 << endl;

            //find the winner/loser for now we are just skipping this and having player 1 always win
            match[0].wins++;
            match[1].losses++;
            remainingPlayers-=2;
            cout << match[0].name << "Wins!" << " Total Wins: " << match[0].wins << endl;
            cout << match[1].name << "Loses!" << " Total Losses: " << match[1].losses << endl;
            str1=movePlayer(0, match, winners, size/2+1);
            str2=movePlayer(1, match, losers, size/2+1);
            
            

        } 
    */

    /*
        tourney structure:
        X   -round robin (.5*numPlayers lives)
            -brackets
            -double elim/triple elim
    */

    /* 
        for(int i = 0; i < size; i++){
            for(int j = i + 1; j < size; j++){
                cout << players[i].name << " VS " << players[j].name << endl;
                //have runGame return the struct that we built at the top
                //runGame(500, players[i].name, players[j].name);
                players[i].wins++;
                players[j].losses++;
                cout << players[i].name << " Wins!" << " Total Wins: " << players[i].wins << endl;
                cout << players[j].name << " Loses!" << " Total Losses: " << players[j].losses << endl;
            }
        } 
    */

    // string clientNameOne = players[2].name;
    // string clientNameTwo = players[0].name; // will change to one at some point

    // cout << "clientNameOne: " << clientNameOne << endl
    //      << "clientNameTwo: " << clientNameTwo << endl;


    // cout << runGame(numGames, players[2], players[0], boardSize) << endl;
    // cout << "^^^ Game returned result" << endl;

    // cout << players[2].name << " RECORD W-L-T: " << players[2].wins << " " << players[2].losses << " " << players[2].ties << endl;
    // cout << players[0].name << " RECORD W-L-T: " << players[0].wins << " " << players[0].losses << " " << players[0].ties << endl;

    return 0;
}











/* 
    When we run the program, X happens.
        - select competition, tournament, or 2 player test
        - 2 player test
            - asks how many games
            - watch all games or just the last
            - flow or stutter step games
                - how fast does the game play
            - which AIs will battle
*/
