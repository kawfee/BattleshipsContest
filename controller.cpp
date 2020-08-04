#include <stdlib.h>
#include <iostream>
#include "subprocess.hpp"
#include <unistd.h>

using namespace std;
using namespace subprocess;

int main(){

    auto p1 = Popen({"./client"});
    auto p2 = Popen({"./client"});
    usleep(9000);

    // TODO: killing only affects one of the processes.
    p1.kill();
    p2.kill();

    return 0;
}

/*
message sent
message received
response sent
response recieved



*/
