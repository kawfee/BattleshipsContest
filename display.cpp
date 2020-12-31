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

int extractInteger(string str);
std::ifstream& GotoLine(std::ifstream& file, unsigned int num);
void printBoard(string currentState[30], int boardChoice);

/*
    string gotoRowCol( const int x, const int y );
    string fgColor( Color c );
    string bgColor( Color c );
    string setTextStyle( TextStyle ts );
    string resetAll( );
    string clrscr();
*/

//structs

struct playerData{
    string name;
    string author;
    char board[10][10];
    string shot_by;
};

struct turnStruct{
    playerData player1;
    playerData player2;
};

using namespace std;
using namespace conio;

int main(){
    
    //Both of these may need to change
    //map<int, turnStruct> match; 
    //map<int, map<int, turnStruct>> game; // Hash Map woot
    int matchIndex = 0;
    int gameIndex  = 0;

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
                    cout << clrscr() << flush;
                    cout << gotoRowCol(1,1) << "Match Number: " << extractInteger(line) << endl;
                    map<int, turnStruct> match;
                    game[gameIndex] = match;
                    matchIndex = 0;   
            }else if(regex_match(line, match_end)){
                cout << gotoRowCol(1,1) << "Match Ended!" << endl;
                gameIndex++;
            }else{
                //save match to current game match

                //we have already read in a line--don't forget to save that one into the struct as well
                //game
                game[gameIndex][matchIndex]
                for (int i=1; i < 26; i++){
                    getline(log_file, line);
                    currentState[i] = line;
                }
            }
        }
    }

    /* 
        if(log_file.is_open()){
            while(getline (log_file,line)){
                file_line++;
                if(regex_match(line, match_start)){
                    //cout << "There was a beginning of a match!" << endl;
                    cout << clrscr() << flush;
                    cout << gotoRowCol(1,1) << "Match Number: " << extractInteger(line) << endl;
                    min_file_line = file_line;
                    // sleep(1);
                }else if(regex_match(line, match_end)){
                    cout << gotoRowCol(1,1) << "Match Ended!" << endl;
                    // sleep(1);
                }else{
                    cout << gotoRowCol(30,1) << "file_line: " << file_line << "                                                               " << endl;
                    //author            [0]     [13]
                    //client's board    [1]     [14] 
                    //board x10         [2-11]  [15-24]
                    //client's shot     [25]    [12]

                    string currentState[30];
                    currentState[0] = line;
                    
                    for (int i=1; i < 26; i++){
                        getline(log_file, line);
                        currentState[i] = line;
                    }

                    cout << gotoRowCol(3, 1) << currentState[0] << endl;
                    cout << gotoRowCol(4, 1) << currentState[1] << endl;
                    
                    printBoard(currentState, 1);

                    cout << gotoRowCol(19, 1) << "                                                                                                                   ";
                    cout << gotoRowCol(19, 1) << currentState[25] << endl;
                    

                    cout << gotoRowCol(3, 50) << currentState[13] << endl;
                    cout << gotoRowCol(4, 50) << currentState[14] << endl;

                    printBoard(currentState, 2);

                    cout << gotoRowCol(19, 50) << currentState[12] << endl;

                    cout << gotoRowCol(21, 1);

                    char temp;
                    //getline(cin, temp_str);
                    temp = cin.get();
                    //char temp = temp_str[0];
                    //char temp[] = cin.get();
                    //cin.clear();
                    if(temp != '\n'){
                        cin.ignore(1024, '\n');
                    }
                    //fflush(stdin);
                    // This next line is for clearing the input to blank
                    cout << gotoRowCol(21, 1) << "                                                                                                                   ";
                    cout << gotoRowCol(22, 1) << "Temp: " << temp << endl;
                    if(temp == 'b' || temp == 'r'){
                        int test = file_line - 27;
                        if(test < min_file_line){
                            file_line -= 1;
                            cout << gotoRowCol(30,1) << "Can't go to line; you are too far back. New File Line: " << file_line << endl;
                        }else{
                            file_line -= 27;
                            GotoLine(log_file, file_line);
                            cout << gotoRowCol(30,1) << "Went to file_line: " << file_line << "                                                                                    " << endl;
                        }
                    }else {
                        file_line += 25;
                    }
                    //sleep(1);
                }

            }

            log_file.close();
        } 
    */

    return 0;
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


std::ifstream& GotoLine(std::ifstream& file, unsigned int num){
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    }
    return file;
}

void printBoard(string currentState[30], int boardChoice){
    if(boardChoice==1){
        int pos = currentState[25].find(":");
        int currRow = (currentState[25])[pos+2]-48;
        int currCol = (currentState[25])[pos+4]-48;

        cout << gotoRowCol(25, 1)
             << "currRow: " << currRow << endl 
             << "currCol: " << currCol << endl;

        for (int row = 2; row < 12; row++){
            cout << gotoRowCol(row+6, 3); 
            for(int col=0; col < 10; col++){
                char ch = (currentState[row])[col];
                if(ch == WATER || ch == SHIP){ 
                    if(row-2==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(CYAN);
                    }else{
                        cout << bgColor(CYAN) << fgColor(BLACK);
                    }
                    cout << WATER << flush;
                }else if(ch == HIT){
                    if(row-2==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(LIGHT_MAGENTA);
                    }else{
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == MISS){
                    if(row-2==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(GRAY);
                    }else{
                        cout << bgColor(GRAY) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == KILL){
                    if(row-2==currRow && col==currCol){
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
        int pos = currentState[12].find(":");
        int currRow = (currentState[12])[pos+2]-48;
        int currCol = (currentState[12])[pos+4]-48;

        cout << gotoRowCol(25, 52)
             << "currRow: " << currRow << gotoRowCol(26, 52) 
             << "currCol: " << currCol << endl;

        for (int row = 15; row < 25; row++){
            cout << gotoRowCol(row-7, 52); 
            for(int col=0; col < 10; col++){
                char ch = (currentState[row])[col];
                if(ch == WATER || ch == SHIP){ 
                    if(row-15==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(CYAN);
                    }else{
                        cout << bgColor(CYAN) << fgColor(BLACK);
                    }
                    cout << WATER << flush;
                }else if(ch == HIT){
                    if(row-15==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(LIGHT_MAGENTA);
                    }else{
                        cout << bgColor(LIGHT_MAGENTA) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == KILL){
                    if(row-15==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(RED);
                    }else{
                        cout << bgColor(RED) << fgColor(BLACK);
                    }
                    cout << ch << flush;
                }else if(ch == MISS){
                    if(row-15==currRow && col==currCol){
                        cout << bgColor(BLACK) << fgColor(GRAY);
                    }else{
                        cout << bgColor(GRAY) << fgColor(BLACK);
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





