#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "ACCSockets.h"
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include "serverApi.h"
#include "global.h"
#include <netdb.h>
#include <arpa/inet.h>
 
void* Server_thread(void* listenfd);
int SOCKET_WAIT_TIME;
Users* g_users = NULL;
int numServersAvailable;
int numThreads;
pthread_cond_t thread_cond;
pthread_mutex_t thread_mutex;
pthread_mutex_t user_mutex;
pthread_t* thread_handles;

//The Server main function
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Usage: ./server n m wait_time\n");
        return 1;
    }
    signal(SIGINT, cleanUp);
    signal(SIGTERM, cleanUp);
    numThreads = atoi(argv[1]);
    int numNewThreads = atoi(argv[2]);
    SOCKET_WAIT_TIME = atoi(argv[3]);
    numServersAvailable = numThreads;

    if (SOCKET_WAIT_TIME < 1 || SOCKET_WAIT_TIME > 120 || numThreads < 1 || numNewThreads < 1)
    {
        printf("Error: n >1, m >1, 1 < wait_time < 120\n");
        return 1;
    }

	thread_handles = malloc(numThreads*sizeof(pthread_t));

    if (pthread_mutex_init(&thread_mutex, NULL) != 0)
    {
        printf("Error: Mutex not initialised");
        return 1;
    }
    if (pthread_mutex_init(&user_mutex, NULL) != 0)
    {
        printf("Error: Mutex not initialised");
        return 1;
    }

    pthread_cond_init(&thread_cond, NULL);

    int listenfd = createListenSocket(50070);

	for(int i = 0; i < numThreads; i++) {
        pthread_create(thread_handles+i, NULL, Server_thread, &listenfd);
	}

    pthread_mutex_lock(&thread_mutex);
    while (1)
    {
        pthread_cond_wait(&thread_cond, &thread_mutex);
 
        numThreads += numNewThreads;
        numServersAvailable += numNewThreads;
	    thread_handles = realloc(thread_handles, numThreads*sizeof(pthread_t));
        for(int i = 0; i < numNewThreads; i++) {
            pthread_create(thread_handles+numThreads-numServersAvailable+i+1, NULL, Server_thread, &listenfd);
    }

    pthread_mutex_unlock(&thread_mutex);

    }
		
    return 0;
}

void* Server_thread(void* listenfd)
{
    char returnIp[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    char hostname[BUFFER_SIZE];
    int sockfd = 0;

    sockfd = listenForCon(*(int*)listenfd, returnIp);


    int exit = 0;

    getHostName(returnIp, hostname);

    pthread_mutex_lock(&thread_mutex);
    numServersAvailable--;
    if (numServersAvailable == 1)
        pthread_cond_signal(&thread_cond);
    pthread_mutex_unlock(&thread_mutex);

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    struct timeval TIMEOUT;
    TIMEOUT.tv_sec = SOCKET_WAIT_TIME;
    sendMessage("Connected to server!", sockfd);

    Users* currrentUser = NULL;
    while (!exit)
    {
        if (select(sockfd+1, &set, NULL, NULL, &TIMEOUT) <= 0)
        { 
            exit = 1;
            //timeout?
            sendMessage("You have been timed out as you have not sent a command.", sockfd);

            char message[2*BUFFER_SIZE];
            sprintf(message, "User %s has been timed out", username);
            bcastMsgUsers(g_users, username, message);
            removeUserName(&g_users, username);
        }
        else
        {
            TIMEOUT.tv_sec = SOCKET_WAIT_TIME;
            if (isSocketClosed(sockfd))
                break;
            //get commnad from user
            //timeout if needed

            if (currrentUser != NULL)
                pthread_mutex_lock(&currrentUser->mutex);

            switch (getCommandFromClient(sockfd))
            {
                case JOIN:
                    joinServer(sockfd, username, hostname, &g_users);
                    currrentUser = getUserNick(g_users, username);
                    break;
                case NICK:
                    nickServer(sockfd, username, g_users);
                    break;
                case WHO:
                    whoServer(sockfd, g_users);

                    break;
                case WHOIS:
                    whoisServer(sockfd, g_users);

                    break;
                case TIME:
                    timeServer(sockfd);

                    break;
                case PRIVMSG:
                    privmsgServer(sockfd, username, g_users);

                    break;
                case BCAST:
                    bcastServer(sockfd, username, g_users);

                    break;
                case QUIT:
                    quitServer(sockfd, username, &g_users);

                    exit = 1;
                    break;
                default:
                    break;

            }
            if (currrentUser != NULL)
                pthread_mutex_unlock(&currrentUser->mutex);
        }
        

    }

    close(sockfd);

    return NULL;
}
