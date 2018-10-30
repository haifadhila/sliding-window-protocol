/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <strings.h>
#include <iostream>
using namespace std;

#define BUFFSIZE 1024
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int writeFile(char* message, char* outputfile){
    ofstream filename;
    filename.open(outputfile, ofstream::binary);

    int size = sizeof(message);
    filename.write(message,size);
    filename.close();
    return 0;
}
 

int main(int argc, char *argv[])
{
    if (argc != 5){
        cout << "input the right message." << endl;
        cout << "./recvfile filename windowsize buffersize port" << endl;
        exit(0);
    } else {
		filename = argv[1];
        windowsize = atoi(argv[2]);
        bufferSize = atoi(argv[3]);
        port = atoi(argv[4]);
    }

     int sockfd, portno;
     int windowSize;
     socklen_t clilen;
     char buffer[1024];
     struct sockaddr_in server_addr, cli_addr;
     int n;
     int recvlen;
     

     // Create Socket
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

    memset((char*)&server_addr, 0 ,sizeof(server_addr));

    //bind socke
	 server_addr.sin_family = AF_INET;
	 server_addr.sin_addr.s_addr = INADDR_ANY;
	 server_addr.sin_port = htons(port);

     if (bind(sockfd, (struct sockaddr *) &server_addr,
              sizeof(server_addr)) < 0) 
              error("Bind error");

     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     

     for (;;) {
		printf("waiting on port %d\n", SERVICE_PORT);
		recvlen = recvfrom(sockfd, buffer, BUFFSIZE, 0, (struct sockaddr *)&cli_addr, &clilen);
		if (recvlen > 0) {
			buffer[recvlen] = 0;
			printf("received message: \"%s\" (%d bytes)\n", buffer, recvlen);
			writeFile(buffer,filename);
		}
		else
			printf("uh oh - something went wrong!\n");
		sprintf(buffer, "ack %d", msgcnt++);
		printf("sending response \"%s\"\n", buffer);
		if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cli_addr, clilen) < 0)
			perror("sendto");
	}
     

     
     printf("Here is the message: %s\n",buffer);
     
     close(newsockfd);
     close(sockfd);
     return 0; 
}