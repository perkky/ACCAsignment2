#include <stdlib.h>
#include <string.h>
#include "ACCSockets.h"
#include <time.h>
#include "serverApi.h"

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
}

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

void bcastMsgUsers(Users* users, char* username, char* message)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) !=0)
            sendMessage(message, users->sockfd);

        users = users->next;
    }

}

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

void sendMsgUsers(Users* users, char* username, char* message)
{
    int sockfd = getSockfdUsers(users, username);

    sendMessage(message, sockfd);
}

void getNick(Users* users, char* username, char* nick)
{
    while (users != NULL)
    {
        if (strcmp(username, users->username) == 0)
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
