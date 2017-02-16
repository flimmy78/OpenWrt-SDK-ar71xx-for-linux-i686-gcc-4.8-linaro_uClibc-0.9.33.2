#include "ramrt.h"

int open_ramrt()
{
#ifdef OS_UNIX
    int shmid;
    void *shmptr;

    if ( (shmid = shmget( SHMDBKEYRTRAM_RT, STATION_SIZE, SHM_MODE) ) < 0 )
    {
        printf("shmget error!\n");
        return 0;
    }

    shmptr = shmat( shmid , 0, 0);
    if ( shmptr == (void *)-1)
    {
         printf("shmat error!\n");
        return 0;
    }
    pStation = (StructStation *)(shmptr);

    return 1;

#endif
}

void wait_for_millisec(int millisec)
{
#ifdef OS_UNIX
    usleep(millisec * 1000);
#endif
}
//获取网络接通标志位
int getNetworkFlat()
{
    return pStation->downData.iFlagNetwork;
}
//COM1是否可以上传全部数据标志位,0可以，2不可以
int getDownDriver1()
{
    return pStation->downData.iFlagCom1;
}

//获取COM1是否可以保存下发指令,0可以，1不可以
int getDownDriver3()
{
    return pStation->downData.iFlagCom3;
}

//获取COM2是否可以保存下发指令,0可以，1不可以
int getDownDriver2()
{
    return pStation->downData.iFlagCom2;
}
//获取COM2是否可以保存下发指令,0可以，1不可以
int getDownDriver4()
{
    return pStation->downData.iFlagCom4;
}

//设置网络接通标志位
int setNetworkFlat(int iValue)
{
    return pStation->downData.iFlagNetwork= iValue;
}
//设置COM1上传全部数据标志位,2可以，0不可以
int setDownDriver1(int iValue)
{
    return pStation->downData.iFlagCom1 = iValue;
}


//设置COM1是否可以保存下发指令,0可以，1不可以
int setDownDriver3(int iValue)
{
    return pStation->downData.iFlagCom3 = iValue;
}

//设置COM2上传全部数据标志位,2可以，0不可以
int setDownDriver2(int iValue)
{
    return pStation->downData.iFlagCom2 = iValue;
}


//设置COM2是否可以保存下发指令,0可以，1不可以
int setDownDriver4(int iValue)
{
    return pStation->downData.iFlagCom4 = iValue;
}

//设置在线设备的最大地址
int setiHandAddr(int iValue)
{
     return pStation->downData.iHandMaxAddr = iValue;
}

//获取在线设备的最大地址
int getiHandAddr()
{
    return  pStation->downData.iHandMaxAddr;
}
//获取远程下载命令驱动号
//int getDownDriverIndex()
//{
//    return pStation->downData.idDriver;
//}

//int setDownDriverIndex(int iIndex)
//{
//    pStation->downData.idDriver = iIndex;
//    return pStation->downData.idDriver;
//}

int getDownCmdLengthCom1()
{
    return pStation->downData.iLenCom1;
}

int getDownCmdLengthCom2()
{
    return pStation->downData.iLenCom2;
}

int setDownCmdLengthCom1(int iLen)
{
        return pStation->downData.iLenCom1 = iLen;
}

int setDownCmdLengthCom2(int iLen)
{
        return pStation->downData.iLenCom2 = iLen;
}

char *getDownCmdCom1()
{
    return pStation->downData.downCmdCom1;
}

char *getDownCmdCom2()
{
    return pStation->downData.downCmdCom2;
}

//清空远程下载命令字符串
int clrDownCmdCom1()
{
    printf("clrDownCmdCom1()\n");
        memset(pStation->downData.downCmdCom1, 0, DOWD_CMD_LENGTH_MAX);
}

//清空远程下载命令字符串
int clrDownCmdCom2()
{
        memset(pStation->downData.downCmdCom2, 0, DOWD_CMD_LENGTH_MAX);
}

