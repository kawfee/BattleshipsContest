#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <unistd.h>
#include <map>

#include "defines.h"
#include "conio.cpp"
#include "conio.h"

//structs
struct playerData{
    string name    = "eat";
    string author  = "pant";
    string board[10];
    string shot    = "first";
    bool   won     = false;
};

struct turnStruct{
    playerData player1;
    playerData player2;
};

void displayTurn(turnStruct turnData, bool endDisplay);
int  extractInteger(string str);
void printBoard(turnStruct turn, int boardChoice, bool endDisplay);

/*
    string gotoRowCol( const int x, const int y );
    string fgColor( Color c );
    string bgColor( Color c );
    string setTextStyle( TextStyle ts );
    string resetAll( );
    string clrscr();
*/



using namespace std;
using namespace conio;

int main(){
    
    //Both of these may need to change
    map<int, map<int, turnStruct>> matches;  // This maps the 50 matches that were played in the log file.
    map<int, turnStruct> match;              // This maps the individual turns in a single match. accessing a turn looks like match[x][y]
    int matchIndex = 0;
    int turnIndex  = 0;
    //cout << "Test['b']: " << test['b'] << endl; // a test for uninitalized value turned up as zero

    /*
    for match in matches:
        for turn in match:
            turn = turnStruct
    
    
    
    while(game[whatMatchThisIs]!=null){
        while(game[whatMatchThisIs][whatTurnThisIs]!=null){
            do:
                game;
        }
    }
    */

    
    regex match_start("^MATCH_START_ROUND:.*");
    regex match_end("^MATCH_OVER");

    cout << clrscr() << flush;

    string line;
    ifstream log_file ("logs/client_vs_client_auto.log");
    
    //print off dummy board
    cout << gotoRowCol(3, 1)   << "author1"      << endl
         << gotoRowCol(4, 1)   << "client1"      << endl
         << gotoRowCol(6, 1)   << " |0123456789" << endl
         << gotoRowCol(7, 1)   << "------------" << endl
         << gotoRowCol(8, 1)   << "A|~~~~~~~~~~" << endl
         << gotoRowCol(9, 1)   << "B|~~~~~~~~~~" << endl
         << gotoRowCol(10, 1)  << "C|~~~~~~~~~~" << endl
         << gotoRowCol(11, 1)  << "D|~~~~~~~~~~" << endl
         << gotoRowCol(12, 1)  << "E|~~~~~~~~~~" << endl
         << gotoRowCol(13, 1)  << "F|~~~~~~~~~~" << endl
         << gotoRowCol(14, 1)  << "G|~~~~~~~~~~" << endl
         << gotoRowCol(15, 1)  << "H|~~~~~~~~~~" << endl
         << gotoRowCol(16, 1)  << "I|~~~~~~~~~~" << endl
         << gotoRowCol(17, 1)  << "J|~~~~~~~~~~" << endl;
        
    cout << gotoRowCol(3, 50)  << "author2"      << endl
         << gotoRowCol(4, 50)  << "client2"      << endl
         << gotoRowCol(6, 50)  << " |0123456789" << endl
         << gotoRowCol(7, 50)  << "------------" << endl
         << gotoRowCol(8, 50)  << "A|~~~~~~~~~~" << endl
         << gotoRowCol(9, 50)  << "B|~~~~~~~~~~" << endl
         << gotoRowCol(10, 50) << "C|~~~~~~~~~~" << endl
         << gotoRowCol(11, 50) << "D|~~~~~~~~~~" << endl
         << gotoRowCol(12, 50) << "E|~~~~~~~~~~" << endl
         << gotoRowCol(13, 50) << "F|~~~~~~~~~~" << endl
         << gotoRowCol(14, 50) << "G|~~~~~~~~~~" << endl
         << gotoRowCol(15, 50) << "H|~~~~~~~~~~" << endl
         << gotoRowCol(16, 50) << "I|~~~~~~~~~~" << endl
         << gotoRowCol(17, 50) << "J|~~~~~~~~~~" << endl;
    

    if(log_file.is_open()){
        while(getline(log_file, line)){
            if(regex_match(line, match_start)){
                //cout << "There was a beginning of a match!" << endl;
                cout << gotoRowCol(1,1) << "Match Number: " << extractInteger(line) << endl;
                map<int, turnStruct> match;
                matches[matchIndex] = match;
                turnIndex = 0;

            }else if(regex_match(line, match_end)){
                cout << gotoRowCol(1,1) << "Match Ended!                   " << endl;
                matchIndex++;
            }else{
                //save match to current game match

                
                //read in player1
                matches[matchIndex][turnIndex].player1.author = line;
                getline(log_file, line);
                matches[matchIndex][turnIndex].player1.name = line;
                getline(log_file, line);
                for(int i=0; i<10; i++){
                    matches[matchIndex][turnIndex].player1.board[i] = line;
                    getline(log_file, line);
                }

                matches[matchIndex][turnIndex].player1.shot = line;
                getline(log_file, line);

                //start reading in player2

                matches[matchIndex][turnIndex].player2.author = line;
                getline(log_file, line);
                matches[matchIndex][turnIndex].player2.name = line;
                getline(log_file, line);
                for(int i=0; i<10; i++){
                    matches[matchIndex][turnIndex].player2.board[i] = line;
                    getline(log_file, line);
                }

                matches[matchIndex][turnIndex].player2.shot = line;

                turnIndex++;
            }
        }
    }

    
    for(int curMatch=0; curMatch<matchIndex+1; curMatch++){
        int turnsInMatch = matches[curMatch].size();
        cout << gotoRowCol(1,1) << "Match Number: " << curMatch << endl;
        cout << gotoRowCol(2,1) << "                                                     " << endl;
        int turnCount = 0;
        for( ; turnCount < turnsInMatch; ){
            displayTurn(matches[curMatch][turnCount], false);
            // handle input for going back
            cout << gotoRowCol(21, 1);
            char temp;
            temp = cin.get();
            if(temp != '\n'){
                cin.ignore(1024, '\n');
            }
            cout << gotoRowCol(21, 1) << "                                                                                                                   ";
            if(temp == 'b' || temp == 'r'){
                if(turnCount>0){
                    turnCount--;
                }
            }else{
                turnCount++;
            }
            //sleep(1);
        }
        //handle end-game display
        cout << gotoRowCol(1,1) << "Match Ended!                                       " << endl;
        displayTurn(matches[curMatch][turnCount-1], true);
        char temp;
        temp = cin.get();
        if(temp != '\n'){
            cin.ignore(1024, '\n');
        }
        //sleep(5);
    }

    return 0;
}


