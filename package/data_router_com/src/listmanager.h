#ifndef _LISTMANAGER_H_ 
#define _LISTMANAGER_H_ 

//C语言线性表-书序表
#include <stdio.h>
#include <stdlib.h>
#include "qcommunicatebase.h"

#define MAXLISTSIZE 1024  /* 定义顺序表最大容量 */
#define MAX_DATA_BUFFER_SIZE        256   //数据缓存的最大值
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                 串口通信协议(MODBUS|RTU) -- 宏与结构体
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#define SERIAL_COMMAND_READ          0x03//读一或多个字
#define SERIAL_COMMAND_WRITE_ONE     0x06//写一个字
#define SERIAL_COMMAND_WRITE_MULTI   0x10//写多个字

typedef struct //串口通信协议的读消息主体(总计6个字节)
{
    UCHAR m_ucAddr;		 //从机地址(1个字节)
    UCHAR m_ucCommand;   //命令码(1个字节)
    UCHAR m_ucHStartAddr;//起始地址高位(1个字节)
    UCHAR m_ucLStartAddr;//起始地址低位(1个字节)
    UCHAR m_ucHReadNum;	 //数据个数高位(1个字节)
    UCHAR m_ucLReadNum;	 //数据个数低位(1个字节)
}tagSerialNormalReadBody;

typedef struct  //串口通信协议的写一个字的消息主体(总计6个字节)
{
    UCHAR m_ucAddr;		    //从机地址(1个字节)
    UCHAR m_ucCommand;      //命令码(1个字节)
    UCHAR m_ucHWriteAddr;   //写数据地址高位(1个字节)
    UCHAR m_ucLWriteAddr;   //写数据地址低位(1个字节)
    UCHAR m_ucHWriteContent;//数据内容高位(1个字节)
    UCHAR m_ucLWriteContent;//数据内容低位(1个字节)
}tagSerialNormalWriteOneBody;

typedef struct  //串口通信协议的写多个字的消息主体(总计7个字节)
{
    UCHAR m_ucAddr;		    //从机地址(1个字节)
    UCHAR m_ucCommand;      //命令码(1个字节)
    UCHAR m_ucHWriteAddr;   //写数据起始地址高位(1个字节)
    UCHAR m_ucLWriteAddr;   //写数据起始地址低位(1个字节)
    UCHAR m_ucHWriteNum;    //数据个数高位(1个字节)
    UCHAR m_ucLWriteNum;    //数据个数低位(1个字节)
    UCHAR m_ucByteNum;      //字节数(1个字节)
}tagSerialNormalWriteMultiBody;

typedef struct  //串口通信协议的读消息应答头部(总计3个字节)
{
    UCHAR m_ucAddr;		//从机地址(1个字节)
    UCHAR m_ucCommand;	//命令码(1个字节)
    UCHAR m_ucReadNum;	//字节个数(1个字节)
}tagSerialNormalReadAck;

typedef struct  //串口通信协议的写多个字的消息应答头部(总计6个字节)
{
    UCHAR m_ucAddr;		    //从机地址(1个字节)
    UCHAR m_ucCommand;      //命令码(1个字节)
    UCHAR m_ucHWriteAddr;   //写数据起始地址高位(1个字节)
    UCHAR m_ucLWriteAddr;   //写数据起始地址低位(1个字节)
    UCHAR m_ucHWriteNum;    //数据个数高位(1个字节)
    UCHAR m_ucLWriteNum;    //数据个数低位(1个字节)
}tagSerialNormalWriteMultiAck;

typedef struct  //串口通信协议的错误消息回应(总计3个字节)
{
    UCHAR m_ucAddr;		//从机地址(1个字节)
    UCHAR m_ucCommand;	//命令码(1个字节)
    UCHAR m_ucFault;	//错误代码(1个字节)
}tagSerialFaultAck;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                 公共资源
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#define SPANDAY_SEC 86400000 //24:00:00转为毫秒的数值(考虑跨天情况的时间)

typedef enum       //任务类型
{
    TT_0TempWrite,	 //临时写任务(优先级最高)
    TT_1TempRead,	 //临时读任务
    TT_2PeriodShake, //周期示波握手任务
    TT_3PeriodRead,	 //周期读数据任务(优先级最低)
    TT_Bottom
}enTaskType;

typedef enum      //任务状态
{
    TS_Ready,        //就绪
    TS_Pause,        //暂停
    TS_Delete,       //待删除
    TS_Running,      //执行中
    TS_Error,        //出错(例如通信超时、CRC校验失败等等原因)
    TS_Finish,       //结束(包括成功和失败)
    TS_Bottom
}enTaskState;

