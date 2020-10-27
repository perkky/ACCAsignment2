#pragma once

void getInput(char* line, char* command, char* remaining);

int joinClient(char* remaining, int sockfd);
void nickClient(char* remaining, int sockfd);
void whoClient(int sockfd);
void timeClient(int sockfd);
void whoisClient(char* remaining, int sockfd);
void whoClient(int sockfd);
void privmsgClient(char* remaining, int sockfd);
void bcastClient(char* remaining, int sockfd);
void quitClient(char* remaining, int sockfd);
