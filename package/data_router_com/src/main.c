#include <stdio.h>
#include "listmanager.h"
#include "ramrt.h"
#include "httpcomm.h"
#include "apptcp.h"

//#define DEBUG_MAIN

#define DRIVE1_CMD "Control"
#define DRIVE2_CMD "Control2"
#define DRIVE_ALL_UPDATE "{\"Act\":\"UploadAll\"}"

#define INIT_TIME 3 //启动十分钟上传所有数据

bool bFlagUpInit = false;
int iDevFailedNum = 1;
bool bModbusflag;

//传入参数驱动号,将不同驱动的所有数据存入共享内存区
void upDateAllData(EN_DRIVE_NUM EN_NUM);

//从共享内存中获取下发指令，并解析执行
void getDownCmdAndAction(EN_DRIVE_NUM_VALUE EN_VALUE, EN_DRIVE_NUM EN_NUM, char* cCmdValue);

//传入参数驱动号,将不同驱动的变化数据存入共享内存区
void upDateChangeData(EN_DRIVE_NUM EN_NUM);

//根据驱动号, 设备号和地址，地址个数保存值
int SaveData(EN_DRIVE_NUM EN_NUM, USHORT* buffer, unsigned short addr, unsigned char num , ULONG ID);

//处理数据的回调函数
void receiveFun(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                UCHAR enReason );
//处理数据的回调函数
void receiveFun1(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                UCHAR enReason );


