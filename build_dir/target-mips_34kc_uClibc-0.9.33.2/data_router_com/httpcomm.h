#ifndef HTTPCOMM_H
#define HTTPCOMM_H

#include <pthread.h>
#include "common.h"
#include "qcommunicatemanage.h"

#define bool int
#define false 0
#define true  1
#define THREAD_SLEEP_SPAN           1     //线程延时时间

void getUpBuf(char* destBuf, int id, int iAddr, int iLen);
int getValueBySplite(char* sour, char split);
void initThreadHttp();
bool m_bExitHttp;
void* ThreadHttp(void* pVoid);
void sendHttp(int client_sockfd, char* sendBuf);
int client_sockfd;//客户端套接字
int charToHex(char c);
int strToHex(char* sour, unsigned char* dest);
int StrReplace(char strRes[],char from[], char to[]) ;
//处理数据的回调函数
void receiveFunHttp(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
        ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
        UCHAR enReason );
pthread_t threadHttp;
#endif // HTTPCOMM_H
