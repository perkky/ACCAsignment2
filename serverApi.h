#include "ACCSockets.h"
#include "global.h"
#include "users.h"

typedef enum COMMAND {JOIN, NICK, WHO, WHOIS, TIME, PRIVMSG, BCAST, QUIT, INVALID} COMMAND;

time_t getTime();

COMMAND getCommandFromClient(int sockfd);

ReturnVal joinServer(int sockfd, char* inUsername, char* hostname, Users** user);

ReturnVal nickServer(int sockfd, char* inUsername, Users* user);

ReturnVal privmsgServer(int sockfd, char* inUsername, Users* user);

ReturnVal timeServer(int sockfd);

ReturnVal whoisServer(int sockfd, Users* user);

ReturnVal whoServer(int sockfd, Users* user);

ReturnVal bcastServer(int sockfd, char* inUsername, Users* user);

ReturnVal quitServer(int sockfd, char* inUsername, Users** users);

char* getTimeString();

void cleanUp(int sig);