int main()
{
    if (!open_ramrt())
    {
        printf("open ramrt error!\n");
    }

    Connect(); //读取串口配置,打开串口
    QCommuSerial_init(SFunCallback_RecvNormal);
    //再创建通讯任务管理线程
    //关闭串口时先要再关闭串口 ,再退出管理线程 Disconnect()  ExitThread_Manage()
    CleanCurrentTask();//清空当前任务
    CreateThread_Manage();//创建任务管理线程
    initThreadHttp();
    initThreadTCP();

    int i = 0;
#ifdef DRIVE_COM1
    setDownDriver1(0);
    setDownDriver3(0);
    //setNetworkFlat(1);

    setDeviceNumByDriverIndex(EN_COM1, drive_data[EN_COM1].iDeviceNum);//设置驱动有几台设备
    for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
    {
        drive_data[EN_COM1].deviceInfo[i].bFlagTask = false;
        drive_data[EN_COM1].deviceInfo[i].bHand = false;
        drive_data[EN_COM1].deviceInfo[i].bReceive = false;
        drive_data[EN_COM1].deviceInfo[i].bSaveHand = false;

        tagTaskItem taskItem;
        memset(&taskItem, 0 , sizeof(taskItem));
        taskItem.m_pFunTaskCallback = receiveFun;
        taskItem.m_ucCommand = SERIAL_COMMAND_READ;
        taskItem.m_enTaskType = TT_1TempRead;
        taskItem.m_enTaskState = TS_Ready;
        taskItem.m_ucTaskRetry = SerialInfo->iResend;
        taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
        taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
        taskItem.m_ulDataNum = 1;
        taskItem.m_ulDeviceAddr = drive_data[EN_COM1].deviceInfo[i].iDeviceID;
        taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
        taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
        UserAddTask(taskItem);
    }

#else
    setDownDriver2(0);
    setDownDriver4(0);
    setDeviceNumByDriverIndex(EN_COM2, drive_data[EN_COM2].iDeviceNum);//设置驱动有几台设备
    for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
    {
        drive_data[EN_COM2].deviceInfo[i].bFlagTask = false;
        drive_data[EN_COM2].deviceInfo[i].bHand = false;
        drive_data[EN_COM2].deviceInfo[i].bReceive = false;
        drive_data[EN_COM2].deviceInfo[i].bSaveHand = false;

        tagTaskItem taskItem;
        memset(&taskItem, 0 , sizeof(taskItem));
        taskItem.m_pFunTaskCallback = receiveFun;
        taskItem.m_ucCommand = SERIAL_COMMAND_READ;
        taskItem.m_enTaskType = TT_1TempRead;
        taskItem.m_enTaskState = TS_Ready;
        taskItem.m_ucTaskRetry = SerialInfo->iResend;
        taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
        taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
        taskItem.m_ulDataNum = 1;
        taskItem.m_ulDeviceAddr = drive_data[EN_COM2].deviceInfo[i].iDeviceID;
        taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
        taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
        UserAddTask(taskItem);
    }
#endif

    int iCountsTest = 0;//该参数仅仅用来测试采集数据变化，方便观察程序执行情况
    time_t nowtime;
    int iOldSec1 = time( &nowtime );//查询485通讯是否接通
    int iOldSec2= time( &nowtime );//上传全部数据计时
    int iOldSec3= time( &nowtime );//上传全部数据计时
    int iOldSec4= time( &nowtime );//重新查询在线设备
     int iOldSec5;//定时存储历史数据
    while(1)
    {
        //处理远程下发命令
#ifdef DRIVE_COM1
        getDownCmdAndAction(EN_COM1_VALUE, EN_COM1, DRIVE1_CMD);
#else
        getDownCmdAndAction(EN_COM2_VALUE, EN_COM2, DRIVE2_CMD);
#endif

        //每隔一段时间扫描所有终端设备，数目和时间间隔由配置表来定
        int iCurrentSec = time( &nowtime );
        if (iCurrentSec - iOldSec1 > 20)
        {
            iOldSec1 = iCurrentSec;
#ifdef DRIVE_COM1
            for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
            {
                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun1;
                taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                taskItem.m_enTaskType = TT_1TempRead;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = 1;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
                taskItem.m_ulDataNum = 1;
                taskItem.m_ulDeviceAddr = drive_data[EN_COM1].deviceInfo[i].iDeviceID;
                taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
                if(drive_data[EN_COM1].deviceInfo[i].bHand)
                    break;
            }
#else
            for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
            {
                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun1;
                taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                taskItem.m_enTaskType = TT_1TempRead;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = 1;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
                taskItem.m_ulDataNum = 1;
                taskItem.m_ulDeviceAddr = drive_data[EN_COM2].deviceInfo[i].iDeviceID;
                taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
            }
#endif
        }
        //printf("drive_data[EN_COM1].iDeviceNum= %d\n",drive_data[EN_COM1].iDeviceNum);
        if(iDevFailedNum>=drive_data[EN_COM1].iDeviceNum)
        {
    #ifdef DRIVE_COM1
                        printf("echo '0' > /sys/class/gpio/gpio1/value\n");
                        system("echo '0' > /sys/class/gpio/gpio1/value");
                        bModbusflag = false;
    #else
                        system("echo '0' > /sys/class/gpio/gpio26/value");
                        bModbusflag = false;
    #endif
                    iDevFailedNum=1;
        }

#ifdef DEBUG_MAIN
        printf("The first time Upload all data****iCurrentSec - iOldSec2 =%d\n",iCurrentSec - iOldSec2 );
#endif

        if (!bFlagUpInit)
        {
            if (iCurrentSec- iOldSec2 > INIT_TIME * 60)//上电五分钟后上传全部数据
            {
                bFlagUpInit = true;
                iOldSec5= time( &nowtime );//定时存储历史数据
#ifdef DRIVE_COM1
#ifdef DEBUG_MAIN
                printf("1*****getDownDriver1(1)=%d\n",getDownDriver1());
#endif
                if(getDownDriver1()==0)
                {
#ifdef DEBUG_MAIN
                    printf("The first time Upload all data****\n");
#endif
                    setDownDriver1(2);
#ifdef DEBUG_MAIN
                    printf("4*****getDownDriver1(1)=%d\n",getDownDriver1());
#endif
                    usleep(100*1000);
                    upDateAllData(EN_COM1);
                }
#else
                if(getDownDriver2()==0)
                {
                    setDownDriver2(2);
                    usleep(100*1000);
                    upDateAllData(EN_COM2);
                    setDownDriver2(0);
                }
#endif
            }
        }
#ifdef DEBUG_MAIN
         printf("Upload all data****iCurrentSec - iOldSec3 =%d\n",iCurrentSec - iOldSec3 );
#endif
        if (iCurrentSec- iOldSec3 >= SerialInfo->iUpAllDataTime)//定时上传全部数据
        {
            iOldSec3= iCurrentSec;
#ifdef DRIVE_COM1
#ifdef DEBUG_MAIN
            printf("2*****getDownDriver1(1)=%d\n",getDownDriver1());
#endif
            if(getDownDriver1()==0)
            {
                setDownDriver1(2);
                usleep(100*1000);
#ifdef DEBUG_MAIN
                printf("3*****getDownDriver1(1)=%d\n",getDownDriver1());
#endif
                upDateAllData(EN_COM1);
            }
#else
            if(getDownDriver2()==0)
            {
                if(getDownDriver2()==0)
                    setDownDriver2(2);
                usleep(100*1000);
                upDateAllData(EN_COM2);
                setDownDriver2(0);
            }
#endif
        }

#ifdef DEBUG_MAIN
        printf("Shakehand  =%d****iCurrentSec - iOldSec4 =%d\n",SerialInfo->iHandTime * 60,iCurrentSec - iOldSec4);
#endif
        if (iCurrentSec - iOldSec4 > SerialInfo->iHandTime * 60)//重新扫描设备并添加周期任务
        {
            iOldSec4 = iCurrentSec;
            drive_data[EN_COM1].deviceInfo[i].bFlagTask = false;
            drive_data[EN_COM1].deviceInfo[i].bHand = false;
            drive_data[EN_COM1].deviceInfo[i].bReceive = false;
            drive_data[EN_COM1].deviceInfo[i].bSaveHand = false;
#ifdef DRIVE_COM1
            for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
            {
                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun;
                taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                taskItem.m_enTaskType = TT_1TempRead;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = SerialInfo->iResend;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
                taskItem.m_ulDataNum = 1;
                taskItem.m_ulDeviceAddr = drive_data[EN_COM1].deviceInfo[i].iDeviceID;
                taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
            }
#else
            iOldSec4 = iCurrentSec;
            drive_data[EN_COM1].deviceInfo[i].bFlagTask = false;
            drive_data[EN_COM1].deviceInfo[i].bHand = false;
            drive_data[EN_COM1].deviceInfo[i].bReceive = false;
            drive_data[EN_COM1].deviceInfo[i].bSaveHand = false;
            for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
            {
                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun;
                taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                taskItem.m_enTaskType = TT_1TempRead;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = SerialInfo->iResend;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = SerialInfo->iHandAddr;
                taskItem.m_ulDataNum = 1;
                taskItem.m_ulDeviceAddr = drive_data[EN_COM2].deviceInfo[i].iDeviceID;
                taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                taskItem.m_ulTaskID = ((taskItem.m_ulDataNum<< 8) & 0xFF00) + 1;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
            }
#endif
        }

        //将驱动中所有设备变化的值保存到共享内存中
     //   upDateChangeData(EN_COM1);
#ifdef DRIVE_COM1
        if (bFlagUpInit)
        {
 #ifdef DEBUG_MAIN
            printf("###getNetworkFlag() = %d\n",getNetworkFlag());
#endif
            upDateChangeData(EN_COM1);
            usleep(100*1000);
            int NetFlag = getNetworkFlat();
            if ( NetFlag > 0 )
            {
 #ifdef DEBUG_MAIN
                printf("###2getNetworkFlat() = %d\n",getNetworkFlat());
 #endif
                upDateChangeData(EN_COM1);
            }
            else
            {
                if(getDownDriver1() == 0)
                {
                    if(iCurrentSec - iOldSec5 >= SerialInfo->iHisDateSave*60)
                    {
                        iOldSec5 = iCurrentSec;
                        setDownDriver1(2);
                        usleep(100*1000);
                        upDateAllData(EN_COM1);
                    }
                }
            }
        }
#else
        if (bFlagUpInit)
        {
            if (getNetworkFlat())
            {
                upDateChangeData(EN_COM2);
            }
            else
            {
                if(iCurrentSec - iOldSec5 >= SerialInfo->iHisDateSave)
                {
                    iOldSec5 = iCurrentSec;
                    setDownDriver1(2);
                    usleep(100*1000);
#ifdef DEBUG_MAIN
                    printf("getNetworkFlat = %d***upDateAllData***getDownDriver1() = %d\n",getNetworkFlat(),getDownDriver1());
#endif
                    upDateAllData(EN_COM2);
                }
            }
        }
#endif

        iCountsTest++;
        usleep(1000 * 200);
    }
    return 0;
}

