#pragma once
#define BUFFER_SIZE 256
#include <pthread.h>
#include "users.h"

extern pthread_cond_t thread_cond;
extern pthread_mutex_t thread_mutex;
extern pthread_mutex_t user_mutex;
extern pthread_t* thread_handles;
extern int numThreads;
extern Users* g_users;
