CXXFLAGS	= -std=c++11 -Wall -O3

SRCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp, %, $(SRCS))

all: $(PROGS)
	mv *.ai ../AI_Executables

gambler: gambler_player.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o gambler_player.ai gambler_player.cpp; mv *.ai ../AI_Executables;

%: %.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o $@.ai $<





