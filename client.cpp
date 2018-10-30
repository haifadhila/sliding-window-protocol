#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
using namespace std;

#define BUFFSIZE 1024
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int readFile(string inputfile, string message){
    string line;
    ifstream filename;
    filename.open("test.txt");
    if (filename.is_open()){
        while(getline(filename, line)){
            message = message + line;
        }
    }else{
        cout << "Can't open file" << endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6){
        cout << "wrong input" << endl;
        cout << "./sendfile filename windowsize buffersize IPaddress port" << endl;
        exit(0);
    }else{
        filename = argv[1]
        windowsize = atoi(argv[2]);
        bufferSize = atoi(argv[3]);
        ipaddr = argv[4]
        port = atoi(argv[5]);
    }


    int sockfd, n;
    struct sockaddr_in serv_addr, cli_addr;
    char* server;

    char buffer[1024];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = ipaddr;
    // if (server == NULL) {
    //     cerr << "ERROR, no such host\n";
    //     exit(0);
    // }
    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    serv_addr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0) {
        perror("bind failed");
        return 0;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server);
    if (inet_aton(server, &serv_addr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
    for (i=0;i < 5;i++){
        printf("Sending packet to %s port %d\n",server, port);

    }
    std::string buff(buffer);
    readFile(filename,buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}