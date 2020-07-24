#include <stdlib.h>
#include <iostream>
#include "subprocess.hpp"
#include <unistd.h>

using namespace std;
using namespace subprocess;

int main(){

    auto p1 = Popen({"./client"});
    auto p2 = Popen({"./client"});
    usleep(2000);
    p1.kill();
    p2.kill();
    /*
    i=system("./server");
    i=system("./client");
    i=system("./client2");
    */
    return 0;
}

/*
message sent
message received
response sent
response recieved



*/
