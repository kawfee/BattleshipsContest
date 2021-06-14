CXXFLAGS	= -std=c++11 -Wall -O3

SRCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp, %, $(SRCS))

all: $(PROGS)
	mv *.ai ../AI_Executables

gambler: gambler_player.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o gambler_player.ai gambler_player.cpp; mv *.ai ../AI_Executables;

gambler_bouch: gambler_player_bouch.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o gambler_player_bouch.ai gambler_player_bouch.cpp; mv *.ai ../AI_Executables;

gambler_joey: gambler_player_joey.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o gambler_player_joey.ai gambler_player_joey.cpp; mv *.ai ../AI_Executables;

%: %.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o $@.ai $<





