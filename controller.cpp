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
    system("ls ./AI_Executables > ais.txt");
    int size = 0;
    string line;
    ifstream ai_file ("ais.txt");
    if (ai_file.is_open())
    {
        while ( getline (ai_file,line) )
        {
            players[size].name = line;
            players[size].author = "";
            players[size].wins = 0;
            players[size].ties = 0;
            players[size].losses = 0;
            size++;
        }
        ai_file.close();
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

        cout << "How size should the board be? (default: 10) "; // Maybe force value of boardSize to be in range 7-10
        getline(cin, temp);
        if (regex_match(temp, int_expr)){
            boardSize = stoi(temp);    
        }
        
        cout << "Watch all recorded games or only the last? [(1:last), 2:all] ";
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

        string matchFile = clientNameOne + "_vs_" + clientNameTwo + ".log";
        string remove = "rm ./logs/";
        string touch = "touch ./logs/"; 
        system((remove + matchFile).c_str());
        system((touch + matchFile).c_str());

        //start game
        cout << runGame(numGames, players[aiChoiceOne], players[aiChoiceTwo], boardSize, matchFile) << endl;
        cout << "^^^ Game returned result" << endl;

        //display final stats
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
    
    
    
    system("rm ais.txt");
    return 0;
}











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