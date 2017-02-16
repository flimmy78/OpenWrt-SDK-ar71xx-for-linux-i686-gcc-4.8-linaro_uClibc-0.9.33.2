#define OS_UNIX
#ifdef OS_UNIX
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include<string.h>
#endif

#ifdef OS_WIN
#include <window.h>
#include <studio.h>
#endif

#define SHM_MODE (SHM_R | SHM_W | IPC_CREAT )

#define UPDATAMAX 1024
#define UPCMDDATAMAX 2000
#define DRIVERNUM 2
#define DEVICENUM 8
#define MAXDOWNCMDNUM 100
#define CHDOWNCMD 1000
#define DEVICETYPESIZE 50
#define DOWD_CMD_LENGTH_MAX 1024
#ifdef OS_UNIX
#define SHMDBKEYRTRAM_RT 114556257
#define STATION_SIZE 70000
#endif

//保存设备单个地址和值
typedef struct
{
    int iAddr;
    unsigned short usValue;
}StructModbusUnit;

//单台设备的信息
typedef struct
{
    int iDeviceID;//设备地址
    char cDeviceTypes[DEVICETYPESIZE];//a01 光伏逆变器
    int iUpAddrNum;//有多少个地址
    StructModbusUnit unitAddrValue[UPDATAMAX];//保存地址的信息和值
}StructUpModbusData;

typedef struct
{
    int idDriver;//两个采集驱动程序的编号  由配置表决定
    StructUpModbusData  upDeviceData[DEVICENUM];//本驱动最多挂8台设备
    int ideviceNum;//本驱动挂载的设备数
}StructDriverUpData;

//typedef struct
//{
//    int idDriver;//保存当前下发指令是哪个驱动的
//    int iDownCmdLength;//远程下发命令原始长度
//    char downCmd[DOWD_CMD_LENGTH_MAX];//远程下发命令原始数据
//}StructDriverDownData;
typedef struct
{
    int iFlagCom1;//上传全部数据标志位 2有 0无
    int iFlagCom3;//标记COM1是否有下发指令 0 无 1 有
    int iLenCom1;//标记COM1下发指令的长度
    char downCmdCom1[DOWD_CMD_LENGTH_MAX];//Com1远程下发命令原始数据
    int iFlagCom2;//上传全部数据标志位 2有 0无
    int iFlagCom4;//标记COM1是否有下发指令 0 无 1 有
    int iLenCom2;//标记COM1下发指令的长度
    char downCmdCom2[DOWD_CMD_LENGTH_MAX];//Com1远程下发命令原始数据
    int iFlagNetwork;//网络接通标志位0断开1接通
    int iHandMaxAddr;//握手成功的最大地址值
}StructDriverDownData;

typedef struct
{
    int iVersion;
    StructDriverDownData downData;//单个驱动单次下发的指令,在数据采集其驱动去解析
    int iDriverNum;//驱动所挂载的设备数目
    StructDriverUpData upData;//单个驱动所有数据
}StructStation;

StructStation *pStation;

int open_ramrt();//open shared ram
void wait_for_millisec(int millisec);

//获取网络接通标志位
int getNetworkFlat();

//获取COM1是否可以保存下发指令,0可以，1不可以
int getDownDriver1();
int getDownDriver3();

//获取COM2是否可以保存下发指令,0可以，1不可以
int getDownDriver2();
int getDownDriver4();

//设置网络接通标志位
int setNetworkFlat(int iValue);

//设置COM1是否可以保存下发指令,0可以，1不可以
int setDownDriver1(int iValue);
int setDownDriver3(int iValue);

//设置COM2是否可以保存下发指令,0可以，1不可以
int setDownDriver2(int iValue);
int setDownDriver4(int iValue);

//设置在线设备的最大地址
int setiHandAddr(int iValue);

//获取在线设备的最大地址
int getiHandAddr();

//获取远程下载命令字符串大小
int getDownCmdLengthCom1();
int getDownCmdLengthCom2();

//设置远程下载命令字符串大小
int setDownCmdLengthCom1(int iLen);
int setDownCmdLengthCom2(int iLen);

//清空远程下载命令字符串大小
int clrDownCmdCom1();

//清空远程下载命令字符串大小
int clrDownCmdCom2();

//获取远程下载命令字符串
char *getDownCmdCom1();
char *getDownCmdCom2();

//设置远程下载命令字符串
int setDownCmdCom1(char *cmd);
int setDownCmdCom2(char *cmd);

//获取是否所有数据都已上传, 0都已上传,1还有数据没有传完
int getIsAllUpdate();

//获取远程下载命令驱动号
//int getDownDriverIndex();
//设置远程下载命令驱动号
//int setDownDriverIndex(int iIndex);

//获取远程下载命令字符串大小
//int getDownCmdLength();
//设置远程下载命令字符串大小
//int setDownCmdLength(int length);

//获取远程下载命令字符串
//char *getDownCmd();
//设置远程下载命令字符串
//int setDownCmd(char *cmd);

//根据驱动号获取该设驱动挂载的设备数目
int getDeviceNumByDriverIndex(int idDriver);

//设置驱动挂载的设备数目
void setDeviceNumByDriverIndex(int idDriver, int iNum);

//根据驱动号来获取该驱动所有要上传的信息
//StructDriverUpData* getSignalDriverUpData(int idDriver);

//根据驱动号来设置该驱动所有要上传的信息
//int setSignalDriverUpData( int idDriver, StructDriverUpData *data);

//根据设备号和设备数组下标号来获取单台设备上传信息
StructUpModbusData * get_device_up_data( int driver_id,int device_id);

//根据设备号和设备数组下标号来设置单台设备上传信息
int set_device_up_data( int driver_id,int device_id,StructUpModbusData *data);

//根据驱动号和设备序号获取设备ID
int get_up_device_addr(  int driver_id,int device_id);
//根据驱动号和设备序号设置设备ID
int set_up_device_addr( int driver_id,int device_id,int addr);

//根据驱动号和设备序号获取设备类型
int get_up_device_type(  int driver_id,int device_id);
//根据驱动号和设备序号设置设备类型
int set_up_device_type( int driver_id,int device_id,char *type);

//获取软件版本
int get_device_verison( );
//设置软件版本
int set_device_verison( int version);















