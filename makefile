CXXFLAGS 	= -std=c++11 -Wall -O3




all: controller
	cd AI_Files; make all; cd ..; 

controller: controller.cpp server.cpp display.cpp
	g++ $(CXXFLAGS) -o controller controller.cpp

clean:
	rm -f ./AI_Executables/* controller; touch ./AI_Executables/.gitkeep; rm -f controller;