typedef enum  //任务出错的原因
{
    ERR_None = 0,    //成功完成,没有错误
    ERR_TimeOut = 100, //任务超时
    ERR_CRC,         //CRC校验错误
    ERR_Data         //数据个数不匹配或值不对
}enTaskErrReason;

typedef enum     //用户修改任务的方式
{
    MF_All,//针对所有
    MF_Addr_Type,//针对指定地址(串口从机地址或以太网IP地址)和指定任务类型
    MF_Addr_AllId,//针对指定地址(串口从机地址或以太网IP地址)和所有身份标记
    MF_Addr_OneId,//针对指定地址(串口从机地址或以太网IP地址)和指定身份标记(id1)
    MF_Addr_MultiId//针对指定地址(串口从机地址或以太网IP地址)和指定区间(id1~id2)的身份标记
} enModifyWay;

typedef enum    //用户修改任务的参数类型
{
    PA_TaskState,//修改任务的状态(删除TS_Delete,暂停TS_Pause,重启TS_Ready)
    PA_TaskSpan,//修改任务的周期
    PA_TaskOvertime,//修改任务的超时时间
    PA_TaskRetry//修改任务的失败重试次数
}enModifyPara;

typedef struct  //用户修改任务的属性
{
    enModifyWay way;
    enTaskType type;
    enModifyPara para;
    ULONG addr;
    ULONG id1;
    ULONG id2;
}tagUserAction;

typedef void (*FUNC_TASKCALLBACK)(ULONG ulTaskID,//任务的身份标识
                                  ULONG ulDeviceAddr,//变频器的通信地址(串口从机地址或以太网IP地址)
                                  UCHAR ucCommand,//命令码
                                  ULONG ulDataAddr,//数据地址
                                  ULONG ulDataNum,//读/写数据个数
                                  USHORT *pusDataBuf,//任务的数据buf,读操作用于存储接收值;写操作用于存储发送值;
                                  ULONG ulReserve,//保留(起预留参数的作用,以备不时之需)
                                  UCHAR enReason//任务出错的原因
                                 );

typedef struct    //任务元素的结构体
{
    enTaskType m_enTaskType;//任务类型
    enTaskState m_enTaskState;//任务状态,注意:初始化时该值应为TS_Ready.
    ULONG m_ulTaskID;//任务的身份标识
    ULONG m_usTaskSpan;//周期任务的时间间隔(单位:毫秒)
    ULONG m_dwTaskStartTime;//任务上一次执行的起始时间(单位:毫秒),注意:初始化时该值应为0.
    USHORT m_usTaskOverTime;//任务超时的许可范围,注意:初始化时该值不能为0.
    UCHAR m_ucTaskRetry;//任务失败重试的次数,注意:初始化时该值不能为0.
    UCHAR m_ucTaskRetry_Backup;//任务失败重试的次数备份,注意:初始化时该值等于m_ucTaskRetry.
    FUNC_TASKCALLBACK m_pFunTaskCallback;//任务的回调函数
    ULONG m_ulDeviceAddr;//变频器的通信地址(串口从机地址或以太网IP地址)
    UCHAR m_ucCommand;//变频器的命令码
    ULONG m_ulDataAddr;//变频器的数据地址
    ULONG m_ulDataNum;//变频器的读/写数据的字数(单位:字,一个字等于两个字节)
    USHORT m_usDataBuf[MAX_DATA_BUFFER_SIZE];//任务的数据buf,读操作用于存储接收值;写操作用于存储发送值;
    ULONG m_ulSendBufSize;//任务发送的实际有效数据的长度,该值小于等于MAX_SEND_BUFFER_SIZE
    ULONG m_ulReserve;//保留(起预留参数的作用,以备不时之需)
}tagTaskItem;

typedef struct  /* 定义顺序表节点类型 */
{
    tagTaskItem taskItem[MAXLISTSIZE];  /* 顺序表*/
    int iAllNum; /*顺序表元素个数 */
}linearlist;

void ListList(linearlist* list); /* 打印线性顺序表 */
void Output(linearlist* list); /* 打印说明文档 */
linearlist* CreateList();/* 创建线性顺序表 */
void AppendNode(linearlist* list, tagTaskItem taskItemTemp); /* 追加节点 */
void InsertNode(linearlist* list, tagTaskItem taskItemTemp, int pos); /* 插入节点 */
void DeleteNode(linearlist* list, int pos);  /* 删除节点 */
void pop_front(linearlist* list);//删除最前的一个元素
void push_back(linearlist* list, tagTaskItem taskItemTemp);//从最后面插入元素
void push_front(linearlist* list, tagTaskItem taskItemTemp);//从最前面插入元素

#endif
