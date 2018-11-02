all: recvfile sendfile

recvfile: server.cpp
	g++ -std=c++11 -pthread server.cpp -o recvfile

sendfile: client.cpp
	g++ -std=c++11 -pthread client.cpp -o sendfile

clean: recvfile sendfile
	rm -f recvfile sendfile
