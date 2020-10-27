#include "global.h"
#include "ACCSockets.h"
#include "serverApi.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "global.h"
#include <malloc.h>
#include <stdlib.h>

//Gets the command from the user and returns a COMMAND Enum
COMMAND getCommandFromClient(int sockfd)
{
    char buffer[BUFFER_SIZE];

    if (recieveMessage(buffer, sockfd) == TIME_OUT)
    {
        return QUIT;
    }

    toLower(buffer);

    if (strncmp("join", buffer, 4) == 0)
        return JOIN;
    else if (strncmp("nick", buffer, 4) == 0)
        return NICK;
    else if (strncmp("who", buffer, 3) == 0)
        return WHO;
    else if (strncmp("iswho", buffer, 5) == 0)
        return WHOIS;
    else if (strncmp("time", buffer, 4) == 0)
        return TIME;
    else if (strncmp("privmsg", buffer, 7) == 0)
        return PRIVMSG;
    else if (strncmp("bcast", buffer, 4) == 0)
        return BCAST;
    else if (strncmp("quit", buffer, 4) == 0)
        return QUIT;
    else
        return INVALID;

}

//Cleans up and frees any memory
void cleanUp(int sig)
{
	for(int i = 0; i < numThreads; i++) {
        pthread_cancel(thread_handles[i]);
        pthread_join(thread_handles[i], NULL);
	}
    free(thread_handles);
    pthread_mutex_destroy(&thread_mutex);
    pthread_mutex_destroy(&user_mutex);
    freeAllUsers(&g_users);
    exit(0);
}

time_t getTime()
{
    return time(NULL);
}

//This function returns the current time as a string
char* getTimeString()
{
    time_t time = getTime();
    struct tm* timeinfo = localtime(&time);

    //get rid of the newline character
    char* time_str = asctime(timeinfo);
    time_str[strlen(time_str)-1] = '\0';

    return time_str;

}

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
    }
}

ReturnVal joinServer(int sockfd, char* inUsername, char* hostname, Users** users)
{
    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);


    char username[BUFFER_SIZE], name[BUFFER_SIZE];
    splitOnFirstSpace(message, username, name);

    if (strlen(username) > 10 || strlen(name) > 20 || strlen(name) == 0 || strlen(username) == 0)
    {
        sendMessage("Error: Username must be less than 10 char and real name must be less than 20 char", sockfd);
    }
    else
    {
        char sendMsg[3*BUFFER_SIZE];
        pthread_mutex_lock(&user_mutex);
        if ( nickExists(*users,username) || usernameExists(*users, username))
        {
            sprintf(sendMsg, "Error: Username/nickname already exists");
        }
        else if (socketExists(*users, sockfd))
        {
            sprintf(sendMsg, "Error: You have already joined");
        }
        else
        {
            strcpy(inUsername, username);
            sprintf(sendMsg,"SERVER: JOIN %s - %s - %s", username, name, hostname);
            addUser(users, username, name, hostname, sockfd);
        }
        pthread_mutex_unlock(&user_mutex);

        sendMessage(sendMsg, sockfd);
    }



    return SUCCESS;
}

ReturnVal nickServer(int sockfd, char* inUsername, Users* user)
{
    char nick[BUFFER_SIZE];
    char message[3*BUFFER_SIZE];
    recieveMessage(nick, sockfd);

    pthread_mutex_lock(&user_mutex);
    if (strlen(nick) > 9)
    {
        sprintf(message, "SERVERR: Error: Nickname must be less than 9 characters");
    }
    else if (!nickExists(user, nick))
    {
        char oldNick[BUFFER_SIZE];
        getNick(user, inUsername, oldNick);
        setNick(user, inUsername, nick);

        if (strlen(oldNick) == 0)
            sprintf(message, "SERVER: Your new nickname is %s", nick);
        else
            sprintf(message, "SERVER: You have changed your nickname from %s to %s", oldNick, nick);
    }
    else
    {
        sprintf(message, "SERVER: Error: %s is already taken", nick);
    }
    pthread_mutex_unlock(&user_mutex);

    sendMessage(message, sockfd);

    return SUCCESS;
}

