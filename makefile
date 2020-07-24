CXXFLAGS= -std=c++11 -Wall
ALLOBJECTS= client client2 server controller

object_files: client client2 server controller
	chmod 755 client
	chmod 755 client2
	chmod 755 server
	chmod 755 controller
client: client.cpp
client2: client2.cpp
server: server.cpp
controller: controller.cpp

clean:
	rm -f client client2 server controller

