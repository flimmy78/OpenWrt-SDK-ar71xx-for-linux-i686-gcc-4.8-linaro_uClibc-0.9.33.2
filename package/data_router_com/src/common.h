#ifndef _COMMON_H_ 
#define _COMMON_H_ 

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>          //文件控制定义
#include <termios.h>
#include <errno.h>

#define OS_UNIX
#ifdef OS_UNIX
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#endif
#include "common.h"
#ifdef OS_WIN
#include <window.h>
#include <studio.h>
#endif

//保留这个宏是定义这个驱动是COM1,去掉是COM2,这两个驱动只有这个地方不一样，做的同一个工程两个驱动
#define DRIVE_COM1

//开启这个宏地址是配置表一个一个读的，配置表有三项设备名称,设备地址,寄存器地址
//配置标可以随意配置8台设备要读的寄存器地址，可以乱序，软件自动排列组帧
//配置表的第一行不读
//#define SIGNAL_ADDR

//开启这个宏输出调试信息
//#define DEBUG_INFO

#define SHM_MODE (SHM_R | SHM_W | IPC_CREAT )

#define UPDATAMAX 1000
#define UPCMDDATAMAX 2000
#define DRIVERNUM 2
#define DEVICENUM 8
#define MAXDOWNCMDNUM 100
#define CHDOWNCMD 1000
#define DEVICETYPESIZE 50
#define DOWD_CMD_LENGTH_MAX 1024
#ifdef OS_UNIX
#define SHMDBKEYRTRAM_RT 114556255
#define STATION_SIZE 20000
#endif

#define MAX_LINE_SIZE 255
#define MAX_READ_LEN 16
#define MAX_LEN 255
#define MAX_DEV_NUM 254
#define DEVICETYPESIZE 50

#define SERIAL_CONFIG_PATH "/etc/config/comm"
#define ADDR_CONFIG_PATH_COM1 "/etc/config/RS485_1"
#define ADDR_CONFIG_PATH_COM2 "/etc/config/RS485_2"

#define SERIAL_CONFIG "/etc/config/comm" //串口配置文件
#define SERIAL_CONFIG_NAME "comm"

#define MQTT_CONFIG "/etc/config/broker" //mqtt 配置文件
#define MQTT_CONFIG_NAME "broker"
#define MQTT_CONFIG_TEMP "/etc/config/broker_temp" //mqtt 配置文件

#define COM1_CONFIG "/etc/config/RS485_1" //串口1地址 配置文件
#define COM1_CONFIG_NAME "RS485_1"

#define COM2_CONFIG "/etc/config/RS485_2" //串口2地址 配置文件
#define COM2_CONFIG_NAME "RS485_2"

#define bool int
#define false 0
#define true 1

typedef unsigned long  ULONG;
typedef unsigned short USHORT;
typedef unsigned char  UCHAR;
typedef unsigned short SOCKET;

typedef struct  //数据发送结构体
{
    unsigned short  usAddrStart;//读数据帧起始地址
    unsigned char   ucDataNum;//读取长度
}SignalDataFrame;

typedef struct
{
    SignalDataFrame* dataFrame;//所有读取的帧
    int iDeviceID;//设备ID
    int iFrameNum;//本台设备读取帧的总数
}DeviceDataFrame;

typedef struct
{
    char sComNum[32];
    int	 iBaudRate;
    int	 iDataBit;
    int	 iStopBit;
    char sVerifyBit[10];
    int    iVerifyBit;
    int    iTaskTime;//任务周期
    int    iResend;//重发次数
    int	 iTimeout;
    int    iHandAddr;//握手地址
    int    iHandTime;//握手周期
    int    iUpAllDataTime;//上传全部数据周期
    int    ijsonType;
    char cjsonAddrKey[20];
    int    iHisDateSave;
}SerialConfig;

//配置表的格式：
//设备名称 设备地址 寄存器地址 寄存器名称 读写类型
//为了节省内存空间 只保设备地址 寄存器地址