//从共享内存中获取下发指令，并解析执行
void getDownCmdAndAction(EN_DRIVE_NUM_VALUE EN_VALUE, EN_DRIVE_NUM EN_NUM, char* cCmdValue)
{
#ifdef DEBUG_MAIN
    printf("getDownDriver1() ******= %d\n",getDownDriver1());
    printf("getDownDriver3() ******= %d\n",getDownDriver3());
#endif
#ifdef DRIVE_COM1
    if (EN_COM1 == EN_NUM)
    {
        if(2 == getDownDriver1())
        {
#ifdef DEBUG_MAIN
            printf("getDownDriver1() 22222= %d\n",getDownDriver1());
            printf("getDownCmdAndAction2222\n");
#endif
            char downCmd[DOWD_CMD_LENGTH_MAX]={0};
            memset(downCmd, 0, DOWD_CMD_LENGTH_MAX);
            char downCmd_test[DOWD_CMD_LENGTH_MAX];

            memcpy(downCmd, getDownCmdCom1(),strlen(getDownCmdCom1()));
            clrDownCmdCom1();
            memcpy(downCmd_test, getDownCmdCom1(),strlen(getDownCmdCom1()));
#ifdef DEBUG_MAIN
            printf("downCmd = %s, iDownLen = %d \n", downCmd, getDownCmdLengthCom1());
            printf("downCmd_test = %s, iDownLen = %d \n", downCmd_test, getDownCmdLengthCom1());
#endif

            if ((strstr(downCmd,"UploadAll") != NULL))
            {
                upDateAllData(EN_NUM);
#ifdef DEBUG_MAIN
                printf("getDownDriver1()22222222222222 = %d\n",getDownDriver1());
#endif
                return;
            }
        }
        if (1== getDownDriver3())
        {
#ifdef DEBUG_MAIN
             printf("getDownDriver3() 1111111111= %d\n",getDownDriver3());
            printf("getDownCmdAndAction333\n");
#endif
            char downCmd[DOWD_CMD_LENGTH_MAX];
            memcpy(downCmd, getDownCmdCom1(),strlen(getDownCmdCom1()));
            clrDownCmdCom2();
//#ifdef DEBUG_MAIN
            printf("downCmd = %s, iDownLen = %d \n", downCmd, getDownCmdLengthCom1());
//#endif
            char *cmd_data = NULL;
            int cmd_length = 0;

            char* token = strtok( downCmd, "\"");
            if (token == NULL)
            {
                printf("cmd token  is error!\n");
                setDownDriver3(0);
                return 0;
            }

            int i = 0;
            while( token != NULL )
            {
                i++;
                token = strtok( NULL, "\"");
                if (i == 1)
                {
                    if (strcmp(token, cCmdValue) != 0)//是驱动1的指令
                    {

#ifdef DEBUG_MAIN
                        printf("modbus command type is error!\n");
#endif
                        setDownDriver3(0);
                        return 0;
                    }
                }
                else if (i == 3)
                {
                    cmd_data = token;
                    break;
                }
            }

            if (strstr(cmd_data, "++") == NULL)//只有单条下发指令
            {
                //添加下发任务
                printf("it is excute cmd  **   cmd_data=%s\n",cmd_data);
                unsigned char cmdData[40];
                memset(cmdData, 0 , 40);
                int iRet = strToHex(cmd_data, cmdData);
                  printf("111111\n");
                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun;
                taskItem.m_ucCommand = SERIAL_COMMAND_WRITE_MULTI;
                taskItem.m_enTaskType = TT_0TempWrite;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = SerialInfo->iResend;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                taskItem.m_ulDataNum = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);
                taskItem.m_ulDeviceAddr = cmdData[0];
                taskItem.m_usTaskSpan = 0;
                int i = 0;
                //printf("222222\n");
                for(i; i < taskItem.m_ulDataNum; i++)
                {
                    //printf("iiiiiiiiiii = %d\n",i);
                taskItem.m_usDataBuf[i] = (USHORT)(((cmdData[7+i*2] << 8) & 0xFF00) + (cmdData[8+i*2] & 0xFF));
                printf("taskItem.m_usDataBuf[i]=%x\n",taskItem.m_usDataBuf[i]);
                }
               // printf("333333\n");

                taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000) +  ((taskItem.m_ulDataNum<< 8) & 0xFF00) + taskItem.m_ulDeviceAddr;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
            }
            else
            {
                token = strtok( cmd_data, "++");
                i = 0;
                while( token != NULL )
                {
#ifdef DEBUG_MAIN
                    printf( "token=%s\c", token );
#endif
//                    cmd_length = strlen(token);
//                    if (cmd_length != 16)
//                    {
//                        printf("++ modbus command error!");
//                       // setDownDriver1(0);
//                        return 0;
//                    }

                    //添加下发任务
                    unsigned char cmdData[40];
                    memset(cmdData, 0 , 40);
                    int iRet = strToHex(token, cmdData);

                    tagTaskItem taskItem;
                    memset(&taskItem, 0 , sizeof(taskItem));
                    taskItem.m_pFunTaskCallback = receiveFun;
                    taskItem.m_ucCommand = SERIAL_COMMAND_WRITE_MULTI;
                    taskItem.m_enTaskType = TT_0TempWrite;
                    taskItem.m_enTaskState = TS_Ready;
                    taskItem.m_ucTaskRetry = SerialInfo->iResend;
                    taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                    taskItem.m_ulDataAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                    taskItem.m_ulDataNum = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);
                    taskItem.m_ulDeviceAddr = cmdData[0];
                    taskItem.m_usTaskSpan = 0;
                    int i = 0;
                    for(i; i < taskItem.m_ulDataNum; i++)
                    {
                    taskItem.m_usDataBuf[i] = (USHORT)(((cmdData[7+i*2] << 8) & 0xFF00) + (cmdData[8+i*2] & 0xFF));
                    }
                    taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000) +  ((taskItem.m_ulDataNum<< 8) & 0xFF00) + taskItem.m_ulDeviceAddr;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                    UserAddTask(taskItem);

                    token = strtok( NULL, "++");
                    i++;
                }
            }
            setDownDriver3(0);
            return;
        }
    }

