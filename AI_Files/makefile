CXXFLAGS	= -std=c++11 -Wall -O3

SRCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp, %, $(SRCS))

all: $(PROGS)
	mv *.ai ../AI_Executables

%: %.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o $@.ai $<




























# EXEC_DIR		= ../AI_Executables
# CXXFLAGS	= -std=c++11 -Wall -O3


# $(EXEC_DIR)/%_ai : %.cpp
# 	$(CPP) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $< -o $@;

# all: $(patsubst %.cpp, %_ai, $(wildcard *.cpp))
# 	mv *_ai $(EXEC_DIR)

