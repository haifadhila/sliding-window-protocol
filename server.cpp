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
#define MAX_FRAME 1034

struct ack{
  char ack;
  int Next_SeqNum;
  char checksum;
} ack_packet;

typedef struct{
  char SOH;
  int SequenceNumber;
  int DataLength;
  char* data;
  char checksum;
}packet;

void error(const char *msg)
{
    perror(msg);
    exit(1);
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
    if (sockfd < 0){
     error("ERROR opening socket");
    }

    if (bind(sockfd, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
     error("Bind error");
    }

    clilen = sizeof(cli_addr);

    char* ack_send=(char*)malloc(6);
    char* recvbuffer = (char*)malloc(MAX_FRAME);
    char* content = (char*)malloc(BUFFSIZE);

    //Create File
    FILE *fstream;
    fstream = fopen(filename, "a+");

    int lfr = -1;
    int laf = lfr + windowSize;

    //make array of packet
    packet recv_packet[laf];

    printf("waiting on port %d\n", port);
    for (;;) {
		  recvlen = recvfrom(sockfd, recvbuffer, MAX_FRAME, 0, (struct sockaddr *)&cli_addr, &clilen);
      if (recvlen < 0) continue;
      packet recvFrame;
      recvFrame.SOH = recvbuffer[0];
      recvFrame.SequenceNumber = (recvbuffer[1] << 24) | (recvbuffer[2] << 16) | (recvbuffer[3] << 8) | (recvbuffer[4]);
      cout << recvFrame.SequenceNumber << endl;
      recvFrame.DataLength = (recvbuffer[5] << 24) | (recvbuffer[6] << 16) | (recvbuffer[7] << 8) | recvbuffer[8];
      recvFrame.data = (char*)malloc(recvFrame.DataLength);
      for(int i = 0; i < recvFrame.DataLength ; i++){
        recvFrame.data[i] = recvbuffer[9+i];
      }
      recvFrame.checksum = recvbuffer[recvFrame.DataLength + 9];

      int datalen = (recvbuffer[5] << 24) | (recvbuffer[6] << 16) | (recvbuffer[7] << 8) | recvbuffer[8];
      cout << "data length = " << datalen << endl;

      //checksum dari recvbuffer yang diterima
      char checksum = 0;
      for (int i=0; i<datalen+10; i++) {
        checksum += recvbuffer[i];
      }

      cout << lfr << endl;
      cout << recvFrame.SequenceNumber -1 << endl;
      printf("%x\n", checksum);
      if (lfr == (recvFrame.SequenceNumber - 1)){
        if (checksum == 0xFFFFFFFF){
          printf("Sent successfully\n");
          ack_packet.ack = 0x06;
          ack_packet.Next_SeqNum = ((recvbuffer[1] << 24) | (recvbuffer[2] << 16) | (recvbuffer[3] << 8) | (recvbuffer[4]) + 1);
          ack_packet.checksum = checksum;

          //generate ack to send
          ack_send[0] = ack_packet.ack;
          ack_send[1] = (ack_packet.Next_SeqNum >> 24) && 0xFF;
          ack_send[2] = (ack_packet.Next_SeqNum >> 16) && 0xFF;
          ack_send[3] = (ack_packet.Next_SeqNum >> 8) && 0xFF;
          ack_send[4] = (ack_packet.Next_SeqNum) && 0xFF;
          ack_send[5] = ack_packet.checksum;

          //insert packet to array
          recv_packet[(recvFrame.SequenceNumber % laf)].SOH = recvFrame.SOH;
          recv_packet[(recvFrame.SequenceNumber % laf)].SequenceNumber = recvFrame.SequenceNumber;
          recv_packet[(recvFrame.SequenceNumber % laf)].DataLength = recvFrame.DataLength;
          recv_packet[(recvFrame.SequenceNumber % laf)].data = recvFrame.data;
          cout << "success inserting until data length" << endl;
          recv_packet[(recvFrame.SequenceNumber % laf)].checksum = recvFrame.checksum;


          printf("THIS IS ACK %d\n", ack_packet.Next_SeqNum);
          std::cout << "sending response" << '\n';
          if (sendto(sockfd,ack_send, 6, 0, (struct sockaddr *)&cli_addr, clilen) < 0){
            cerr << "Error sending ACK" << '\n';
          };
          lfr = lfr + 1;
          laf = lfr + windowSize;
        } else {
          cout << "paket loss" << endl;
          ack_packet.ack = 0x15;
          ack_packet.Next_SeqNum = lfr + 1;
          ack_packet.checksum = checksum;

          //generate ack to send
          ack_send[0] = ack_packet.ack;
          ack_send[1] = (ack_packet.Next_SeqNum >> 24) && 0xFF;
          ack_send[2] = (ack_packet.Next_SeqNum >> 16) && 0xFF;
          ack_send[3] = (ack_packet.Next_SeqNum >> 8) && 0xFF;
          ack_send[4] = (ack_packet.Next_SeqNum) && 0xFF;
          ack_send[5] = ack_packet.checksum;

          printf("THIS IS NAK %d\n", ack_packet.Next_SeqNum);
          std::cout << "sending response" << '\n';
          if (sendto(sockfd,ack_send, 6, 0, (struct sockaddr *)&cli_addr, clilen) < 0){
            cerr << "Error sending NAK" << '\n';
          };
        }
      } else {
        cout << "hello" << endl;
        if ((checksum == 0xFFFFFFFF) && (recvFrame.SequenceNumber < laf)){
          cout << "paket masuk tapi nak" << endl;
          ack_packet.ack = 0x15;
          ack_packet.Next_SeqNum = lfr + 1;
          ack_packet.checksum = checksum;

          //generate ack to send
          ack_send[0] = ack_packet.ack;
          ack_send[1] = (ack_packet.Next_SeqNum >> 24) && 0xFF;
          ack_send[2] = (ack_packet.Next_SeqNum >> 16) && 0xFF;
          ack_send[3] = (ack_packet.Next_SeqNum >> 8) && 0xFF;
          ack_send[4] = (ack_packet.Next_SeqNum) && 0xFF;
          ack_send[5] = ack_packet.checksum;

          printf("THIS IS NAK %d\n", ack_packet.Next_SeqNum);
          std::cout << "sending response" << '\n';
          if (sendto(sockfd,ack_send, 6, 0, (struct sockaddr *)&cli_addr, clilen) < 0){
            cerr << "Error sending NAK" << '\n';
          };

          recv_packet[(recvFrame.SequenceNumber % laf)].SOH = recvFrame.SOH;
          recv_packet[(recvFrame.SequenceNumber % laf)].SequenceNumber = recvFrame.SequenceNumber;
          recv_packet[(recvFrame.SequenceNumber % laf)].DataLength = recvFrame.DataLength;
          recv_packet[(recvFrame.SequenceNumber % laf)].data = (char*)malloc(sizeof(recvFrame.data));
          strcpy(recv_packet[(recvFrame.SequenceNumber % laf)].data, recvFrame.data);
          recv_packet[(recvFrame.SequenceNumber % laf)].checksum = recvFrame.checksum;

        } else {
          cout << "checksum beda" << endl;
          if (checksum != 0xFFFFFFFF){
            ack_packet.ack = 0x15;
            ack_packet.Next_SeqNum = lfr + 1;
            ack_packet.checksum = checksum;

            //generate ack to send
            ack_send[0] = ack_packet.ack;
            ack_send[1] = (ack_packet.Next_SeqNum >> 24) && 0xFF;
            ack_send[2] = (ack_packet.Next_SeqNum >> 16) && 0xFF;
            ack_send[3] = (ack_packet.Next_SeqNum >> 8) && 0xFF;
            ack_send[4] = (ack_packet.Next_SeqNum) && 0xFF;
            ack_send[5] = ack_packet.checksum;

            printf("THIS IS NAK %d\n", ack_packet.Next_SeqNum);
            std::cout << "sending response" << '\n';
            if (sendto(sockfd,ack_send, 6, 0, (struct sockaddr *)&cli_addr, clilen) < 0){
              cerr << "Error sending NAK" << '\n';
            };
          }
        }
        for(int i = 0; i < laf; i++){
          fwrite(recv_packet[i].data, sizeof(char), sizeof(recv_packet[i].data), fstream);
        }
      }
    }
   printf("Here is the message: %s\n",buffer);
   fclose(fstream);
   close(sockfd);
   return 0;
}
