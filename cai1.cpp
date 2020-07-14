// C++ program to implement FIFO
// This side reads first, then reads 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <iostream>


int main() 
{ 
	int fd1;
	int fd2; 

	// FIFO file path 
	char * myfifo1 = "./cta1";
	char * myfifo2 = "./atc1"; 

	// Creating the named file(FIFO) 
	// mkfifo(<pathname>,<permission>) 
	//mkfifo(myfifo, 0666); 

	char str1[80], str2[80]; 
	while (1) 
	{ 
		// First open in read only and read 
		fd1 = open(myfifo1, O_RDONLY); 
		read(fd1, str1, 80); 

		// Print the read string and close 
        std::cout << "User1:" << str1 <<std::endl;
		//printf("User1: %s\n", str1); 
		close(fd1); 
    
		// Now open in write mode and write 
		// string taken from user.
        std::cout << "About to open write" << std::endl; 
		fd2 = open(myfifo2,O_WRONLY); 
		strncpy(str2, "hello world\n", strlen("hello world\n"));
		write(fd2, str2, strlen(str2)+1); 
        std::cout << "Wrote message" << std::endl;
		close(fd2); 
	} 
	return 0; 
} 
