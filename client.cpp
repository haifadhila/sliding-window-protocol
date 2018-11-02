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
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;


#define MAX_FRAME 1034
#define MAX_DATA 1024
#define ACKSIZE 6
#define TIMEOUT 5
#define time_now chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define time_diff(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()

int windowSize, bufferSize, port;
FILE *f;
time_stamp now = time_now();
int sockfd, n;
struct sockaddr_in serv_addr;  //server address
struct sockaddr_in cli_addr; // my address
int lar = -1;
int lfs = lar + windowSize;
bool *window_ack_recv;
bool *window_send_data_check;
time_stamp *window_send_data_time;
mutex window_info_mutex;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//checksum buat ack
char checksumAck(char *ack, int acklen) {
    u_long sum = 0;
    for (int i=0; i < acklen;i++) {
        sum+= ack[i];
    }
    sum = ~sum;
    return sum;
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


void acklistener() {
    bool listening = true;
    char ack[ACKSIZE];

    while (listening) {
        socklen_t serv_addr_size;
        int ack_size = recvfrom(sockfd, (char *)ack, ACKSIZE, MSG_WAITALL, (struct sockaddr *) &serv_addr, &serv_addr_size);

        bool ack_negative;
        if (ack[0] == 0x15) 
            ack_negative=true;
        else ack_negative=false;

        uint32_t temp_sequence_number;
        memcpy(&temp_sequence_number, ack+1, 4);
        int ack_sequence_number = ntohl(temp_sequence_number);

        bool ack_error = (ack[5] != checksumAck(ack, ACKSIZE - (int) 1));
        
        window_info_mutex.lock();
        bool inside_window = !ack_error && ack_sequence_number > lar && ack_sequence_number <=lfs;
        if (inside_window) {
            if (!ack_negative) {
                //not error
                cout << "ack sent";
                window_ack_recv[ack_sequence_number- (lar+1)] = true;
            } else {
                //set time current for sending
                cout << "ack not sent";
                window_send_data_time[ack_sequence_number-(lar+1)] = now;
            }
        }

        window_info_mutex.unlock();
    }
}
// Reading File eksternal per buffer
bool readFile(FILE *f, int *real_buff_size, char* buffer) {
    //return read done or not
    char charFile;
    ifstream filename;
    int max_read_file , i;

    // printf("file opened\n");
    *real_buff_size = fread(buffer, 1, bufferSize*MAX_DATA, f);
    if (*real_buff_size < bufferSize*MAX_DATA) {
        return true;
    } else if (*real_buff_size == bufferSize*MAX_DATA) {
        char temp_buffer[bufferSize];
        int temp_buff_size = fread(buffer, 1, bufferSize, f);
        if (temp_buff_size==0) {
            return true;
        }
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

int intoFrame(char msg[], int seqnum, const char* buffer, int data_length){
  // msg is frame with SOH, Sequence Number, Data Length, Data, and checksum
      msg[0] = 0x1;
      msg[1] = (seqnum >> 24) & 0xFF;
      msg[2] = (seqnum >> 16) & 0xFF;
      msg[3] = (seqnum >> 8) & 0xFF;
      msg[4] = seqnum & 0xFF;

      printf("data length = %d\n", data_length);

      msg[5] = (data_length >> 24) & 0xFF;
      msg[6] = (int(data_length) >> 16) & 0xFF;
      msg[7] = (int(data_length) >> 8) & 0xFF;
      msg[8] = int(data_length) & 0xFF;

      cout << "DL:" << ((msg[5] << 24) | (msg[6] << 16) | (msg[7] << 8) | (msg[8]));

      for (int i=0; i<data_length; i++){    
          msg[9+i] = buffer[seqnum*1024+i];
      }

      for (int i=0; i<data_length; i++) {
          printf("%c", msg[9+i]);
      }

      msg[data_length+9] = checksumVal(data_length, msg);

      return data_length;
}

int main(int argc, char *argv[]) {
    char* filename;
    char* ipaddr;
    int recvlen;

    cout << "Hello! BEGIN!" << endl;
    if (argc != 6){
        cout << "wrong input" << endl;
        cout << "./sendfile filename windowsize buffersize IPaddress port" << endl;
        exit(0);
    } else {
        filename = argv[1];
        windowSize = atoi(argv[2]);
        bufferSize = atoi(argv[3]);
        ipaddr = argv[4];
        port = atoi(argv[5]);
    }


    // Create Socket
    cout << "Open socket: ";
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1)
        error("ERROR opening socket");
    else
        cout << "socket opened" << endl;

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

    f= fopen(filename, "rb");

    bool all_file_read = false;
    int real_buff_size;
    char buffer[bufferSize*MAX_DATA];
    char msg_frame[MAX_FRAME];


    cout << "creating thread. . . " << endl;
    thread ack_thread(acklistener);
    cout << "start reading file: " << endl;
    int count = 0;
    // while (count<1) {
    while (!all_file_read) {
      count++;
        all_file_read = readFile(f, &real_buff_size, buffer);
        if (all_file_read) {
            cout << "all file read on going" << endl;
        } else {
            cout << "all file read stopped" << endl;
        }
      
        window_info_mutex.lock();
        // Sliding Window
        window_send_data_time = new time_stamp[windowSize];
        window_ack_recv = new bool[windowSize];
        window_send_data_check = new bool[windowSize];
        int sequence_number;
        int add_frame;
        if (real_buff_size % MAX_DATA == 0) {
            add_frame=0;
        } else {
            add_frame=1;
        }
        int frame_number= real_buff_size/MAX_DATA + add_frame;

        //initialize: all frame in window haven't been sent
        cout << "initializing sliding window protocol. . . " << endl;
        for (int i=0; i<windowSize; i++) {
            window_ack_recv[i] = false;
            window_send_data_check[i] = false;
        }


        window_info_mutex.unlock();

        bool all_frame_send = false;
        int countlagi=0;
        while (!all_frame_send) {
        // while (!all_frame_send) {
          countlagi++;

            window_info_mutex.lock();
            int move=0;
            if (window_ack_recv[0]) {
                //jika ack frame 0 sudah received, maka cek bisa digeser sebesar apa
                move+=1;
                for (int i=1; i<windowSize; i++) {
                    if (!window_ack_recv[i]) {
                        break;
                    }
                    move+=1;
                }

                //update window data
                for (int i=0; i<windowSize-move; i++) {
                    window_ack_recv[i] = window_ack_recv[i+move];
                    window_send_data_check[i] = window_send_data_check[i+move];
                    window_send_data_time[i] = window_send_data_time[i+move];
                }

                //update window with new frame
                for (int i=windowSize; i>windowSize-move; i--) {
                    window_ack_recv[i] = false;
                    window_send_data_check[i]=false;
                }

                lar = lar+move;
                lfs = lar+ windowSize;
            }
            window_info_mutex.unlock();
            cout << "shift handled" << endl;

            //send frame
            for (int i=0; i<windowSize; i++) {
                cout << "start sending frame. . ." << endl;
                sequence_number = lar + 1 + i;
                cout << "sequence_number = " << sequence_number << endl;
                cout << "frame_number = " << frame_number << endl;
                if (sequence_number <= frame_number) {
                    window_info_mutex.lock();
                    cout << "sequence_number < frame_number" << endl;
                    bool havent_send = !window_send_data_check[i];
                    
                    bool timeout_without_ack = !window_ack_recv[i] && (time_diff(time_now(), window_send_data_time[i]) > TIMEOUT);
                    
                    if (havent_send || timeout_without_ack) {
                        cout << "haven't send data or timeout without receiving ack" << endl;

                        int buf_remain = real_buff_size-sequence_number*1024;
                        int data_length;

                        if (buf_remain<1024)
                            data_length = buf_remain;
                        else
                            data_length = 1024;

                        cout << "creating frame. . . " << endl;
                        cout << "data_length: " << data_length;
                        intoFrame(msg_frame,sequence_number,buffer,data_length);
                        // cout << frame_size;
                        cout << "frame created." << endl;

                        // memcpy(data, buffer + buf_remain, data_size);

                        // frame_size = create_frame(seq_num, frame, data, data_size, eot);

                        sendto(sockfd, msg_frame, 1034, 0, (struct sockaddr *)&serv_addr, slen);
                        window_send_data_check[i]= true;
                        window_send_data_time[i]= time_now();
                        cout << "frame sent" << endl;
                    } else {
                        break;
                    }
                    window_info_mutex.unlock();
                } else { 
                    break;
                }
            }

            if (lar >= sequence_number -1) {
                all_frame_send= true;
            }
        }
        if (all_file_read) {
            break;
        }
    }   // while all_file_read close

    delete [] window_ack_recv;
    delete [] window_send_data_time;

    fclose(f);
    ack_thread.detach();
    close(sockfd);
    return 0;
}
