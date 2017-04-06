#ifndef APPTCP_H
#define APPTCP_H

#include <pthread.h>
#include "common.h"
#include "qcommunicatemanage.h"

#define bool int
#define false 0
#define true  1
#define THREAD_SLEEP_SPAN           1     //线程延时时间

void getUpBufTCP(char* destBuf, unsigned int  id, int iAddr, int iLen);
//int getValueBySplite(char* sour, char split);
void initThreadTCP();

bool m_bExitHttp;
void* ThreadTCP(void* pVoid);
void sendTCP(int client_sockfd, char* sendBuf);
int client_sockfdtcp;//客户端套接字
int charToHexTCP(char c);
int strToHexTCP(char* sour, unsigned char* dest);
//处理数据的回调函数
void receiveFunTCP(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
        ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
        UCHAR enReason );
pthread_t threadTCP;
#endif