//设置远程下载命令字符串
int setDownCmdCom1(char *cmd)
{
    if (cmd != NULL || strlen(cmd) < DOWD_CMD_LENGTH_MAX )
    {
        memcpy(pStation->downData.downCmdCom1, cmd, strlen(cmd));
        return 1;
    }
    else
        return 0;
}

int setDownCmdCom2(char *cmd)
{
    if (cmd != NULL || strlen(cmd) < DOWD_CMD_LENGTH_MAX )
    {
        memcpy(pStation->downData.downCmdCom2, cmd, strlen(cmd));
        return 1;
    }
    else
        return 0;
}

//获取远程下载命令字符串大小
//int getDownCmdLength()
//{
//    return pStation->downData.iDownCmdLength;
//}

//设置远程下载命令字符串大小
//int setDownCmdLength(int length)
//{
//        pStation->downData.iDownCmdLength = length;
//        return length;
//}

//获取远程下载命令字符串
//char *getDownCmd()
//{
//    return pStation->downData.downCmd;
//}

//设置远程下载命令字符串
//int setDownCmd(char *cmd)
//{
//    if (cmd != NULL || strlen(cmd) < DOWD_CMD_LENGTH_MAX )
//    {
//        memcpy(pStation->downData.downCmd,cmd,strlen(cmd));
//        return 1;
//    }
//    else
//        return 0;
//}

//设置驱动挂载的设备数目
void setDeviceNumByDriverIndex(int idDriver, int iNum)
{
    if (idDriver >= DRIVERNUM)
    {
        return -1;
    }
    pStation->iDriverNum = iNum;
//    if (0 == idDriver)
//    {
//        pStation->iDriver1Num = iNum;
//    }
//    else
//    {
//        pStation->iDriver2Num = iNum;
//    }
}

//根据设备号获取该设驱动挂载的设备数目
int getDeviceNumByDriverIndex(int idDriver)
{
    if (idDriver >= DRIVERNUM)
    {
        return -1;
    }
    return pStation->iDriverNum;
//    if (0 == idDriver)
//    {
//        return pStation->iDriver1Num;
//    }
//    else
//    {
//        return pStation->iDriver2Num;
//    }
}

//根据驱动号来获取该驱动所有要上传的信息
//StructDriverUpData* getSignalDriverUpData(int idDriver)
//{
//    if (idDriver >= DRIVERNUM)
//    {
//        return -1;
//    }
//    return &pStation->up_data[idDriver];
//}

//根据驱动号来设置该驱动所有要上传的信息
//int setSignalDriverUpData( int idDriver, StructDriverUpData *data)
//{
//    if ( idDriver >= DRIVERNUM || idDriver < 0)
//    {
//        return -1;
//    }
//    if ( idDriver >= DEVICENUM || idDriver < 0)
//    {
//        return -1;
//    }
//    printf("7777777777 \n");
//    //运行到该处直接崩溃
//    memcpy(&pStation->up_data[idDriver] , data, sizeof(StructDriverUpData));
//    printf("8888888888 \n");
//    return 0 ;
//}

//根据设备号和设备数组下标号来获取单台设备上传信息
StructUpModbusData * get_device_up_data( int driver_id,int device_id)
{
    if ( driver_id >= DRIVERNUM || driver_id < 0)
    {
                return -1;
    }
    if ( device_id >= DEVICENUM || device_id < 0)
    {
                return -1;
    }

    return &pStation->upData.upDeviceData[device_id] ;

//    if (0 == driver_id)
//    {
//        return &pStation->upCom1Data.upDeviceData[device_id] ;
//    }
//    else
//    {
//        return &pStation->upCom2Data.upDeviceData[device_id] ;
//    }
}

