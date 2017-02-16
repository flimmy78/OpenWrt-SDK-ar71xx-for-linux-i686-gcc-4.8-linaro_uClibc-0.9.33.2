#ifndef _QCOMMUNICATEMANAGE_H_ 
#define _QCOMMUNICATEMANAGE_H_ 

#include <time.h>
#include "listmanager.h"
#include <pthread.h>

void CreateThread_Manage(void);//创建线程
void* ThreadProc_Manage(void* pvoid);//管理者线程
extern bool UserAddTask(tagTaskItem item); //用户手动新增任务
void ExitThread_Manage(void);//退出线程
void UpdateCurrentTask(enTaskState state);//更新正在执行的任务
void CleanCurrentTask(void);//清除正在执行的任务
bool UserModifyTask(tagUserAction action, USHORT val);//用户手动修改任务
ULONG UserGetMillisecond(void);//用户得到当前的系统时间(单位:毫秒)
USHORT GetCRC(UCHAR *pChar, int iNum);//得到CRC校验值
void ErgodicList(enTaskType type);//遍历链表,找出最迫切需要执行的任务,放到链表首部
void Serial_DealWith_SendNormal(void);//串口:构造需要发送的内容
void Serial_DealWith_RecvNormal(UCHAR *pRecvBuf, ULONG addr, int iLen); //串口:处理接收到的数据
void Serial_DealWith_RecvNormal_Correct(UCHAR *pRecvBuf);//串口:处理接收到的数据(应答正确)
void Serial_DealWith_RecvNormal_Fault(UCHAR *pRecvBuf);//串口:处理接收到的数据(应答错误)
void SFunCallback_RecvNormal(UCHAR *pRecvBuf, ULONG addr, int len);

UCHAR m_ucTaskErrReason;//任务出错的原因
UCHAR m_ucSendBuf[MAX_SENDNORMAL_BUFFER_SIZE];
UCHAR m_ucRecvBuf[MAX_RECVNORMAL_BUFFER_SIZE];
int m_iRecvLen;
pthread_t thread_manager;
pthread_mutex_t work_mutex;
bool m_bManagerExit;
tagTaskItem m_TaskCurrent;//正在执行的任务
linearlist *m_list_TaskStorage[TT_Bottom];

#endif // QCOMMUNICATEMANAGE_H
