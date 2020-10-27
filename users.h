#pragma once
#define BUFFER_SIZE 256
#include <pthread.h>

typedef struct Users
{
    int sockfd;
    char username[10];
    char name[20];
    char nick[9];
    char hostname[BUFFER_SIZE];
    int joinTime;
    struct Users* next;
    pthread_mutex_t mutex;
} Users;

int usernameExists(Users* user, char* username);
int nickExists(Users* user, char* nick);
int socketExists(Users* user, int sockfd);
void addUser(Users** users, char* username, char* name, char* hostname, int sockfd);
void removeUserName(Users** users, char* username);
void removeUserSock(Users** users, int sockfd);
void getNick(Users* users, char* username, char* nick);
void setNick(Users* users, char* username, char* nick);
Users* getUserNick(Users* users, char* nick);

void bcastMsgUsers(Users* users, char* username, char* message);
void sendMsgUsers(Users* users, char* username, char* message);

void freeAllUsers(Users** users);
