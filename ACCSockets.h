#pragma once
#define	SA struct sockaddr
#define BUFFER_SIZE 256
#include <netinet/in.h>

extern int SOCKET_WAIT_TIME;

typedef enum ReturnVal { SUCCESS, DISCONNECTED, INVALID_PARAMETER, ERROR, TIME_OUT} ReturnVal;

int isSocketClosed(int sockfd);

int createListenSocket(int port);

int listenForCon(int listenfd, char* returnIp);

int sendFile(char* fileName, int sockfd);

int recieveFile(char* fileName, int sockfd);

int connectToSocket(char* ip_addr, int port);

int recieveMessage(char* dest, int sockfd);

int sendMessage(char* message, int sockfd);

int getIpAddr(char* inIpOrHost, char* outIp);

void toLower(char* str);

void getHostName(char* ipAddr, char* hostname);