void displayTurn(turnStruct turnData, bool endDisplay){
    if(!endDisplay){
        cout << gotoRowCol(3, 1) << turnData.player1.author << endl;
        cout << gotoRowCol(4, 1) << turnData.player1.name << "'s board                                                                                   " << endl;
        printBoard(turnData, 1, false);
        cout << gotoRowCol(19, 1) << "                                                                                                                   ";
        cout << gotoRowCol(19, 1) << turnData.player2.shot << endl;

        cout << gotoRowCol(3, 50) << turnData.player2.author << endl;
        cout << gotoRowCol(4, 50) << turnData.player2.name << "'s board                                                                                   " << endl;
        printBoard(turnData, 2, false);
        cout << gotoRowCol(19, 50) << "                                                                                                                   ";
        cout << gotoRowCol(19, 50) << turnData.player1.shot << endl;
    }else{
        cout << gotoRowCol(3, 1) << turnData.player1.author << endl;
        cout << gotoRowCol(4, 1) << "Final Status of " << turnData.player1.name << "'s board" << endl;
        printBoard(turnData, 1, true);
        cout << gotoRowCol(19, 1) << "                                                                                                                   ";
        cout << gotoRowCol(19, 1) << turnData.player2.shot << endl;
        

        cout << gotoRowCol(3, 50) << turnData.player2.author << endl;
        cout << gotoRowCol(4, 50) << "Final Status of " << turnData.player2.name << "'s board" << endl;
        printBoard(turnData, 2, true);
        cout << gotoRowCol(19, 50) << "                                                                                                                   ";
        cout << gotoRowCol(19, 50) << turnData.player1.shot << endl;


        if(turnData.player1.won && turnData.player2.won){
            cout << gotoRowCol(25, 1) << "TIE!!!" << endl;
        }
        else if(turnData.player1.won){
            cout << gotoRowCol(25, 1) << turnData.player1.name << " WON!!!" << endl;
        }else{
            cout << gotoRowCol(25, 1) << turnData.player2.name << " WON!!!" << endl;
        }
    }
    
}


