CXXFLAGS 	= -std=c++11 -Wall -O3
ORIGIN_DIR 	= AI_Files
TARGET_DIR 	= AI_Executables
OBJ 		= $(addprefix $(TARGET_DIR)/, $(patsubst %.cpp, %.o, $(wildcard *.cpp)))

# object_files: client client_auto controller
# client_auto: client_auto.cpp
# client: client.cpp
# controller: controller.cpp

# # How to deal with making executables in sub-directory for a cleaner file system
# move:
# 	make; mv client_auto AI_Executables; mv client AI_Executables;

all: $(patsubst ./$(ORIGIN_DIR)%.cpp, $(TARGET_DIR)%.out, $(wildcard *.cpp))
%.out: %.cpp makefile
	g++ $< -o $@ $(CXXFLAGS)

clean:
	rm -f ./AI_Executables/* controller; touch ./AI_Executables/.gitkeep;