#else
 //   {
        if(2 == getDownDriver2())
        {
            printf("getDownCmdAndAction2222\n");
            char downCmd[DOWD_CMD_LENGTH_MAX];
            memcpy(downCmd, getDownCmdCom2(),strlen(getDownCmdCom2()));
            clrDownCmdCom2();
#ifdef DEBUG_MAIN
            printf("downCmd = %s, iDownLen = %d \n", downCmd, getDownCmdLengthCom1());
#endif

            if ((strstr(downCmd,"UploadAll") != NULL))
            {
                upDateAllData(EN_NUM);
                setDownDriver2(0);
                return;
            }
        }
        if (1== getDownDriver4())
        {
            char downCmd[DOWD_CMD_LENGTH_MAX];
            memcpy(downCmd, getDownCmdCom2(),strlen(getDownCmdCom2()));

#ifdef DEBUG_MAIN
            printf("downCmd = %s, iDownLen = %d \n", downCmd, getDownCmdLengthCom2());
#endif

            char *cmd_data = NULL;
            int cmd_length = 0;

            char* token = strtok( downCmd, "\"");
            if (token == NULL)
            {
                printf("cmd token  is error!\n");
                setDownDriver4(0);
                return 0;
            }

            int i = 0;
            while( token != NULL )
            {
                i++;
                token = strtok( NULL, "\"");
                if (i == 1)
                {
                    if (strcmp(token, cCmdValue) != 0)//是驱动1的指令
                    {

#ifdef DEBUG_MAIN
                        printf("modbus command type is error!\n");
#endif
                        setDownDriver4(0);
                        return 0;
                    }
                }
                else if (i == 3)
                {
                    cmd_data = token;
                    break;
                }
            }

            if (strstr(cmd_data, "++") == NULL)//只有单条下发指令
            {
//                cmd_length = strlen(cmd_data);
//                if (cmd_length != 16)
//                {
//#ifdef DEBUG_MAIN
//                    printf("modbus command error!");
//#endif
//                    setDownDriver2(0);
//                    return 0;
//                }
                //添加下发任务
                unsigned char cmdData[40];
                memset(cmdData, 0 , 40);
                int iRet = strToHex(cmd_data, cmdData);

                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFun;
                taskItem.m_ucCommand = SERIAL_COMMAND_WRITE_MULTI;
                taskItem.m_enTaskType = TT_0TempWrite;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = SerialInfo->iResend;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                taskItem.m_ulDataNum = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);
                taskItem.m_ulDeviceAddr = cmdData[0];
                taskItem.m_usTaskSpan = 0;
                int i = 0;
                for(i; i < taskItem.m_ulDataNum; i++)
                {
                taskItem.m_usDataBuf[i] = (USHORT)(((cmdData[7+i*2] << 8) & 0xFF00) + (cmdData[8+i*2] & 0xFF));
                }
                taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000) +  ((taskItem.m_ulDataNum<< 8) & 0xFF00) + taskItem.m_ulDeviceAddr;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                UserAddTask(taskItem);
            }
            else
            {
                token = strtok( cmd_data, "++");
                i = 0;
                while( token != NULL )
                {
#ifdef DEBUG_MAIN
                    printf( "token=%s\c", token );
#endif
//                    cmd_length = strlen(token);
//                    if (cmd_length != 16)
//                    {
//                        printf("++ modbus command error!");
//                        setDownDriver2(0);
//                        return 0;
//                    }

                    //添加下发任务
                    unsigned char cmdData[40];
                    memset(cmdData, 0 , 40);
                    int iRet = strToHex(token, cmdData);

                    tagTaskItem taskItem;
                    memset(&taskItem, 0 , sizeof(taskItem));
                    taskItem.m_pFunTaskCallback = receiveFun;
                    taskItem.m_ucCommand = SERIAL_COMMAND_WRITE_MULTI;
                    taskItem.m_enTaskType = TT_0TempWrite;
                    taskItem.m_enTaskState = TS_Ready;
                    taskItem.m_ucTaskRetry = SerialInfo->iResend;
                    taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                    taskItem.m_ulDataAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                    taskItem.m_ulDataNum = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);
                    taskItem.m_ulDeviceAddr = cmdData[0];
                    taskItem.m_usTaskSpan = 0;
                    int i = 0;
                    for(i; i < taskItem.m_ulDataNum; i++)
                    {
                    taskItem.m_usDataBuf[i] = (USHORT)(((cmdData[7+i*2] << 8) & 0xFF00) + (cmdData[8+i*2] & 0xFF));
                    }
                    taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000) +  ((taskItem.m_ulDataNum<< 8) & 0xFF00) + taskItem.m_ulDeviceAddr;//高16位地址,低16位中的高8位是读取个数，低8位是ID号
                    UserAddTask(taskItem);

                    token = strtok( NULL, "++");
                    i++;
                }
            }
            setDownDriver4(0);
            return;
        }
#endif// }
}

//传入参数驱动号,将不同驱动的所有数据存入共享内存区
void upDateAllData(EN_DRIVE_NUM EN_NUM)
{
    if(!bModbusflag)//如果485不通则不上传数据
        return;

    StructDriverUpData up_drive_data;
    memset(&up_drive_data, 0, sizeof(up_drive_data));
    up_drive_data.ideviceNum = drive_data[EN_NUM].iDeviceNum;//有几台机器
    setDeviceNumByDriverIndex(EN_NUM, drive_data[EN_NUM].iDeviceNum);//设置驱动有几台设备
    int i = 0;
    //printf("upDateAllData44444!!\n");
    for (i = 0; i < up_drive_data.ideviceNum; i++)
    {
//#ifdef DEBUG_MAIN
        printf("upDateAllData start******getDownDriver1() = %d!!\n",getDownDriver1());
//#endif
        if(!drive_data[EN_COM1].deviceInfo[i].bHand)
            continue;
        int j = 0;
        up_drive_data.upDeviceData[i].iUpAddrNum = 0;
        for (j = 0; j < drive_data[EN_NUM].deviceInfo[i].iDeviceNum; j++)//每个驱动挂多少设备
        {
            //                drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue = iCountsTest + EN_NUM * 5000;//该处赋值是测试用,发行版注释掉就可以了
            drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarOldValue
                    = drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue;

            up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].iAddr
                    = drive_data[EN_NUM].deviceInfo[i].deviceData[j].usVarAddr;

            up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].usValue
                    = drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue;

            #ifdef DEBUG_MAIN
            printf("EN_NUM = COM%d ,i = %d, up_drive_data.upDeviceData[i].iUpAddrNum = %d, adddr = %d, value = %d \n",
                   EN_NUM,i, up_drive_data.upDeviceData[i].iUpAddrNum, up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].iAddr,
                    up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].usValue);
          #endif
            up_drive_data.upDeviceData[i].iUpAddrNum++;
        }
       #ifdef DEBUG_MAIN
        printf("RAM**up_drive_data.upDeviceData[i].iUpAddrNum = %d\n",up_drive_data.upDeviceData[i].iUpAddrNum);
