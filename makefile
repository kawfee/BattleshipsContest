CXXFLAGS= -std=c++11 -Wall
ALLOBJECTS= client client_manual controller

# is this /* working? 
object_files: client client_manual controller
	chmod 755 ./client_Ais/*
	chmod 755 controller
client: client.cpp
client_manual: client_manual.cpp
controller: controller.cpp

# How to deal with making executables in sub-directory for a cleaner file system
move:
	make; mv client client_Ais; mv client_manual client_Ais;

clean:
	rm -f ./client_Ais/client ./client_Ais/client_manual controller
