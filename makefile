CXXFLAGS = -Og -std=c++11 -Wall -g -pg -no-pie -fno-builtin
LDFLAGS  = -pg

# is this /* working? 
object_files: client client_auto client_manual controller
	chmod 755 ./AI_Executables/*
	chmod 755 controller
client: client.cpp
client_auto: client_auto.cpp
client_manual: client_manual.cpp
controller: controller.cpp

# How to deal with making executables in sub-directory for a cleaner file system
move:
	make; mv client AI_Executables; mv client_auto AI_Executables; mv client_manual AI_Executables;

clean:
	rm -f ./AI_Executables/client ./AI_Executables/client_auto ./AI_Executables/client_manual controller ais.txt



# CXXFLAGS = -Og -Wall -g -pg -no-pie -fno-builtin
# LDFLAGS = -pg

# g++ $(CXXFLAGS) -o $(EXEC) $(OBJ) $(LDFLAGS)

# profile:
#         gprof infectsim | more
