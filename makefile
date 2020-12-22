#CXXFLAGS = -Og -std=c++11 -Wall -g -no-pie -fno-builtin
#LDFLAGS  = -pg

CXXFLAGS = -std=c++11 -Wall

object_files: client client_auto controller
client_auto: client_auto.cpp
client: client.cpp
controller: controller.cpp

# How to deal with making executables in sub-directory for a cleaner file system
move:
	make; mv client_auto AI_Executables; mv client AI_Executables;

clean:
	rm -f ./AI_Executables/client_auto ./AI_Executables/client controller


















# CXXFLAGS = -Og -std=c++11 -Wall -g -no-pie -fno-builtin
# LDFLAGS  = -pg

# # is this /* working? 
# object_files: client client_auto client_manual controller
# 	chmod 755 ./AI_Executables/*
# 	chmod 755 controller
# client: client.cpp
# client_auto: client_auto.cpp
# client_manual: client_manual.cpp
# controller: controller.cpp

# # How to deal with making executables in sub-directory for a cleaner file system
# move:
# 	make; mv client AI_Executables; mv client_auto AI_Executables; mv client_manual AI_Executables;

# controller:
# 	g++ $(CXXFLAGS) -o controller controller.cpp $(LDFLAGS)

# clean:
# 	rm -f ./AI_Executables/client ./AI_Executables/client_auto ./AI_Executables/client_manual controller ais.txt

# # CXXFLAGS = -Og -Wall -g -pg -no-pie -fno-builtin
# # LDFLAGS = -pg

# # g++ $(CXXFLAGS) -o $(EXEC) $(OBJ) $(LDFLAGS)

# # profile:
# #         gprof infectsim | more

