CXXFLAGS= -std=c++11 -Wall
ALLOBJECTS= client client_manual controller

# is this /* working? 
object_files: client client_manual controller
	chmod 755 ./AI_Executables/*
	chmod 755 controller
client: client.cpp
client_manual: client_manual.cpp
controller: controller.cpp

# How to deal with making executables in sub-directory for a cleaner file system
move:
	make; mv client AI_Executables; mv client_manual AI_Executables;

clean:
	rm -f ./AI_Executables/client ./AI_Executables/client_manual controller ais.txt
