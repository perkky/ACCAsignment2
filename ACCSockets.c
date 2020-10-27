#include <time.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "ACCSockets.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

struct timeval TIMEOUT;

int getFileSize(char* fileName)
{
    int size = 0;
    FILE* fp = fopen(fileName, "r");
    fseek(fp, 0L, SEEK_END);

    size = (int)ftell(fp);

    return size;
}

/* Converts a string to all lowercase*/
void toLower(char* str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        str[i] = tolower(str[i]);
    }
}


/* Sends a message to socket with sockfd
 * This sends the whole message, only stopping at a new line character or 0.
 * Once the message has been sent, 0 is sent to indicate it is finished.
 * Returns SUCCESS if successful and ERROR if an error occurs*/
int sendMessage(char* message, int sockfd)
{
    int bytesSent;
    int msgLen = strlen(message);
    char charToSend;
    int i = 0;

    while (msgLen > i)
    {
        charToSend = *(message + i++);
        if (charToSend == 0 || charToSend == '\n')
            break;
        else if ((bytesSent = send(sockfd, &charToSend, 1,0)) <= 0)
            return ERROR;
    }
    
    //Send 0 to indicate the message is complete
    char null = 0;
    send(sockfd, &null, 1,0);


    return SUCCESS;
}

/* Recieves a message from the socket socfd.
 * This continues to read characters from the socket until it reaches a 0,
 * as this indicates the end of the message has been reached.
 * select() is used to timout this if no respone is given in SOCKET_WAIT_TIME.
 * Returns SUCCESS if successful and TIME_OUT if it timesout. */
int recieveMessage(char* dest, int sockfd)
{
    char charRecieved;
    int bytesRecieved;
    int selectReturn;
    int i = 0;

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    TIMEOUT.tv_sec = 2;

    while( (selectReturn = select(sockfd+1, &set, NULL, NULL, &TIMEOUT)) == 1 && (bytesRecieved = recv(sockfd, &charRecieved, 1, 0)) > 0)
    {
        if (charRecieved == 0 || charRecieved == '\n')
        {
            *(dest + i++) = 0;
            break;

        }
        else
            *(dest + i++) = charRecieved;

    }

    //Return 1 if there was an error
    return selectReturn == 1 ? SUCCESS : TIME_OUT;
}

/* Functions the same as sendMessage(), however it sends a file instead. */
int sendFile(char* fileName, int sockfd)
{
    int fileSize = getFileSize(fileName);
    //send the file size
    char fileSizeStr[BUFFER_SIZE];
    sprintf(fileSizeStr, "%d", fileSize);
    sendMessage(fileSizeStr, sockfd);
    
    FILE* f = fopen(fileName, "r");
    int bytesRead;

    char charToSend;

    while (( bytesRead = fread(&charToSend, sizeof(char), 1, f)) > 0 )
    {
        if (send(sockfd, &charToSend, 1, 0) < 0 )
                return -1;
    }


    return fclose(f);
}

/* Functions the same as recieveMessage(), however it recieves a file instead. */
int recieveFile(char* fileName, int sockfd)
{
    FILE* f = fopen(fileName, "w");

    //recieve file size
    char fileSizeStr[BUFFER_SIZE];
    recieveMessage(fileSizeStr, sockfd);
    int fileSize = atoi(fileSizeStr);

    char charRecieved;
    int bytesRead;

    while(fileSize-- > 0 && (bytesRead = recv(sockfd, &charRecieved, 1, 0)) > 0)
    {
        fwrite(&charRecieved, sizeof(char), 1, f);
    }

    return fclose(f);
}

/* Listens on a port. Returns the sockfd of the listen socket*/
int createListenSocket(int port)
{
	int			listenfd;
	struct sockaddr_in	servaddr;

	if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		fprintf(stderr, "socket creation failed\n");
        return -1;
	}
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        fprintf(stderr, "setsockopt failed\n");
        return -1;
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port); /* daytime server */

	if ( (bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0) {
		fprintf(stderr, "bind failed\n");
        return -1;
	}
		
	if ( listen(listenfd, 1024) < 0) {
		fprintf(stderr, "listen failed\n");
        return -1;
	}

    return listenfd;
}

int listenForCon(int listenfd, char* returnIp)
{
    int connfd;
	struct sockaddr_in cliaddr;
	socklen_t		len;
	char			buff[BUFFER_SIZE];
	const char	*ptr;

    len = sizeof(cliaddr);

    if ( (connfd = accept(listenfd, (SA *) &cliaddr, &len)) < 0 ) {
        printf("%d\n", errno);
        fprintf(stderr, "accept failed\n");
        return -1;
    }
    if( (ptr = inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff))) == NULL) {
        fprintf(stderr, "inet_ntop error \n");
        return -1;
    }

    strcpy(returnIp, buff);

    return connfd;
}


/* Connects to a socket */
int connectToSocket(char* ip_addr, int port)
{
	int	sockfd;
	struct sockaddr_in	servaddr;


	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Socket error\n");
		return -1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);	/* daytime server */
	if (inet_pton(AF_INET, ip_addr, &servaddr.sin_addr) <= 0){
		printf("Inet_pton error for %s\n", ip_addr);
		return -1;
	}

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
		printf("Connection error\n");
		return -1; 
	}

    return sockfd;
}

/* Checks if the input is a valid IPv4 address 
 * Returns 0 if it is and 1 if it isnt.*/
int isValidIpAddress(char* ipAddr)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddr, &(sa.sin_addr));
    return result != 0;

}

void getHostName(char* ipAddr, char* hostname)
{
    struct in_addr addr;
    inet_aton(ipAddr,&addr);
    struct hostent* host = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
    strcpy(hostname, host->h_name);
}

/* Checks to see if the input string is a valid ip addres or a valid hostname.
 * Exports the output ip in outIp.
 * Returns 0 if it is able to get an ip and 1 if it isnt. */
int getIpAddr(char* inIpOrHost, char* outIp)
{
    if (isValidIpAddress(inIpOrHost))
    {
        strcpy(outIp, inIpOrHost);
    }
    else
    {
        struct hostent* host = gethostbyname(inIpOrHost);
        struct in_addr a;
        if (host != NULL)
        {
            bcopy(*host->h_addr_list++, (char*)&a, sizeof(a));
            strcpy(outIp, inet_ntoa(a));
        }
        else
            return 1;

    }

    return 0;
}

int isSocketClosed(int sockfd)
{
    char x;
    return recv(sockfd, &x, 1, MSG_DONTWAIT|MSG_PEEK) == 0;
}