ReturnVal privmsgServer(int sockfd, char* inUsername, Users* user)
{
    char message[2*BUFFER_SIZE];
    recieveMessage(message, sockfd);

    char target[20], text[BUFFER_SIZE];
    memset(target, '\0', 20);
    memset(text, '\0', BUFFER_SIZE);
    splitOnFirstSpace(message, target, text);

    char returnMessage[BUFFER_SIZE];

    pthread_mutex_lock(&user_mutex);
    if( usernameExists(user, target) || nickExists(user, target))
    {
        char inNick[20];
        getNick(user,inUsername, inNick);
        if (strlen(inNick) == 0)
            strcpy(inNick, inUsername);

        char userMessage[2*BUFFER_SIZE];
        sprintf(userMessage, "PRIVMSG %s: %s", inNick, text);
        sendMsgUsers(user, target, userMessage);

        sprintf(returnMessage,"SERVER: Sent message to %s", target);
    }
    else
    {
        sprintf(returnMessage,"SERVER: Error: %s does not exist", target);
    }
    pthread_mutex_unlock(&user_mutex);

    sendMessage(returnMessage, sockfd);

    return SUCCESS;
}

ReturnVal timeServer(int sockfd)
{

    sendMessage(getTimeString(), sockfd);

    return SUCCESS;
}

ReturnVal whoisServer(int sockfd, Users* user)
{
    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);

    pthread_mutex_lock(&user_mutex);
    Users* myUser = getUserNick(user, message);

    if (myUser == NULL)
    {
        sendMessage("Error: User does not exist",sockfd);
    }
    else
    {
        char userInfo[2*BUFFER_SIZE];
        sprintf(userInfo, "SERVER: %s, %s", myUser->name, myUser->hostname);
        sendMessage(userInfo, sockfd);
    }
    pthread_mutex_unlock(&user_mutex);

    return SUCCESS;
}

ReturnVal bcastServer(int sockfd, char* inUsername, Users* user)
{
    char message[BUFFER_SIZE];
    recieveMessage(message, sockfd);


    pthread_mutex_lock(&user_mutex);
    char inNick[20];
    getNick(user,inUsername, inNick);
    char text[2*BUFFER_SIZE];
    sprintf(text,"BCASTMSG %s: %s", inNick, message); 
    bcastMsgUsers(user, inUsername, text);
    pthread_mutex_unlock(&user_mutex);

    sendMessage("Message broadcasted.", sockfd);

    return SUCCESS;
}

ReturnVal whoServer(int sockfd, Users* user)
{
    pthread_mutex_lock(&user_mutex);
    Users* temp = user;

    int numUsers =1;
    while (temp->next != NULL)
    {
        temp = temp->next;
        numUsers++;
    }

    //send the number of users
    char numUsersStr[BUFFER_SIZE];
    sprintf(numUsersStr, "%d", numUsers);
    sendMessage(numUsersStr, sockfd);
    
    temp = user;
    for (int i = 0; i < numUsers; i++)
    {
        char userInfo[2*BUFFER_SIZE];
        sprintf(userInfo, "SERVER: %s, %s, %s, %s, %d seconds", temp->nick, temp->username,temp->name, temp->hostname, (int)getTime() - temp->joinTime);
        sendMessage(userInfo, sockfd);
        temp = temp->next;
    }
    pthread_mutex_unlock(&user_mutex);

    return SUCCESS;
}

ReturnVal quitServer(int sockfd, char* inUsername, Users** users)
{
    char message[BUFFER_SIZE];
    memset(message, '\0', BUFFER_SIZE);
    recieveMessage(message, sockfd);

    char nick[10];
    pthread_mutex_lock(&user_mutex);
    getNick(*users, inUsername, nick);
    pthread_mutex_unlock(&user_mutex);

    if (strcmp("yes", message) == 0)
    {
        char userMessage[BUFFER_SIZE];
        memset(userMessage, '\0', BUFFER_SIZE);
        recieveMessage(userMessage, sockfd);

        char text[2*BUFFER_SIZE];
        sprintf(text,"SERVER: %s is no longer in our chatting session. %s's last message: %s", nick, nick, userMessage); 

        pthread_mutex_lock(&user_mutex);
        bcastMsgUsers(*users, inUsername, text);
        pthread_mutex_unlock(&user_mutex);

    }
    else
    {
        char text[2*BUFFER_SIZE];
        sprintf(text,"SERVER: %s is no longer in our chatting session. There is no last message from %s!", nick, nick); 

        pthread_mutex_lock(&user_mutex);
        bcastMsgUsers(*users, inUsername, text);
        pthread_mutex_unlock(&user_mutex);

    }

    char timeMessage[BUFFER_SIZE];
    sprintf(timeMessage, "SERVER: You have been chatting for %d seconds. Bye %s!",(int)getTime()-getUserNick(*users,nick)->joinTime ,nick);

    sendMessage(timeMessage, sockfd);

    pthread_mutex_lock(&user_mutex);
    removeUserName(users, inUsername);
    pthread_mutex_unlock(&user_mutex);

    return SUCCESS;
}