#endif
        memcpy(up_drive_data.upDeviceData[i].cDeviceTypes,"a01",sizeof("a01"));
        up_drive_data.upDeviceData[i].iDeviceID = drive_data[EN_NUM].deviceInfo[i].iDeviceID;

        while (getIsAllUpdate() > 0)
        {
          //  if(!getNetworkFlat())
          //      break;
            usleep(200*1000);
        }
        set_device_up_data(EN_NUM,i,&up_drive_data.upDeviceData[i]);
        usleep(200*1000);
    }
}

//传入参数驱动号,将不同驱动的变化数据存入共享内存区
void upDateChangeData(EN_DRIVE_NUM EN_NUM)
{
 #ifdef DEBUG_MAIN
    printf("upDateChangeData start****!!\n");
#endif
    StructDriverUpData up_drive_data;
    memset(&up_drive_data, 0, sizeof(up_drive_data));
    up_drive_data.ideviceNum = drive_data[EN_NUM].iDeviceNum;//有几台机器
    setDeviceNumByDriverIndex(EN_NUM, drive_data[EN_NUM].iDeviceNum);//设置驱动有几台设备
    bool bFlag = false;
    int i = 0;
#ifdef DRIVE_COM1
    if(getDownDriver1() == 2 && up_drive_data.ideviceNum == 8)
        return;
#else
    if(getDownDriver3() == 2 && up_drive_data.ideviceNum == 8)
        setDownDriver3(0);
#endif
    for (i = 0; i < up_drive_data.ideviceNum; i++)
    {
        int j = 0;
        up_drive_data.upDeviceData[i].iUpAddrNum = 0;
        for (j = 0; j < drive_data[EN_NUM].deviceInfo[i].iDeviceNum; j++)//每个驱动挂多少设备
        {
            //                drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue = iCountsTest + EN_NUM * 5000;//该处赋值是测试用,发行版注释掉就可以了
            if (drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue
                    != drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarOldValue)
            {
                drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarOldValue
                        = drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue;

                up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].iAddr
                        = drive_data[EN_NUM].deviceInfo[i].deviceData[j].usVarAddr;

                up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].usValue
                        = drive_data[EN_NUM].deviceInfo[i].deviceData[j].sVarValue;

#ifdef DEBUG_MAIN
                printf("EN_NUM = COM%d ,i = %d, up_drive_data.upDeviceData[i].iUpAddrNum = %d, adddr = %d, value = %d \n",
                       EN_NUM,i, up_drive_data.upDeviceData[i].iUpAddrNum, up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].iAddr,
                        up_drive_data.upDeviceData[i].unitAddrValue[up_drive_data.upDeviceData[i].iUpAddrNum].usValue);
#endif
                up_drive_data.upDeviceData[i].iUpAddrNum++;
                bFlag = true;
            }
        }
        memcpy(up_drive_data.upDeviceData[i].cDeviceTypes,"a01",sizeof("a01"));
        up_drive_data.upDeviceData[i].iDeviceID = drive_data[EN_NUM].deviceInfo[i].iDeviceID;
        if (bFlag)
        {
            while (getIsAllUpdate() > 0)
            {
               // if(!getNetworkFlat())
               //     break;
                usleep(200*1000);
                //printf("upDateAllData6666!!\n");
            }
            set_device_up_data(EN_NUM,i,&up_drive_data.upDeviceData[i]);
             //printf("upDateChangeData end****!!\n");
        }

    }
}

