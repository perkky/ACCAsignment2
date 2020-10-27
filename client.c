#include "ACCSockets.h"
#include <stdio.h>
#include <stdlib.h>
#include "clientApi.h"
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("Usage: ./client <IPaddress/hostname> <port>\n");
		return 1;
	}

    char ip[BUFFER_SIZE];
    if (getIpAddr(argv[1], ip) != 0)
    {
        printf("Error: Unable to resolve hostname\n");
        return 1;
    }

    int sockfd = connectToSocket(ip, atoi(argv[2]));


    int quit = sockfd > 0 ? 0 : 1;

    if (!quit)
    {
        char message[2*BUFFER_SIZE];
        recieveMessage(message, sockfd);
        printf("%s\n", message);
    }
    
    int haveJoined = 0;
    while (!quit)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockfd, &set);
        FD_SET(0, &set);
        
        printf("USER> ");
        fflush(stdout);

        if (select(sockfd+1, &set,  NULL, NULL, NULL) == -1)
        {
            return -1;
        }

        //Stdin input recieved
        if (FD_ISSET(0, &set))
        {
            char message[2*BUFFER_SIZE];
            fgets(message, 256, stdin);
            
            char remaining[BUFFER_SIZE], command[BUFFER_SIZE];
            getInput(message, command, remaining);


            if (strncmp(command, "join", 4) == 0)
                haveJoined += joinClient(remaining, sockfd);
            else if (haveJoined == 0)
                printf("Error: you must join first\n");
            else if (strncmp(command, "nick", 4) == 0)
                nickClient(remaining, sockfd);
            else if (strncmp(command, "whois", 5) == 0)
                whoisClient(remaining, sockfd);
            else if (strncmp(command, "who", 3) == 0)
                whoClient(sockfd);
            else if (strncmp(command, "time", 4) == 0)
                timeClient(sockfd);
            else if (strncmp(command, "privmsg", 7) == 0)
                privmsgClient(remaining, sockfd);
            else if (strncmp(command, "bcastmsg", 8) == 0)
                bcastClient(remaining, sockfd);
            else if (strncmp(command, "quit", 4) == 0)
            {
                quitClient(remaining, sockfd);
                quit = 1;
            }
            else
                printf("Error: Incorrect command\n");

        }

        //Message from socket recieved
        if (FD_ISSET(sockfd, &set))
        {
            if (isSocketClosed(sockfd))
            {
                printf("Connection to server lost.\n");
                break;
            }

            char message[2*BUFFER_SIZE];
            recieveMessage(message, sockfd);

            printf("\n%s\n", message);
        }
    } 

    return 0;
}
