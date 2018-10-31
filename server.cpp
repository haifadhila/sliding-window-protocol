/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <strings.h>
#include <iostream>
using namespace std;

#define ACKSIZE 6
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
    unsigned int windowSize;
    char* buffer;
    int bufferSize;
    char* filename;
    unsigned int port;
    int msgcnt;

    if (argc != 5){
        cout << "input the right message." << endl;
        cout << "./recvfile filename windowsize buffersize port" << endl;
        exit(0);
    } else {
		    filename = argv[1];
        windowSize = atoi(argv[2]);
        bufferSize = atoi(argv[3]);
        port = atoi(argv[4]);
    }

     int sockfd;
     socklen_t clilen;
     struct sockaddr_in server_addr, cli_addr;
     int n;
     int recvlen;

     //set memory
     memset((char*)&cli_addr,0,sizeof(cli_addr));
     memset((char*)&server_addr,0,sizeof(server_addr));
  	 server_addr.sin_family = AF_INET;
  	 server_addr.sin_addr.s_addr = INADDR_ANY;
  	 server_addr.sin_port = htons(port);

     // Create Socket
     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");

     if (bind(sockfd, (struct sockaddr *) &server_addr,
              sizeof(server_addr)) < 0)
              error("Bind error");
     clilen = sizeof(cli_addr);

     int success=0;
    for (;;) {
        char* msg=(char*)malloc(1034);
        char* ack=(char*)malloc(6);
		    printf("waiting on port %d\n", port);
		    recvlen = recvfrom(sockfd, msg, BUFFSIZE, 0, (struct sockaddr *)&cli_addr, &clilen);
        printf("%s\n", buffer);
		    if (recvlen > 0) {
            int datalen = (msg[5] << 24 | msg[6] << 16 | msg[7] << 8 | msg[8]);
			       char checksum = 0;
              for (int i=0; i<datalen+10; i++){
                checksum += msg[i];
              }
              if (checksum == 0xFFFFFFFF){
                printf("Sent successfully\n");
                success++;
              }
             printf("received message: \"%s\" (%d bytes)\n", msg, recvlen);
			       writeFile(msg,filename);
		    }else{
              printf("uh oh - something went wrong!\n");
        }

  		sprintf(buffer, "ack %d", msgcnt++);
  		printf("sending response \"%s\"\n", buffer);
  		if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cli_addr, clilen) < 0)
  			perror("sendto");
	}

     printf("Here is the message: %s\n",buffer);
     close(sockfd);
     return 0;
}
