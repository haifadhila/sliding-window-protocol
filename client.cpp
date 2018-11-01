#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

#define BUFFSIZE 1024
#define MAX_FRAME 1034
#define ACKSIZE 6
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// Reading File eksternal
int readFile(string inputfile, string &message){
    string line;
    char charFile;
    ifstream filename;

    filename.open(inputfile);

    if (filename.is_open()){
        while(getline(filename, line)){
            message = message + line;
        }
    }else{
        cout << "Can't open file" << endl;
    }
}

unsigned long getFileSize(string filename){
  FILE *p_file = NULL;
  p_file = fopen(filename.c_str(),"rb");
  fseek(p_file,0,SEEK_END);
  int size = ftell(p_file);
  fclose(p_file);
  return size;
}

int main(int argc, char *argv[])
{
    char* filename;
    char* ipaddr;
    int windowSize, bufferSize, port;
    int recvlen;

    if (argc != 6){
        cout << "wrong input" << endl;
        cout << "./sendfile filename windowsize buffersize IPaddress port" << endl;
        exit(0);
    }else{
        filename = argv[1];
        windowSize = atoi(argv[2]);
        bufferSize = atoi(argv[3]);
        ipaddr = argv[4];
        port = atoi(argv[5]);
    }

    int sockfd, n;
    struct sockaddr_in serv_addr;  //server address
    struct sockaddr_in cli_addr; // my address

    const char* buffer;

    // Create Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
        error("ERROR opening socket");

    int slen = sizeof(serv_addr);

    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0) {
        perror("bind failed");
        return 0;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_aton(ipaddr, &serv_addr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // read file and casting the message to buffer

    string message;
    readFile(filename, message);
    printf("%s\n", message.c_str());
    buffer = message.c_str();
    printf("%s\n", buffer);

    int total_frame;
    float x;
    x = getFileSize(filename)/(float)1024;
    total_frame = ceil(x);
    // msg is frame with SOH, Sequence Number, Data Length, Data, and checksum
    for (int i=0; i < total_frame; i++) {
        char* msg=(char*)malloc(MAX_FRAME);
        int n = 0;
        msg[0] = 0x1;
        msg[1] = (n >> 24) & 0xFF;
        msg[2] = (n >> 16) & 0xFF;
        msg[3] = (n >> 8) & 0xFF;
        msg[4] = n & 0xFF;
        int msglen = strlen(buffer);
        printf("msglen= %d\n", msglen);
        msg[5] = (msglen >> 24) & 0xFF;
        msg[6] = (msglen >> 16) & 0xFF;
        msg[7] = (msglen >> 8) & 0xFF;
        msg[8] = msglen & 0xFF;

        for (int i=0; i<msglen; i++){
            msg[9+i] = buffer[i];
        }

        for (int i=0; i<msglen; i++) {
            printf("%c", msg[9+i]);
        }

        char checksum = 0;
        for (int i=0; i<msglen+9; i++){
            checksum += msg[i];
        }
        checksum = ~checksum;

        msg[msglen+9] = checksum;

        printf("Sending packet to %s port %d\n",ipaddr, port);

        // int lar = 0;
        // int lfs = 0;
        // int j = 0;
        // int totalpacket = 0;
        // while((lfs - lar) <= windowSize){
        //   if (strlen(msg) == MAX_FRAME){
        //     totalpacket++;
        //   }
        //   if (sendto(sockfd, msg, MAX_FRAME, 0, (struct sockaddr *)&serv_addr, slen) == -1){
        //     std::cerr << "Error sending packet" << '\n';
        //     exit(1);
        //   }
        //   lfs++;
        // }
        //
        //   char* ack = (char*)malloc(6);
        //   recvlen = recvfrom(sockfd, ack, 6, 0, (struct sockaddr *)&serv_addr, &slen);
        //   if (recvlen >= 0) {
        //     ack[recvlen] = 0;	/* expect a printable string - terminate it */
        //     printf("received message: \"%s\"\n", ack);
        //     lar++;
        //   }
        // }
        if (sendto(sockfd, msg, MAX_FRAME, 0, (struct sockaddr *)&serv_addr, slen) == -1){
          cerr << "error sending packet";
          exit(1);
        }

        char* ack = (char*)malloc(6);
        recvlen = recvfrom(sockfd, ack, 6, 0, (struct sockaddr *)&serv_addr, &slen);
          if (recvlen >= 0) {
            ack[recvlen] = 0;	/* expect a printable string - terminate it */
            printf("received message: \"%s\"\n", ack);
        }
    }
    close(sockfd);
    return 0;
}
