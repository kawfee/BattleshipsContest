#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

int main() 
{ 
    int fd;
    
    // FIFO file path 
    const char * myfifo1 = "./cta1"; 
  
    // Creating the named file(FIFO) 
    // mkfifo(<pathname>, <permission>) 
    //mkfifo(myfifo1, 0666); 
  
    char arr1[80], arr2[80]; 
    while (1) 
    { 
  
        // Open FIFO for Read only 
        fd = open(myfifo1, O_RDONLY); 
  
        // Read from FIFO 
        read(fd, arr1, sizeof(arr1)-1); 
  
        // Print the read message 
        printf("User2: %s\n", arr1); 
        close(fd); 
        printf("closed successfully\n");
        
        // Open FIFO for write only 
        fd = open(myfifo1, O_WRONLY); 
        printf("opened succ\n");
  
        // Take an input arr2ing from user. 
        // 80 is maximum length 
        //fgets(arr2, 80, stdin); 
        for(int i =0;i<10;i++){
            arr2[i]=(char)i;
        }
        arr2[10]='\0';
        printf("for succ\n");
        
        // Write the input arr2ing on FIFO 
        // and close it 
        printf("GOT TO THE WRITE!\n");
        write(fd, arr2, strlen(arr2)+1); 
        close(fd) ;

    } 
    return 0; 
} 

