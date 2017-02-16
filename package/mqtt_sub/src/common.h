#ifndef _COMMON_H_ 
#define _COMMON_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>          //文件控制定义
#include <termios.h>
#include <errno.h>

#define  MQTT_DEBUG

#define MAX_LINE_SIZE 255
#define MAX_READ_LEN 16
#define MAX_LEN 255
#define MAX_DEV_NUM 254
#define MAX_NAME_LEN 32 //名字最大长度

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

typedef struct //数据存储结构体
{
    unsigned short usVarAddr; //寄存器地址
    USHORT sVarValue;//寄存器的值
}DeviceDataInfo;

typedef struct
{
    char sComNum[MAX_NAME_LEN];
    int	 iBaudRate;
    int	 iDataBit;
    int	 iStopBit;
    char sVerifyBit[10];
    int    iVerifyBit;
    int    iTaskTime;//任务周期
    int	 iTimeout;
    int    iHandAddr;//握手地址
    int    iHandTime;//握手周期
}SerialConfig;

typedef struct
{
    char strAddress[MAX_NAME_LEN]; //服务器地址
    char strClientID[MAX_NAME_LEN];//客户ID
    char strBindAddtess[MAX_NAME_LEN];//采集器主板IP地址 自动获取 不用配置
    int iPort;//端口号
    char strTopic[MAX_NAME_LEN];//标题名
    int iQos;//qos模式 固定值不用配置
    int iKeepalive;//keepalive 固定值不用配置
    unsigned long ulTimeout;//超时时间 固定值不用配置
    char strUseName[MAX_NAME_LEN];//用户名
    char strPwd[MAX_NAME_LEN];//密码
}tagMqttInfo;

typedef struct
{
    int iType; //表明机器类型,1是GPRS采集器,2是数据采集器，这里要配置为2
    char strAddrKey[MAX_NAME_LEN];//地址前面加的标号
}tagJson;

//配置表的格式：
//设备名称 设备地址 寄存器地址 寄存器名称 读写类型
//为了节省内存空间 只保设备地址 寄存器地址
typedef struct //读取配置表的数据结构
{
    int iDeviceID;//设备地址
    unsigned short usVarAddr;//寄存器地址
    short sVarValue;//寄存器的值
}DeviceConfigInfo;

typedef struct
{
    DeviceDataInfo* deviceData;//该设备所有的寄存器地址和值
    int iDeviceID;//设备ID
    int iDeviceNum;//寄存器的个数
    bool bFlagTask;//添加了通讯任务
    bool bHand;//成功握手
    bool bReceive;//成功接受数据
    bool bSaveHand;//保存握手数据
}SignalDevice;

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
 }DeviceAddrInfo;

SignalDeviceInfo* deviceNumInfo;//保存每台设备要读的寄存器数量
extern unsigned int uVarNum ;
extern  DeviceConfigInfo* allDeviceConfigInfo; //一次保存所有信息
extern  DeviceAddrInfo allDeviceInfo; //按设备ID来保存所有的信息
extern  SerialConfig* SerialInfo;
extern  tagMqttInfo* mqttInfo;
extern  tagJson*  jsonInfo;
extern DeviceDataFrame* allDeviceDataFrame;

int WriteNewClientID(const char* path, const char* pathTemp, const char* newId);
int ReadMqttConfig(const char* path);
 int ReadSerialConfig(const char* path);
int ReadMqttConfigOnly(const char* path);
int ReadXmlFile(const char* path);
int GetXmlElement(FILE* fp, const char* sName, char* sValue);
int ReadCsvFile(const char* path);
unsigned int GetCsvTotalLine(FILE* fp);
int IncreaseSort(DeviceConfigInfo* deviceInfo, unsigned num);
unsigned int GetFrameNum(SignalDevice src);
void CreateDataFrame(void);
void ltrim ( char *s );
void rtrim ( char *s );
void trim ( char *s );
void replaceFirst(char *str1,char *str2,char *str3);
void replace(char *str1,char *str2,char *str3);

#endif 


