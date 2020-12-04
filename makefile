CXXFLAGS= -std=c++11 -Wall
ALLOBJECTS= client client_manual controller #server

object_files: client client_manual controller #server
	chmod 755 client
	chmod 755 client_manual
#	chmod 755 server
	chmod 755 controller
client: client.cpp
client_manual: client_manual.cpp
#server: server.cpp
controller: controller.cpp

clean:
	rm -f client client_manual controller