void receiveFun1(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                UCHAR enReason )
{
    if ( ERR_None ==  enReason)
    {
        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
#ifdef DEBUG_MAIN
            printf("hand success ulDeviceAddr = %d \n", ulDeviceAddr);
#endif
            int i = 0;
#ifdef DRIVE_COM1
            for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
            {
                if (drive_data[EN_COM1].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM1].deviceInfo[i].bHand = true;
                    iDevFailedNum =1;
#ifdef DEBUG_MAIN
                    printf("echo '1' > /sys/class/gpio/gpio1/value\n");
#endif
                    system("echo '1' > /sys/class/gpio/gpio1/value");
                    bModbusflag = true;
                    break;
                }
            }

            if (!drive_data[EN_COM1].deviceInfo[i].bSaveHand)
            {
                drive_data[EN_COM1].deviceInfo[i].bSaveHand = true;
                //将握手地址中的数据也保存吧,保存一次就好，很耗费处理时间
                SaveData(EN_COM1, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);
            }
#ifdef DEBUG_MAIN
            printf("drive_data[EN_COM1].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM1].deviceInfo[i].iDeviceID);
#endif
#else
            for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
            {
                if (drive_data[EN_COM2].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM2].deviceInfo[i].bHand = true;                    
                    system("echo '1' > /sys/class/gpio/gpio26/value");
                    break;
                }
            }

            if (!drive_data[EN_COM2].deviceInfo[i].bSaveHand)
            {
                drive_data[EN_COM2].deviceInfo[i].bSaveHand = true;
                //将握手地址中的数据也保存吧,保存一次就好，很耗费处理时间
                SaveData(EN_COM2, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);
            }

#ifdef DEBUG_MAIN
            printf("drive_data[EN_COM2].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM2].deviceInfo[i].iDeviceID);
#endif
#endif

        }
    }
    else if (ERR_TimeOut == enReason)
    {

#ifdef DEBUG_MAIN
        printf("TimeOut::ulDeviceAddr = %d \n", ulDeviceAddr);
#endif

        int i = 0;
#ifdef DRIVE_COM1
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
        {
            if (drive_data[EN_COM1].deviceInfo[i].iDeviceID == ulDeviceAddr)
            {
                iDevFailedNum++;
                printf("iDevFailedNum = %d\n",iDevFailedNum);
                break;
            }
        }

#ifdef DEBUG_MAIN
        printf("TimeOut::drive_data[EN_COM1].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM1].deviceInfo[i].iDeviceID);
#endif

        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
            drive_data[EN_COM1].deviceInfo[i].bHand = false;
            drive_data[EN_COM1].deviceInfo[i].bSaveHand = false;
        }
        else
        {
            drive_data[EN_COM1].deviceInfo[i].bReceive = false;
        }

        if ((drive_data[EN_COM1].deviceInfo[i].bFlagTask)
                && (!drive_data[EN_COM1].deviceInfo[i].bHand)
                && (!drive_data[EN_COM1].deviceInfo[i].bReceive))
        {
            //删除普通的读任务
            tagUserAction action;
            action.addr = ulDeviceAddr;
            action.way = MF_Addr_AllId;
            action.para = PA_TaskState;
            UserModifyTask(action, TS_Delete);
        }
#else
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
        {
            if (drive_data[EN_COM2].deviceInfo[i].iDeviceID == ulDeviceAddr)
            {
                break;
            }
        }

#ifdef DEBUG_MAIN
        printf("TimeOut::drive_data[EN_COM2].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM2].deviceInfo[i].iDeviceID);
#endif

        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
            drive_data[EN_COM2].deviceInfo[i].bHand = false;
            drive_data[EN_COM2].deviceInfo[i].bSaveHand = false;
        }
        else
        {
            drive_data[EN_COM2].deviceInfo[i].bReceive = false;
        }

        if ((drive_data[EN_COM2].deviceInfo[i].bFlagTask)
                && (!drive_data[EN_COM2].deviceInfo[i].bHand)
                && (!drive_data[EN_COM2].deviceInfo[i].bReceive))
        {
            //删除普通的读任务
            tagUserAction action;
            action.addr = ulDeviceAddr;
            action.way = MF_Addr_AllId;
            action.para = PA_TaskState;
            UserModifyTask(action, TS_Delete);
        }
#endif
    }
}

