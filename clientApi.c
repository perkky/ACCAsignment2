#include <stdlib.h>
#include <string.h>
#include "ACCSockets.h"
#include <stdio.h>

//Splits the string on the first space
//If no space is found, second is set to an empty string
void splitOnFirstSpace(char* str, char* first, char* second)
{
    char* spaceLoc = strchr(str,' ');
    if (spaceLoc != NULL)
    {
        strncpy(first, str, spaceLoc-str);
        strcpy(second, spaceLoc+1);
    }
    else
    {
        strcpy(first, str);
        memset(second, 0, BUFFER_SIZE);
    }
}

int containsSpace(char* str)
{
    return (strchr(str,' ') != NULL);
}

//Gets input from the user
void getInput(char* line, char* command, char* remaining)
{
    splitOnFirstSpace(line, command, remaining);

    toLower(command);
}

int joinClient(char* remaining, int sockfd)
{
    sendMessage("JOIN",sockfd);
    sendMessage(remaining, sockfd);

    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);

    printf("%s\n", message);

    return strncmp("Error", message,5) != 0;
}

void nickClient(char* remaining, int sockfd)
{
    if (containsSpace(remaining))
        printf("Error: Nickname cannot contain a space\n");
    else
    {
        sendMessage("NICK",sockfd);
        sendMessage(remaining, sockfd);

        char message[BUFFER_SIZE];
        recieveMessage(message, sockfd);

        printf("%s\n", message);
    }
}

void privmsgClient(char* remaining, int sockfd)
{
    sendMessage("PRIVMSG",sockfd);
    sendMessage(remaining,sockfd);

    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);

    printf("%s\n",message);
}

void timeClient(int sockfd)
{
    sendMessage("TIME",sockfd);

    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);

    printf("%s\n",message);
}

void whoisClient(char* remaining, int sockfd)
{
    if (strlen(remaining) != 0)
    {
        sendMessage("ISWHO",sockfd);
        sendMessage(remaining, sockfd);

        char message[BUFFER_SIZE];
        recieveMessage(message, sockfd);

        printf("%s\n",message);

    }
    else
    {
        printf("Error: no name given\n");

    }

}

void whoClient(int sockfd)
{
    sendMessage("WHO",sockfd);

    //recieve the number of users
    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);

    int numUsers = atoi(message);

    for (int i = 0; i < numUsers; i++)
    {
        char userInfo[BUFFER_SIZE];
        recieveMessage(userInfo, sockfd);

        printf("%s\n", userInfo);
    }
}

void bcastClient(char* remaining, int sockfd)
{
    if (strlen(remaining) != 0)
    {
        sendMessage("BCAST",sockfd);
        sendMessage(remaining, sockfd);

        char message[BUFFER_SIZE];
        recieveMessage(message, sockfd);

        printf("%s\n",message);

    }
    else
    {
        printf("Error: No text given\n");
    }

}

void quitClient(char* remaining, int sockfd)
{
    sendMessage("QUIT",sockfd);

    if (strlen(remaining) != 0)
    {
        //yes indicates that a message is being sent
        sendMessage("yes",sockfd);
        sendMessage(remaining,sockfd);

    }
    else
    {
        sendMessage("no",sockfd);
    }

    
    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);
    printf("%s\n",message);
}
