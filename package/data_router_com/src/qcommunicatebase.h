#ifndef _QCOMMUNICATEBASE_H_ 
#define _QCOMMUNICATEBASE_H_ 

#include "common.h"
#include <pthread.h>

#define bool int
#define false 0
#define true 1

/* 注意:
通信类数据接收和发送分两类:
1.平常数据(Normal)
2.示波数据(Oscillo)
*/
#define MAX_SENDNORMAL_BUFFER_SIZE  256   //发送平常数据缓存的最大值
#define MAX_RECVNORMAL_BUFFER_SIZE  1024  //接收平常数据缓存的最大值
#define MAX_RECVOSCILLO_BUFFER_SIZE 32768 //接收示波数据缓存的最大值
#define OSCILLO_DATA_POINT_NUM      64    //示波一次采样多少个数据点
#define THREAD_SLEEP_SPAN           1     //线程延时时间
#define COM_TIME_OUT                10    //串口读写的延时

typedef void (*FUNC_RECVSUCCESS)(UCHAR *pRecvBuf, ULONG addr, int len); //接收成功的函数指针

int SerialInit(void);
int OpenCom(const char *dev);
int SetSpeed(int fd, int speed);
int SetParity(int fd, int databits, int stopbits, int parity,int iTimeOut);

void QCommuSerial_init(FUNC_RECVSUCCESS pRecv);
void QCommuSerial_free();
void SetBufSize_SendNormal(int iSendBufferSize);//设置发送平常数据缓冲区大小
void SetBufSize_RecvNormal(int iRecvBufferSize);//设置接收平常数据缓冲区大小
void SendData(/*char*/UCHAR *buffer, int len) ;//发送数据
void ClearComBuf(void) ;//清除串口缓冲区
int IsExist( );
bool Connect(void);
void Disconnect(void);
FUNC_RECVSUCCESS		m_pFuncRecvNormal;//接收成功时调用
char	*m_pBufSendNormal;//发送平常数据缓冲区
UCHAR	*m_pBufRecvNormal;//接收平常数据缓冲区
int		m_iBufSizeSendNormal;//发送平常数据缓冲区大小，默认为128
int		m_iBufSizeRecvNormal;//接收平常数据缓冲区大小，默认为1024
void*  Thread_Receive(void* pvoid);
bool m_bExit;
pthread_t thread_receive;
void *thread_result;
int fd_serail;

#endif



