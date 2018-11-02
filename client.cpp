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
//reading file eksternal All
int readFileAll(string inputfile, string &message){
  string line;
  ifstream fstream;
  fstream.open(inputfile);

  if (fstream.is_open()){
    while (getline(fstream,line)){
      message = message + line;
    }
  }else{
    cout << "Can't open file";
  }
  return 0;
}

// Reading File eksternal per buffer
char* readFile(string inputfile, int bufferSize){
    char* message;
    char charFile;
    ifstream filename;
    int max_read_file , i;

    max_read_file = bufferSize * MAX_FRAME;
    message = (char*)malloc(max_read_file);
    i = 0;
    filename.open(inputfile);
    if (filename.is_open()){
      while (!filename.eof() && (i < max_read_file)){
          filename.get(charFile);
          message[i] = charFile;
          i++;
      }
      return message;
    }else{
      cout << "Can't read the file";
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

char checksumVal(int msglen, char* msg){
  char checksum = 0;
  for (int i=0; i<msglen+9; i++){
      checksum += msg[i];
  }
  checksum = ~checksum;

  msg[msglen+9] = checksum;

  return checksum;
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
    const char* buffer;
    string message;
    // char* message = (char*)malloc(bufferSize*BUFFSIZE);
    // strcpy(message,readFile(filename, bufferSize));
    readFileAll(filename,message);
    buffer = message.c_str();
    printf("%s\n", buffer);

    int total_frame;
    float x;
    x = getFileSize(filename)/(float)1024;
    total_frame = ceil(x);
    printf("TOTAL FRAME: %d\n\n",total_frame);

    // msg is frame with SOH, Sequence Number, Data Length, Data, and checksum
    for (int i=0; i < total_frame; i++) {
        char* msg=(char*)malloc(MAX_FRAME);
        int seqnum = i;
        msg[0] = 0x1;
        msg[1] = (seqnum >> 24) & 0xFF;
        msg[2] = (seqnum >> 16) & 0xFF;
        msg[3] = (seqnum >> 8) & 0xFF;
        msg[4] = seqnum & 0xFF;

        int msglen = strlen(buffer);
        int data_length;
        if (msglen>1024)
          data_length = 1024;
        else
          data_length = msglen;

        printf("msglen= %d\n", msglen);
        printf("data length = %d\n", data_length);

        msg[5] = (data_length >> 24) & 0xFF;
        msg[6] = (data_length >> 16) & 0xFF;
        msg[7] = (data_length >> 8) & 0xFF;
        msg[8] = data_length & 0xFF;

        for (int i=0; i<data_length; i++){
            msg[9+i] = buffer[seqnum*1024+i];
        }

        for (int i=0; i<data_length; i++) {
            printf("%c", msg[9+i]);
        }

        printf("\nFRAME KE-%d\n",seqnum+1);
        printf("PINDAH FRAME\n\n");

        char checksum = checksumVal(data_length,msg);

        printf("Sending packet to %s port %d\n",ipaddr, port);

        // Sliding Window
        int lar = 0;
        int lfs = lar + windowSize;
        int j = 0;
        int buff_i = 0;
        int send_frame = 0;
        while(buff_i < bufferSize){
          if (send_frame < lfs){
            //send the frame
            if (sendto(sockfd, msg, MAX_FRAME, 0, (struct sockaddr *)&serv_addr, slen) == -1){
              std::cerr << "Error sending packet" << '\n';
              exit(1);
            }
            send_frame++;
            buff_i++;
          }
          char* ack = (char*)malloc(6);
          recvlen = recvfrom(sockfd, ack, 6, 0, (struct sockaddr *)&serv_addr, &slen);
          if (ack[0] == 0x06) {
            ack[recvlen] = 0;	/* expect a printable string - terminate it */
            cout << "disini ack" << endl;
            printf("received message: \"%x\"\n", ack);
            lar++;
          }else{
            //nerima nak
            if (sendto(sockfd, msg, MAX_FRAME, 0, (struct sockaddr *)&serv_addr, slen) == -1){
              std::cerr << "Error sending packet" << '\n';
              exit(1);
              }
          }
        }
        // if (sendto(sockfd, msg, MAX_FRAME, 0, (struct sockaddr *)&serv_addr, slen) == -1){
        //   cerr << "error sending packet";
        //   exit(1);
        // }
        //
        // char* ack = (char*)malloc(6);
        // recvlen = recvfrom(sockfd, ack, 6, 0, (struct sockaddr *)&serv_addr, &slen);
        //   if (recvlen >= 0) {
        //     ack[recvlen] = 0;	/* expect a printable string - terminate it */
        //     printf("received message: \"%s\"\n", ack);
        // }
    }
    close(sockfd);
    return 0;
}