int extractInteger(string str) { 
    stringstream ss;     
  
    /* Storing the whole string into string stream */
    ss << str; 
  
    /* Running loop till the end of the stream */
    string temp; 
    int found; 
    while (!ss.eof()) { 
  
        /* extracting word by word from stream */
        ss >> temp; 
  
        /* Checking the given word is integer or not */
        if (stringstream(temp) >> found) 
            return found;
  
        /* To save from space at the end of string */
        temp = ""; 
    }
    return -1; 
} 


void printBoard(turnStruct turn, int boardChoice, bool endDisplay){
    if(boardChoice==1){
        int pos = turn.player2.shot.find(":");
        int currRow = (turn.player2.shot)[pos+2]-48;
        int currCol = (turn.player2.shot)[pos+4]-48;

        for (int row = 0; row < 10; row++){
            cout << gotoRowCol(row+8, 3); 
            for(int col=0; col < 10; col++){
                char ch = turn.player1.board[row][col];
                if(ch == WATER || (!endDisplay && ch == SHIP)){ 
                    if(row==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(CYAN);
                    }else{
                        cout << bgColor(CYAN) << fgColor(BLACK);
                    }
                    cout << WATER << flush;
                }else if(endDisplay && (ch == SHIP || ch == HIT)){
                    if(ch == HIT){
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    else if(ch == SHIP){
                        cout << bgColor(WHITE) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                    turn.player1.won = true;
                }else if(ch == HIT){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(LIGHT_MAGENTA);
                    }else{
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == MISS){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(GRAY);
                    }else{
                        cout << bgColor(GRAY) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == KILL){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(RED);
                    }else{
                        cout << bgColor(RED) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else{
                    cout << bgColor(RESET) << fgColor(WHITE) << ch << flush;
                }
            }
            cout << endl;
        }
        cout << bgColor(RESET) << fgColor(WHITE);
    }else{
        int pos = turn.player1.shot.find(":");
        int currRow = (turn.player1.shot)[pos+2]-48;
        int currCol = (turn.player1.shot)[pos+4]-48;

        for (int row = 0; row < 10; row++){
            cout << gotoRowCol(row+8, 52); 
            for(int col=0; col < 10; col++){
                char ch = turn.player2.board[row][col];
                if(ch == WATER || (!endDisplay && ch == SHIP)){ 
                    if(row==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(CYAN);
                    }else{
                        cout << bgColor(CYAN) << fgColor(BLACK);
                    }
                    cout << WATER << flush;
                }else if(endDisplay && (ch == SHIP || ch == HIT)){
                    if(ch == HIT){
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    else if(ch == SHIP){
                        cout << bgColor(WHITE) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                    turn.player2.won = true;
                }else if(ch == HIT){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(LIGHT_MAGENTA);
                    }else{
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == MISS){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(GRAY);
                    }else{
                        cout << bgColor(GRAY) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == KILL){
                    if(row==currRow && col==currCol && !endDisplay){
                        cout << bgColor(BLACK) << fgColor(RED);
                    }else{
                        cout << bgColor(RED) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else{
                    cout << bgColor(RESET) << fgColor(WHITE) << ch << flush;
                }
            }
            cout << endl;
        }
        cout << bgColor(RESET) << fgColor(WHITE);
    }
    
}





