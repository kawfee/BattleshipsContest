#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <unistd.h>


#include "conio.cpp"
#include "conio.h"

int extractInteger(string str);
std::ifstream& GotoLine(std::ifstream& file, unsigned int num);

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
    

    regex match_start("^MATCH_START_ROUND:.*");
    regex match_end("^MATCH_OVER");

    cout << clrscr() << flush;

    string line;
    ifstream log_file ("logs/client_vs_client_auto.log");
    
    //print off dummy board
    cout << gotoRowCol(3, 1) << "author1" << endl;
    cout << gotoRowCol(4, 1) << "client1" << endl;
    
    cout << gotoRowCol(6, 1)  << " |0123456789" << endl
         << gotoRowCol(7, 1)  << "------------" << endl
         << gotoRowCol(8, 1)  << "A|~~~~~~~~~~" << endl
         << gotoRowCol(9, 1)  << "B|~~~~~~~~~~" << endl
         << gotoRowCol(10, 1) << "C|~~~~~~~~~~" << endl
         << gotoRowCol(11, 1) << "D|~~~~~~~~~~" << endl
         << gotoRowCol(12, 1) << "E|~~~~~~~~~~" << endl
         << gotoRowCol(13, 1) << "F|~~~~~~~~~~" << endl
         << gotoRowCol(14, 1) << "G|~~~~~~~~~~" << endl
         << gotoRowCol(15, 1) << "H|~~~~~~~~~~" << endl
         << gotoRowCol(16, 1) << "I|~~~~~~~~~~" << endl
         << gotoRowCol(17, 1) << "J|~~~~~~~~~~" << endl;
        
    cout << gotoRowCol(3, 50) << "author2" << endl;
    cout << gotoRowCol(4, 50) << "client2" << endl;
    
    cout << gotoRowCol(6, 50)  << " |0123456789" << endl
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
        while(getline (log_file,line)){
            if(regex_match(line, match_start)){
                //cout << "There was a beginning of a match!" << endl;
                cout << gotoRowCol(1,1) << "Match Number: " << extractInteger(line) << endl;
                // sleep(1);
            }else if(regex_match(line, match_end)){
                cout << gotoRowCol(1,1) << "Match Ended!" << endl;
                sleep(1);
            }else{
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

                // handle the board
                cout << gotoRowCol(8, 3)  << currentState[2]  << endl;
                cout << gotoRowCol(9, 3)  << currentState[3]  << endl;
                cout << gotoRowCol(10, 3) << currentState[4]  << endl;
                cout << gotoRowCol(11, 3) << currentState[5]  << endl;
                cout << gotoRowCol(12, 3) << currentState[6]  << endl;
                cout << gotoRowCol(13, 3) << currentState[7]  << endl;
                cout << gotoRowCol(14, 3) << currentState[8]  << endl;
                cout << gotoRowCol(15, 3) << currentState[9]  << endl;
                cout << gotoRowCol(16, 3) << currentState[10] << endl;
                cout << gotoRowCol(17, 3) << currentState[11] << endl;

                cout << gotoRowCol(19, 1) << currentState[25] << endl;

                cout << gotoRowCol(3, 50) << currentState[13] << endl;
                cout << gotoRowCol(4, 50) << currentState[14] << endl;

                // handle the board
                cout << gotoRowCol(8, 52)  << currentState[15]  << endl;
                cout << gotoRowCol(9, 52)  << currentState[16]  << endl;
                cout << gotoRowCol(10, 52) << currentState[17]  << endl;
                cout << gotoRowCol(11, 52) << currentState[18]  << endl;
                cout << gotoRowCol(12, 52) << currentState[19]  << endl;
                cout << gotoRowCol(13, 52) << currentState[20]  << endl;
                cout << gotoRowCol(14, 52) << currentState[21]  << endl;
                cout << gotoRowCol(15, 52) << currentState[22]  << endl;
                cout << gotoRowCol(16, 52) << currentState[23] << endl;
                cout << gotoRowCol(17, 52) << currentState[24] << endl;

                cout << gotoRowCol(19, 50) << currentState[12] << endl;

                cout << gotoRowCol(21, 1);
                char temp = cin.get();
                cin.clear();
                fflush(stdin);
                cout << gotoRowCol(21, 1) << "                                                   ";
                cout << gotoRowCol(22, 1) << "Temp: " << temp << endl;
                //sleep(1);
            }

        }

        log_file.close();
    }

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


/*
    Currently we go line by line through file and change data as necessary when we print.
    But what we should be doing is reading states of the game at a time to match how we will step forward and backwards through the file.
    That way we can go to previous board states easier need to figure out navigating to a specific line of a file.
*/