//根据设备号和设备数组下标号来设置单台设备上传信息
int set_device_up_data( int driver_id,int device_id,StructUpModbusData *data)
{
    if ( driver_id >= DRIVERNUM || driver_id < 0)
    {
        return -1;
    }
    if ( device_id >= DEVICENUM || device_id < 0)
    {
        return -1;
    }
    memcpy(&pStation->upData.upDeviceData[device_id] , data, sizeof(StructUpModbusData));
//    if (0 == driver_id)
//    {
//        memcpy(&pStation->upCom1Data.upDeviceData[device_id] , data, sizeof(StructUpModbusData));
//    }
//    else
//    {
//        memcpy(&pStation->upCom2Data.upDeviceData[device_id] , data, sizeof(StructUpModbusData));
//    }
    return 0 ;
}

//根据驱动号和设备序号获取设备ID
int get_up_device_addr(  int driver_id,int device_id)
{
    if ( driver_id > DRIVERNUM || driver_id < 0)
    {
                return -1;
    }
    if ( device_id > DEVICENUM || device_id < 0)
    {
                return -1;
    }

    return pStation->upData.upDeviceData[device_id].iDeviceID;

//    if (0 == driver_id)
//    {
//        return pStation->upCom1Data.upDeviceData[device_id].iDeviceID;
//    }
//    else
//    {
//         return pStation->upCom2Data.upDeviceData[device_id].iDeviceID;
//    }
}

//根据驱动号和设备序号设置设备ID
int set_up_device_addr( int driver_id,int device_id,int addr)
{
    if ( driver_id > DRIVERNUM || driver_id < 0)
    {
                return -1;
    }
    if ( device_id > DEVICENUM || device_id < 0)
    {
                return -1;
    }

    pStation->upData.upDeviceData[device_id].iDeviceID = addr;
//    if (0 == driver_id)
//    {
//        pStation->upCom1Data.upDeviceData[device_id].iDeviceID = addr;
//    }
//    else
//    {
//        pStation->upCom2Data.upDeviceData[device_id].iDeviceID = addr;
//    }
}

//根据驱动号和设备序号获取设备类型
int get_up_device_type(  int driver_id,int device_id)
{
    if ( driver_id > DRIVERNUM || driver_id < 0)
    {
                return -1;
    }
    if ( device_id > DEVICENUM || device_id < 0)
    {
                return -1;
    }

    return pStation->upData.upDeviceData[device_id].cDeviceTypes;

//    if (0 == driver_id)
//    {
//        return pStation->upCom1Data.upDeviceData[device_id].cDeviceTypes;
//    }
//    else
//    {
//        return pStation->upCom2Data.upDeviceData[device_id].cDeviceTypes;
//    }
}

//根据驱动号和设备序号设置设备类型
int set_up_device_type( int driver_id,int device_id,char *type)
{
    if ( driver_id > DRIVERNUM || driver_id < 0)
    {
                return -1;
    }
    if ( device_id > DEVICENUM || device_id < 0)
    {
                return -1;
    }

    memcpy(pStation->upData.upDeviceData[device_id].cDeviceTypes,type,sizeof(char)*DEVICETYPESIZE);

//    if (0 == driver_id)
//    {
//        memcpy(pStation->upCom1Data.upDeviceData[device_id].cDeviceTypes,type,sizeof(char)*DEVICETYPESIZE);
//    }
//    else
//    {
//        memcpy(pStation->upCom2Data.upDeviceData[device_id].cDeviceTypes,type,sizeof(char)*DEVICETYPESIZE);
//    }
}

//获取软件版本
int get_device_verison( )
{
    return pStation->iVersion;
}

//设置软件版本
int set_device_verison( int version)
{
    return pStation->iVersion = version;
}

//获取是否所有数据都已上传, 0都已上传,1还有数据没有传完
int getIsAllUpdate()
{
    int i = 0;
    for (i = 0; i < pStation->iDriverNum; i++)
    {
        if (pStation->upData.upDeviceData[i].iUpAddrNum > 0)
        {
            printf("pStation->upData.upDeviceData[%d].iUpAddrNum  = %d\n",i,pStation->upData.upDeviceData[i].iUpAddrNum);
            return 1;
        }
    }
    return 0;
}
