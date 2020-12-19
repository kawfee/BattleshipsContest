#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <regex>
#include <sstream>

#include <time.h>

#include "conio.cpp"
#include "conio.h"

int extractIntegerWords(string str);

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
    

    // regex round_start("^MATCH_START_ROUND:.*");

    // //cout << clrscr();

    // string line;
    // ifstream log_file ("logs/client_vs_client_auto.log");
    
    // if(log_file.is_open()){
    //     while(getline (log_file,line)){
    //         if(regex_match(line, round_start)){
    //             cout << "There was a beginning of a match!" << endl;
    //             cout << "Match Number: " << extractIntegerWords(line) << endl;
    //         }
    //     }

    //     log_file.close();
    // }

    return 0;
}



int extractIntegerWords(string str) { 
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

