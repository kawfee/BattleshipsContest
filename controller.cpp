#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fstream>

#include "subprocess.hpp"
#include "server.cpp"
#include "defines.h"


using namespace std;
using namespace subprocess;


int main(){
    srand(time(NULL));

    // Edit how many players here
    const int playerCount = 5;

    /*
        make a list of all AI executable files
        call runGame() for those files and do tourney cr-arbage
    */
    Player players[playerCount];
    system("ls ./client_Ais > temp.txt");
    int size = 0;
    string line;
    ifstream myfile ("temp.txt");
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

    /* for(int i = 0; i < size; i++){
        for(int j = i + 1; j < size; j++){
            cout << players[i].name << " VS " << players[j].name << endl;
            //have runGame return the struct that we built at the top
            //runGame(500, players[i].name, players[j].name);
            players[i].wins++;
            players[j].losses++;
            cout << players[i].name << " Wins!" << " Total Wins: " << players[i].wins << endl;
            cout << players[j].name << " Loses!" << " Total Losses: " << players[j].losses << endl;
        }
    } */

    string clientNameOne = players[2].name;
    string clientNameTwo = players[0].name; // will change to one at some point

    cout << "clientNameOne: " << clientNameOne << endl
         << "clientNameTwo: " << clientNameTwo << endl;

    int boardSize = 10;
    int numGames = 10;

    cout << runGame(numGames, players[2], players[0], boardSize) << endl;
    cout << "^^^ Game returned result" << endl;

    cout << "PLAYER[2] RECORD W-L-T: " << players[2].wins << " " << players[2].losses << " " << players[2].ties << endl;
    cout << "PLAYER[0] RECORD W-L-T: " << players[0].wins << " " << players[0].losses << " " << players[0].ties << endl;

    return 0;
}












/* 
    string movePlayer(int choice, Player arr[], Player arr2[], int size){
        Player ans=arr[choice];

        int i=choice;
        while(i<size){
            arr[i]=arr[i+1];
            i++;
        }
        arr[i]=NULL;

        i=0;
        while(i<size){
            if(arr2[i]==NULL){
                arr2[i]=ans;
                break;
            }
            i++;
        }

        return ans.name;
    } 
*/