typedef struct //读取配置表的数据结构
{
    int iDeviceID;//设备地址
    unsigned short usVarAddr;//寄存器地址
//    short sVarValue;//寄存器的值
}DeviceConfigInfo;

typedef struct //数据存储结构体
{
    unsigned short usVarAddr; //寄存器地址
    USHORT sVarValue;//寄存器的值(当前值)
    USHORT sVarOldValue;//寄存器上次的值,因为要省流量，相同值不上报
}DeviceDataInfo;

typedef struct
{
    DeviceDataInfo* deviceData;//该设备所有的寄存器地址和值
    char cDeviceTypes[DEVICETYPESIZE];//a01 光伏逆变器
    int iDeviceID;//设备ID
    int iDeviceNum;//寄存器的个数
    bool bFlagTask;//添加了通讯任务
    bool bHand;//成功握手
    bool bReceive;//成功接受数据
    bool bSaveHand;//保存握手数据
}SignalDevice;

typedef struct //读取配置表的数据结构
{
    int iDeviceID;//设备地址
    unsigned short usVarAddr;//寄存器起始地址
    int iNum;//连读的个数
    int iSampleTime;//采样周期(分钟)
}DeviceMulConfigInfo;

typedef struct
{
    int iDeviceID;//设备地址
    int iNum;//要读取的地址个数
}SignalDeviceInfo;

//获取配置表中有几台机器，每台机器读取的地址个数
typedef struct
 {
     SignalDevice* deviceInfo;//每台设备包含的信息
     int iDeviceNum;//标记有几台设备
     int idDriver;//两个采集驱动程序的编号  由配置表决定 0表示串口驱动1,1表示串口驱动2
 }DeviceAddrInfo;

typedef enum
{
    EN_COM1_VALUE = 1,
    EN_COM2_VALUE
}EN_DRIVE_NUM_VALUE;

typedef enum
{
    EN_COM1 = 0,
    EN_COM2
}EN_DRIVE_NUM;

SignalDeviceInfo* deviceNumInfoCOM1;//保存每台设备要读的寄存器数量
SignalDeviceInfo* deviceNumInfoCOM2;
#ifdef SIGNAL_ADDR
DeviceConfigInfo* allDeviceConfigInfoCOM1;//一次保存COM1配置表所有信息
DeviceConfigInfo* allDeviceConfigInfoCOM2; //一次保存COM2配置表所有信息
#else
DeviceMulConfigInfo* allDeviceConfigInfoCOM1;//一次保存COM1配置表所有信息
int iConfigCom1Num;//配置表1的帧个数
DeviceMulConfigInfo* allDeviceConfigInfoCOM2;//一次保存COM2配置表所有信息
int iConfigCom2Num;//配置表2的帧个数
DeviceConfigInfo* allDeviceCOM1;
DeviceConfigInfo* allDeviceCOM2;
//将连续的配置表转化为单个地址的配置表,返回值为单个地址个数
int changeConfigContinuToSignal(DeviceMulConfigInfo* pConfig, int iNum, EN_DRIVE_NUM enDrive);
#endif

extern  SerialConfig* SerialInfo;
DeviceAddrInfo  drive_data[DRIVERNUM];//两个驱动的数据都保存在这里,0是COM1的数据，1,是COM2的数据

int ReadComConfig(const char* path, EN_DRIVE_NUM enDrive);
int ReadXmlFile(const char* path);
int GetXmlElement(FILE* fp, const char* sName, char* sValue);

#ifdef SIGNAL_ADDR
unsigned int GetCsvTotalLine(FILE* fp);
int ReadAddrConfigFile(const char* path, EN_DRIVE_NUM enDrive);
void CreateDataFrame(void);
extern DeviceDataFrame* allDeviceDataFrame;
#endif

unsigned int GetAddrNum(FILE* fp);
int IncreaseSort(DeviceConfigInfo* deviceInfo, unsigned num, EN_DRIVE_NUM enDrive);
unsigned int GetFrameNum(SignalDevice src);

void ltrim ( char *s );
void rtrim ( char *s );
void trim ( char *s );
#endif 