void receiveFun(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                UCHAR enReason )
{
    if ( ERR_None ==  enReason)
    {
        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
//#ifdef DEBUG_MAIN
            printf("hand success ulDeviceAddr = %d \n", ulDeviceAddr);
//#endif
            int i = 0;
#ifdef DRIVE_COM1
            for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
            {
                if (drive_data[EN_COM1].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM1].deviceInfo[i].bHand = true;
                    setiHandAddr(ulDeviceAddr);//保存在线设备最大地址

//                    iDevFailedNum =1;
//#ifdef DRIVE_COM1
//                    //printf("echo '1' > /sys/class/gpio/gpio1/value\n");
                    system("echo '1' > /sys/class/gpio/gpio1/value");
                    bModbusflag =true;
//#else
//                    system("echo '1' > /sys/class/gpio/gpio26/value");
//#endif
                    break;
                }
            }
            printf(" The max  ulDeviceAddr = %d \n", getiHandAddr());

            if (!drive_data[EN_COM1].deviceInfo[i].bSaveHand)
            {
                drive_data[EN_COM1].deviceInfo[i].bSaveHand = true;
                //将握手地址中的数据也保存吧,保存一次就好，很耗费处理时间
                SaveData(EN_COM1, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);
            }
#ifdef DEBUG_MAIN
            printf("drive_data[EN_COM1].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM1].deviceInfo[i].iDeviceID);
#endif
#else
            for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
            {
                if (drive_data[EN_COM2].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM2].deviceInfo[i].bHand = true;
                    break;
                }
            }

            if (!drive_data[EN_COM2].deviceInfo[i].bSaveHand)
            {
                drive_data[EN_COM2].deviceInfo[i].bSaveHand = true;
                //将握手地址中的数据也保存吧,保存一次就好，很耗费处理时间
                SaveData(EN_COM2, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);
            }

#ifdef DEBUG_MAIN
            printf("drive_data[EN_COM2].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM2].deviceInfo[i].iDeviceID);
#endif
#endif

#ifdef DRIVE_COM1
#ifdef SIGNAL_ADDR
            if (!drive_data[EN_COM1].deviceInfo[i].bFlagTask)
            {
                drive_data[EN_COM1].deviceInfo[i].bFlagTask = true;
                //添加采集任务
                int j = 0;
                for (j = 0; j < drive_data[EN_COM1].iDeviceNum; j++)
                {
                    if (allDeviceDataFrame[j].iDeviceID == ulDeviceAddr)
                    {
                        break;
                    }
                }

#ifdef DEBUG_MAIN
                printf("allDeviceDataFrame[j].iDeviceID = %d \n", allDeviceDataFrame[j].iDeviceID);
                printf("allDeviceDataFrame[j].iFrameNum = %d \n", allDeviceDataFrame[j].iFrameNum);
#endif

                int m = 0;
                for (m = 0; m < allDeviceDataFrame[j].iFrameNum; m++)
                {
                    tagTaskItem taskItem;
                    memset(&taskItem, 0 , sizeof(taskItem));
                    taskItem.m_pFunTaskCallback = receiveFun;
                    taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                    taskItem.m_enTaskType = TT_3PeriodRead;
                    taskItem.m_enTaskState = TS_Ready;
                    taskItem.m_ucTaskRetry = SerialInfo->iResend;
                    taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                    taskItem.m_ulDataAddr = allDeviceDataFrame[j].dataFrame[m].usAddrStart;
                    taskItem.m_ulDataNum = allDeviceDataFrame[j].dataFrame[m].ucDataNum;
                    taskItem.m_ulDeviceAddr = ulDeviceAddr;
                    taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                    taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                            + ((taskItem.m_ulDataNum << 8) & 0xFF00)
                            + (taskItem.m_ulDeviceAddr & 0xFF);//高16位地址,低16位中的高8位是读取个数，低8位是ID号

#ifdef DEBUG_MAIN
                    printf("allDeviceDataFrame[j].dataFrame[m].usAddrStart = %d \n", allDeviceDataFrame[j].dataFrame[m].usAddrStart);
                    printf("allDeviceDataFrame[j].dataFrame[m].ucDataNum = %d \n", allDeviceDataFrame[j].dataFrame[m].ucDataNum);
#endif

                    bool bRet = false;
                    bRet =  UserAddTask(taskItem);
                    if (!bRet)
                    {
                        printf("add tastTtem false! \n");
                    }
                    else
                    {
                        printf("add tastTtem success! \n");
                    }

#ifdef DEBUG_MAIN
                    printf("99:addr task taskItem.m_ulDeviceAddr = %lu, taskItem.m_ulDataAddr = %lu, taskItem.m_ulDataNum = %lu \n",
                           taskItem.m_ulDeviceAddr,
                           taskItem.m_ulDataAddr,
                           taskItem.m_ulDataNum);
#endif
                }
            }
#else
            if (!drive_data[EN_COM1].deviceInfo[i].bFlagTask)
            {
                drive_data[EN_COM1].deviceInfo[i].bFlagTask = true;
                //添加采集任务
                int j = 0;
                for (j = 0; j < iConfigCom1Num; j++)
                {
                    if (allDeviceConfigInfoCOM1[j].iDeviceID == ulDeviceAddr)
                    {
                        tagTaskItem taskItem;
                        memset(&taskItem, 0 , sizeof(taskItem));
                        taskItem.m_pFunTaskCallback = receiveFun;
                        taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                        taskItem.m_enTaskType = TT_3PeriodRead;
                        taskItem.m_enTaskState = TS_Ready;
                        taskItem.m_ucTaskRetry = SerialInfo->iResend;
                        taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                        taskItem.m_ulDataAddr = allDeviceConfigInfoCOM1[j].usVarAddr;
                        taskItem.m_ulDataNum = allDeviceConfigInfoCOM1[j].iNum;
                        taskItem.m_ulDeviceAddr = ulDeviceAddr;
                        taskItem.m_usTaskSpan = allDeviceConfigInfoCOM1[j].iSampleTime * 60 * 1000;
                        taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                                + ((taskItem.m_ulDataNum << 8) & 0xFF00)
                                + (taskItem.m_ulDeviceAddr & 0xFF);//高16位地址,低16位中的高8位是读取个数，低8位是ID号

                        bool bRet = false;
                        bRet =  UserAddTask(taskItem);
#ifdef DEBUG_MAIN
                        if (!bRet)
                        {
                            printf("add tastTtem false! \n");
                        }
                        else
                        {
                            printf("add tastTtem success! \n");
                        }
#endif
                    }
                }
            }
#endif
#else
#ifdef SIGNAL_ADDR
            if (!drive_data[EN_COM2].deviceInfo[i].bFlagTask)
            {
                drive_data[EN_COM2].deviceInfo[i].bFlagTask = true;
                //添加采集任务
                int j = 0;
                for (j = 0; j < drive_data[EN_COM2].iDeviceNum; j++)
                {
                    if (allDeviceDataFrame[j].iDeviceID == ulDeviceAddr)
                    {
                        break;
                    }
                }

#ifdef DEBUG_MAIN
                printf("allDeviceDataFrame[j].iDeviceID = %d \n", allDeviceDataFrame[j].iDeviceID);
                printf("allDeviceDataFrame[j].iFrameNum = %d \n", allDeviceDataFrame[j].iFrameNum);
#endif

                int m = 0;
                for (m = 0; m < allDeviceDataFrame[j].iFrameNum; m++)
                {
                    tagTaskItem taskItem;
                    memset(&taskItem, 0 , sizeof(taskItem));
                    taskItem.m_pFunTaskCallback = receiveFun;
                    taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                    taskItem.m_enTaskType = TT_3PeriodRead;
                    taskItem.m_enTaskState = TS_Ready;
                    taskItem.m_ucTaskRetry = SerialInfo->iResend;
                    taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                    taskItem.m_ulDataAddr = allDeviceDataFrame[j].dataFrame[m].usAddrStart;
                    taskItem.m_ulDataNum = allDeviceDataFrame[j].dataFrame[m].ucDataNum;
                    taskItem.m_ulDeviceAddr = ulDeviceAddr;
                    taskItem.m_usTaskSpan = SerialInfo->iTaskTime;
                    taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                            + ((taskItem.m_ulDataNum << 8) & 0xFF00)
                            + (taskItem.m_ulDeviceAddr & 0xFF);//高16位地址,低16位中的高8位是读取个数，低8位是ID号

#ifdef DEBUG_MAIN
                    printf("allDeviceDataFrame[j].dataFrame[m].usAddrStart = %d \n", allDeviceDataFrame[j].dataFrame[m].usAddrStart);
                    printf("allDeviceDataFrame[j].dataFrame[m].ucDataNum = %d \n", allDeviceDataFrame[j].dataFrame[m].ucDataNum);
#endif
                    bool bRet = false;
                    bRet =  UserAddTask(taskItem);
                    if (!bRet)
                    {
                        printf("add tastTtem false! \n");
                    }
                    else
                    {
                        printf("add tastTtem success! \n");
                    }

#ifdef DEBUG_MAIN
                    printf("99:addr task taskItem.m_ulDeviceAddr = %lu, taskItem.m_ulDataAddr = %lu, taskItem.m_ulDataNum = %lu \n",
                           taskItem.m_ulDeviceAddr,
                           taskItem.m_ulDataAddr,
                           taskItem.m_ulDataNum);
#endif
                }
            }
#else
            if (!drive_data[EN_COM2].deviceInfo[i].bFlagTask)
            {
                drive_data[EN_COM2].deviceInfo[i].bFlagTask = true;
                //添加采集任务
                int j = 0;
                for (j = 0; j < iConfigCom2Num; j++)
                {
                    if (allDeviceConfigInfoCOM2[j].iDeviceID == ulDeviceAddr)
                    {
                        tagTaskItem taskItem;
                        memset(&taskItem, 0 , sizeof(taskItem));
                        taskItem.m_pFunTaskCallback = receiveFun;
                        taskItem.m_ucCommand = SERIAL_COMMAND_READ;
                        taskItem.m_enTaskType = TT_3PeriodRead;
                        taskItem.m_enTaskState = TS_Ready;
                        taskItem.m_ucTaskRetry = SerialInfo->iResend;
                        taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                        taskItem.m_ulDataAddr = allDeviceConfigInfoCOM2[j].usVarAddr;
                        taskItem.m_ulDataNum = allDeviceConfigInfoCOM2[j].iNum;
                        taskItem.m_ulDeviceAddr = ulDeviceAddr;
                        taskItem.m_usTaskSpan = allDeviceConfigInfoCOM2[j].iSampleTime * 60 * 1000;
                        taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                                + ((taskItem.m_ulDataNum << 8) & 0xFF00)
                                + (taskItem.m_ulDeviceAddr & 0xFF);//高16位地址,低16位中的高8位是读取个数，低8位是ID号

                        bool bRet = false;
                        bRet =  UserAddTask(taskItem);
#ifdef DEBUG_MAIN
                        if (!bRet)
                        {
                            printf("add tastTtem false! \n");
                        }
                        else
                        {
                            printf("add tastTtem success! \n");
                        }
#endif
                    }
                }
            }
#endif
#endif
        }
        else //采集的数据保存
        {
            int i = 0;
#ifdef DRIVE_COM1
            for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
            {
                if (drive_data[EN_COM1].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM1].deviceInfo[i].bReceive = true;
                    break;
                }
            }
            SaveData(EN_COM1, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);   //将读到的数据存储在结构体数据中
#else
            for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
            {
                if (drive_data[EN_COM2].deviceInfo[i].iDeviceID == ulDeviceAddr)
                {
                    drive_data[EN_COM2].deviceInfo[i].bReceive = true;
                    break;
                }
            }
            SaveData(EN_COM2, pusDataBuf, ulDataAddr, ulDataNum, ulDeviceAddr);   //将读到的数据存储在结构体数据中
#endif

#ifdef DEBUG_MAIN
            printf("ulDataAddr = %lu, ulDeviceAddr = %lu \n", ulDataAddr, ulDeviceAddr);
            for(i = 0; i < ulDataNum; i++)
            {
                printf("%d ", *(pusDataBuf + i));
            }
            printf("\n");
#endif
        }
    }
    else if (ERR_TimeOut == enReason)
    {

#ifdef DEBUG_MAIN
        printf("TimeOut::ulDeviceAddr = %d \n", ulDeviceAddr);
#endif

        int i = 0;
#ifdef DRIVE_COM1
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
        {
            if (drive_data[EN_COM1].deviceInfo[i].iDeviceID == ulDeviceAddr)
            {
               // iDevFailedNum++;
               // printf("iDevFailedNum = %d\n",iDevFailedNum);
                break;
            }
        }

#ifdef DEBUG_MAIN
        printf("TimeOut::drive_data[EN_COM1].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM1].deviceInfo[i].iDeviceID);
#endif

        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
            drive_data[EN_COM1].deviceInfo[i].bHand = false;
            drive_data[EN_COM1].deviceInfo[i].bSaveHand = false;
        }
        else
        {
            drive_data[EN_COM1].deviceInfo[i].bReceive = false;
        }

        if ((drive_data[EN_COM1].deviceInfo[i].bFlagTask)
                && (!drive_data[EN_COM1].deviceInfo[i].bHand)
                && (!drive_data[EN_COM1].deviceInfo[i].bReceive))
        {
            //删除普通的读任务
            tagUserAction action;
            action.addr = ulDeviceAddr;
            action.way = MF_Addr_AllId;
            action.para = PA_TaskState;
            UserModifyTask(action, TS_Delete);
        }
#else
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
        {
            if (drive_data[EN_COM2].deviceInfo[i].iDeviceID == ulDeviceAddr)
            {
                break;
            }
        }

#ifdef DEBUG_MAIN
        printf("TimeOut::drive_data[EN_COM2].deviceInfo[i].iDeviceID = %d \n", drive_data[EN_COM2].deviceInfo[i].iDeviceID);
#endif

        if ((SerialInfo->iHandAddr == ulDataAddr) && (1 == ulDataNum)) //握手协议
        {
            drive_data[EN_COM2].deviceInfo[i].bHand = false;
            drive_data[EN_COM2].deviceInfo[i].bSaveHand = false;
        }
        else
        {
            drive_data[EN_COM2].deviceInfo[i].bReceive = false;
        }

        if ((drive_data[EN_COM2].deviceInfo[i].bFlagTask)
                && (!drive_data[EN_COM2].deviceInfo[i].bHand)
                && (!drive_data[EN_COM2].deviceInfo[i].bReceive))
        {
            //删除普通的读任务
            tagUserAction action;
            action.addr = ulDeviceAddr;
            action.way = MF_Addr_AllId;
            action.para = PA_TaskState;
            UserModifyTask(action, TS_Delete);
        }
#endif
    }
}
//根据驱动号, 设备号和地址，地址个数保存值
int SaveData(EN_DRIVE_NUM EN_NUM, USHORT* buffer, unsigned short addr, unsigned char num , ULONG ID)
{
    int i = 0;
    for (i = 0; i < drive_data[EN_NUM].iDeviceNum; i++)
    {
        if (drive_data[EN_NUM].deviceInfo[i].iDeviceID == ID)
        {
            break;
        }
    }

    int j = 0;
    for (j = 0; j < drive_data[EN_NUM].deviceInfo[i].iDeviceNum; j++)
    {
        if (drive_data[EN_NUM].deviceInfo[i].deviceData[j].usVarAddr == addr)
        {
            int m = 0;
            for (m = 0; m < num; m++ )
            {
                if (drive_data[EN_NUM].deviceInfo[i].deviceData[j + m].usVarAddr <= (addr + num - 1))
                {
                    drive_data[EN_NUM].deviceInfo[i].deviceData[j + m].sVarValue = *(buffer + drive_data[EN_NUM].deviceInfo[i].deviceData[j + m].usVarAddr - addr);
                }
                else
                {
                    return 0;
                }
            }
        }
    }
}
