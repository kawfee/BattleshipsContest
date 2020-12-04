#include <stdlib.h>
#include <iostream>
#include "subprocess.hpp"
#include <unistd.h>
#include "server.cpp"
#include <string.h>

using namespace std;
using namespace subprocess;

int main(){

    /*
        make a list of all AI executable files
        call runGame() for those files and do tourney cr-arbage
    */
    
    string clientNameOne = "client";
    string clientNameTwo = "client";

    runGame(10, clientNameOne, clientNameTwo);
    

    return 0;
}

/*
message sent
message received
response sent
response recieved



*/
