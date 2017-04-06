#ifndef _NETCOMMU_H_
#define _NETCOMMU_H_

#include <pthread.h>

#define bool int
#define false 0
#define true  1
#define THREAD_SLEEP_SPAN           1     //线程延时时间

void initThreadUDP();
bool m_bExitUDP;
void*  ThreadUDP(void* );
pthread_t threadUDP;

void initThreadTCP();
bool m_bExitTCP;
void*  ThreadTCP(void* );
pthread_t threadTCP;
#endif



