#include <stdlib.h>
#include <string.h>
#include "ACCSockets.h"
#include <time.h>
#include "serverApi.h"

//Returns true if the userrname exists, otherwise false
int usernameExists(Users* user, char* username)
{
    while (user != NULL)
    {
        if (strcmp(user->username, username) == 0)
            return 1;

        user = user->next;
    }

    return 0;
}

//Returns true if the nickname exists, otherwise false
int nickExists(Users* user, char* nick)
{
    while (user != NULL)
    {
        if (strcmp(user->nick, nick) == 0)
            return 1;

        user = user->next;
    }

    return 0;
}

//Returns true if the socket number exists, otherwise false
int socketExists(Users* user, int sockfd)
{
    while (user != NULL)
    {
        if (sockfd == user->sockfd)
            return 1;

        user = user->next;
    }

    return 0;
}

//Adds a user to the Users struct
void addUser(Users** users, char* username, char* name, char* hostname, int sockfd)
{
    if (*users == NULL)
    {
        *users = (Users*)malloc(sizeof(Users));
        strcpy((*users)->name, name);
        strcpy((*users)->username, username);
        strcpy((*users)->nick, username);
        strcpy((*users)->hostname, hostname);
        (*users)->joinTime = (int)getTime();
        (*users)->sockfd = sockfd;
        (*users)->next = NULL;
        pthread_mutex_init(&(*users)->mutex, NULL);

        return;
    }

    Users* user = *users;

    while (user->next != NULL)
        user = user->next;

    user->next = (Users*)malloc(sizeof(Users));
    user = user->next;

    strcpy(user->name, name);
    strcpy(user->username, username);
    strcpy(user->nick, username);
    strcpy(user->hostname, hostname);
    user->joinTime = (int)getTime();
    user->sockfd = sockfd;
    user->next = NULL;
    pthread_mutex_init(&user->mutex, NULL);
}

//Removes a user to the Users struct based on their username
void removeUserName(Users** users, char* username)
{
    Users* prev = NULL;
    Users* user = *users;
    while (user != NULL)
    {
        if (strcmp(username, user->username) == 0 || strcmp(username, user->nick) == 0)
        {
            if (prev == NULL)
            {
                *users = user->next;
            }
            else
            {
                prev->next = user->next;
            }

            free(user);

        }

        prev = user;
        user = user->next;
    }
}

//Removes a user to the Users struct based on their socket id
void removeUserSock(Users** users, int sockfd)
{
    Users* prev = NULL;
    Users* user = *users;
    while (user != NULL)
    {
        if (user->sockfd == sockfd)
        {
            if (prev == NULL)
            {
                *users = user->next;
            }
            else
            {
                prev->next = user->next;
            }

        }

        prev = user;
        user = user->next;
    }
}

//Broadcast a message to all other users
void bcastMsgUsers(Users* users, char* username, char* message)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) !=0)
        {
            pthread_mutex_unlock(&users->mutex);
            sendMessage(message, users->sockfd);
            pthread_mutex_unlock(&users->mutex);
        }

        users = users->next;
    }

}

//get a sockid of a user with a specific username
int getSockfdUsers(Users* users, char* username)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) == 0 || strcmp(username, users->nick) == 0)
            return users->sockfd;

        users = users->next;
    }

    return -1;
}

//send message to a s pecific user with a specific username or nickname
void sendMsgUsers(Users* users, char* username, char* message)
{
    int sockfd = getSockfdUsers(users, username);
    char nick[BUFFER_SIZE];
    getNick(users, username, nick);
    Users* user = getUserNick(users, nick);

    pthread_mutex_lock(&user->mutex);
    sendMessage(message, sockfd);
    pthread_mutex_unlock(&user->mutex);
}

//returns the nickname of a user with username
void getNick(Users* users, char* username, char* nick)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) == 0 || strcmp(username, users->nick) == 0)
        {
            strcpy(nick, users->nick);
            return;
        }

        users = users->next;
    }
}


void setNick(Users* users, char* username, char* nick)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) == 0)
        {
            strcpy(users->nick, nick);
            return;
        }

        users = users->next;
    }
}

//Returns the User struct of a user with the nickname.
//Returns NULL if no user is found.
Users* getUserNick(Users* users, char* nick)
{
    while (users != NULL)
    {
        if (strcmp(nick, users->nick) == 0)
        {
            return users;
        }

        users = users->next;
    }

    return NULL;
}

//Frees all users memory
void freeAllUsers(Users** users)
{
    Users* prev = *users;
    Users* next = *users;

    while (next != NULL)
    {
        prev = next;
        next = next->next;

        free(prev);
    }

}
