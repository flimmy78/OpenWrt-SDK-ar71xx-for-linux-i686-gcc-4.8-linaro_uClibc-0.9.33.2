#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcpy()
#include<ctype.h>
#include "ramrt.h"
#include <errno.h>
#include "mosquitto.h"
#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include "dataroutedb.h"
#include <unistd.h>//判断文件是否存在函数access()
#include "sqlite3.h"
#include <dirent.h>//读取给定目录下的文件

//#define MQTT_CMD
#ifdef MQTT_CMD
#include <sys/wait.h>
#include <sys/types.h>
#endif

#define false 0
#define true 1
#define MQTT_XML_PATH "/etc/config/broker"
#define SERIAL_XML_PATH "/etc/config/comm"
#define MSGMODE_NONE 0
#define MSGMODE_CMD 1
#define MSGMODE_STDIN_LINE 2
#define MSGMODE_STDIN_FILE 3
#define MSGMODE_FILE 4
#define MSGMODE_NULL 5

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2
#define PATH "/usr/bin/dataroute"
#define PATHDB "/usr/bin/dataroute/DB"
#define USB_MAX  2000000
#define DBFILESIZE  100
#define LOCALDBNUM  2
#define USBDBNUM  1024
#define ADDRNUM  285
#define DRIVE_COM1
#define NUM_MAX 1
#define NUM_MIN 0
#define NUM_SECMIN 2
#define SORT_INCR 1
#define SORT_DESC 0


/* Global variables for use in callbacks. See sub_client.c for an example of
 * using a struct to hold variables for use in callbacks. */
static char *topic = NULL;
static char *message = NULL;
static long msglen = 0;
static int qos = 0;
static int retain = 0;
static int mode = MSGMODE_NONE;
static int status = STATUS_CONNECTING;
static int mid_sent = 0;
static int last_mid = -1;
static int last_mid_sent = -1;
static bool connected = true;
static char *username = NULL;
static char *password = NULL;
static bool disconnect_sent = false;
static bool quiet = false;
int iUpdateLocalIndex = 0;//更新本地数据库索引
int iInsertLocalIndex = 0;  //插入本地数据库索引
int iUpdateUSBIndex = 0;//更新USB数据库索引
int iInsertUSBIndex = 0;  //插入USB数据库索引
int iInsertUSBIndexBackUp;
int iInsertLocalIndexBackUp;
bool iInsertIndexFlag = false;
int iHistoryIndex = 0;//读取历史数据库索引
int iLocalDBIndex = 0;//USB文件索引
long iUsbDBIndex = 0;//本地文件索引
int iUSBMax = 0;
int iMvIndex = 1;
static bool bConnectFlag = false;
static bool bHistoryFlag = false;
char *mntpath = NULL;
char cUSBDbPath[LASTMAX];
bool bHistoryFileRendFlag = true;
bool bUsbFlag = false;
bool bSaveAndSend = true;
char cDevPath[CMDMAX];
char cMntPath[CMDMAX];
char cMkdirCmd[CMDMAX];
char cMountCmd[CMDMAX];
int i_count_mqtt = 0;

char *id = NULL;
int i;
char *host = "localhost";
int port = 1883;
char *bind_address = NULL;
int keepalive = 15;
char buf[1024];
bool debug = false;
struct mosquitto *mosq = NULL;
int rc;
char hostname[256];
int len;
unsigned int max_inflight = 20;
bool use_srv = false;
char err[1024];
int send_fault_counts = 0;
#ifdef MQTT_CMD
void MqttSendMsg(StructDriverUpData * upData);
#else
int MqttSendMsg(StructUpModbusData * upData, int device);
void SendHistoryData();
int GetFileMaxAndMinNum(char* path,int NumType);
int GetFileNum(char* path);
long getfileindex(char* IndexName);
int mountdevice();
void usbDBmanage();
void localDBmanage();
char* getusbdevice(char* path);
void destroy_mqtt();
#endif

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str);
void print_usage(void);
int mqtt_init();
long getlocalhostip();

int main(int argc, char *argv[])
{
    int rc_loop=0;
    long ulIP = getlocalhostip();
    char addr[MAX_NAME_LEN] = {0};
    sprintf(addr, "%d.%d.%d.%d",
           (ulIP >> 24) & 0xFF,
           (ulIP >> 16) & 0xFF,
           (ulIP >> 8) & 0xFF,
           ulIP & 0xFF);

    if (!open_ramrt())
    {
        printf("open ramrt error!\n");
    }
    else
        printf("open ramrt successful !\n");

    /* 读取配置文件 */

    if(ReadMqttConfig(MQTT_XML_PATH) == -1)
        return -1;
    if(ReadSerialConfig(SERIAL_XML_PATH) == -1)
        return -1;
    strcpy(mqttInfo->strAddress, "14.215.130.180");
    mqttInfo->iPort =1883;
    strcpy(mqttInfo->strTopic , "r");
    strcpy(mqttInfo->strUseName ,  "invtGTerm");
    strcpy(mqttInfo->strPwd ,  "in2015v11t11");
    mqttInfo->iQos = 0;//不在配置表中读取，直接赋值为零
    mqttInfo->iKeepalive = 15;//不在配置表中读取，直接赋值为6000
    mqttInfo->ulTimeout = 10000;//不在配置表中读取，直接赋值为10000
    memcpy(mqttInfo->strBindAddtess, addr, MAX_NAME_LEN);
#ifdef MQTT_DEBUG
    printf("mqttInfo->strAddress = %s\n",mqttInfo->strAddress);
    printf("mqttInfo->iPort = %d\n",mqttInfo->iPort);
    printf("mqttInfo->strTopic = %s\n",mqttInfo->strTopic);
    printf("mqttInfo->strUseName = %s\n",mqttInfo->strUseName);
    printf("mqttInfo->strPwd = %s\n",mqttInfo->strPwd);
    printf("mqttInfo->strClientID = %s\n",mqttInfo->strClientID);
    printf("mqttInfo->strBindAddtess = %s\n",mqttInfo->strBindAddtess);
    printf("mqttInfo->iKeepalive = %d\n",mqttInfo->iKeepalive);
    printf("jsonInfo->iType = %d\n",jsonInfo->iType);
    printf("jsonInfo->strAddrKey = %s\n",jsonInfo->strAddrKey);
#endif

#ifndef MQTT_CMD
    rc = mqtt_init();
    if (rc)
    {
        printf("mqtt_init error!\n");
        system("echo '1' > /sys/class/gpio/gpio27/value");
       // return 1;
    }
    else
    {
        printf("mqtt_init success!**** rc  = %d\n",rc);
        system("echo '0' > /sys/class/gpio/gpio27/value");
        setNetworkFlat(1);
    }
#endif
    int counts = 0;
    int i_count = 0;
    mosquitto_loop_start(mosq);
    while(1)
    {
        printf("***************\n");
        int iMountFlag = mountdevice();
        if( iMountFlag == 0 )
        {
            printf("it is mounted USB and iUsbDBIndex=%d,mntpath=%s!!\n",iUsbDBIndex,mntpath);
            usbDBmanage();
            bUsbFlag = true;
            iInsertLocalIndex=0;
        }
        else
        {
            //printf("it is mounted Local!!\n");
             bUsbFlag = false;
             iInsertUSBIndex=0;
        }
        localDBmanage();
        int iDrive = getDeviceNumByDriverIndex(0);
        printf("device num = %d \n", iDrive);
        int i = 0;
        StructUpModbusData * upData;
        for (i = 0; i <  iDrive; i++)
        {
           // printf("mntpath = %s  11\n",mntpath);
          upData  = get_device_up_data(0,i);
          MqttSendMsg(upData,i);
          usleep(250*1000);
            //printf("mntpath = %s  22\n",mntpath);
        }
        SendHistoryData();
        counts++;
        usleep(300*1000);
        rc_loop = mosquitto_loop(mosq,-1,1);
        printf("***rc_loop ***=%d\n",rc_loop );
        if(rc_loop  != MOSQ_ERR_SUCCESS)
        {
            printf("rc_loop err =%d,restart mqtt!!!\n",rc_loop );
            setNetworkFlat(0);
            char cTest[255];
            sprintf(cTest," echo %d >%s/test3.txt",rc_loop,PATH);
            printf("%s\n",cTest);
            system(cTest);
            i_count++;
            sprintf(cTest," echo %d >%s/test.txt",i_count,PATH);
            printf("%s\n",cTest);
            system(cTest);
            printf("echo '1' > /sys/class/gpio/gpio27/value" );
            system("echo '1' > /sys/class/gpio/gpio27/value");
            //mosquitto_reconnect(mosq);
            destroy_mqtt();
            mqtt_init();
        }
    }

#ifndef MQTT_CMD
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
#endif

    return 0;
}

//mosquitto_pub -h 14.215.130.186 -p 1883 -t invt -u invtGTerm -P in2015v11t11 -m "{\"id\": \"201603161115999\",\"type\": 2,\"k24112\": 4096}"
// command  send mode
#ifdef MQTT_CMD
void MqttSendMsg(StructDriverUpData * upData)
{
    //     StructDriverUpData * upData = get_driver_up_data(0,0);
    unsigned char sendBuf[2048] = {0};
    //        sprintf(package_buf, "mosquitto_pub -h 172.16.6.115 -t mqtt -m %s",package_version); //发送局域网
    sprintf(sendBuf, "mosquitto_pub -h %s -p %d -t %s -u %s -P %s -i %s -m \"{\\\"ADDR\\\":%d,\\\",\\\"sign\\\":\\\"%s\\\",",
            mqttInfo->strAddress,
            mqttInfo->iPort,
            mqttInfo->strTopic,
            mqttInfo->strUseName,
            mqttInfo->strPwd,
            mqttInfo->strClientID,
            upData->struct_up_modbus_data[0].modbus_addr,
            upData->struct_up_modbus_data[0].device_types);
    int num = upData->struct_up_modbus_data[0].up_modbus_cmd_num;
    int i = 0;
    for (i = 0; i < num-1 ; i++)
    {
        char cTemp[100] = {0};
        sprintf(cTemp, "\\\"%d\\\":%d,"
                , upData->struct_up_modbus_data[0].unitAddrValue[i].addr
                , upData->struct_up_modbus_data[0].unitAddrValue[i].u_value);
        strcat(sendBuf, cTemp);
    }
    char cTemp[100] = {0};
    sprintf(cTemp, "\\\"%d\\\":%d\}\""
            , upData->struct_up_modbus_data[0].unitAddrValue[num-1].addr
            , upData->struct_up_modbus_data[0].unitAddrValue[num-1].u_value);
    strcat(sendBuf, cTemp);

    printf(sendBuf);
    printf("\n");
    int sub_status = system(sendBuf);
    WIFEXITED(sub_status);

}
// programer  send mode
#else

int MqttSendMsg(StructUpModbusData * upData, int device)
{
    printf("MqttSendMsg!!!\n");
    int rc;
    int rc_back = -1;

//    destroy_mqtt();//以后测试是否是公司网络原因？？
//    rc  =mqtt_init();
//    if(rc == 0)
//    {
//        if(rc_back!=rc)
//            system("echo '0' > /sys/class/gpio/gpio27/value");
//        setNetworkFlat(1);
//        rc_back = rc;
//    }
//    else
//    {
//        if(rc_back!=rc)
//            system("echo '1' > /sys/class/gpio/gpio27/value");
//        rc_back = rc;
//    }
    int iRet = -1;
    unsigned char sendBuf[UPDATAMAX*100] = {0};
    printf("upData->iUpAddrNum = %ld\n",upData->iUpAddrNum);
    int num = upData->iUpAddrNum;
    printf("00:device addr num = %d iHistoryIndex=%d **device_num =%d\n",num,iHistoryIndex,device);
    if (num <= 0 || num >=2048)
    {
        return 0;
    }
    else
    {
        sprintf(sendBuf, "{\"type\":3,\"ADDR\":%d,\"sign\":\"%s\",",
                upData->iDeviceID,
                upData->cDeviceTypes);
        printf("MqttSendMsg3333 = %s\n",sendBuf);
        int i = 0;
        for (i = 0; i < num-1 ; i++)
        {
            char cTemp[100] = {0};
            sprintf(cTemp, "\"%d\":%d,"
                    , upData->unitAddrValue[i].iAddr
                    , upData->unitAddrValue[i].usValue);
            strcat(sendBuf, cTemp);
        }
        char cTemp[100] = {0};
        printf("1111****getDownDriver1()=%d****getDownDriver2()=%d\n",getDownDriver1(),getDownDriver2());
#ifdef DRIVE_COM1
        if(getDownDriver1() == 2)
        {
            sprintf(cTemp, "\"%d\":%d,\"isTotal\":1}"
                    , upData->unitAddrValue[num-1].iAddr
                    , upData->unitAddrValue[num-1].usValue);
            strcat(sendBuf, cTemp);
        }
#else
        if(getDownDriver2() == 2)
        {
            printf("3333\n");
            sprintf(cTemp, "\"%d\":%d,\"isTotal\":1}"
                    , upData->unitAddrValue[num-1].iAddr
                    , upData->unitAddrValue[num-1].usValue);
            strcat(sendBuf, cTemp);
        }
#endif
        else
        {
            sprintf(cTemp, "\"%d\":%d}"
                    , upData->unitAddrValue[num-1].iAddr
                    , upData->unitAddrValue[num-1].usValue);
            strcat(sendBuf, cTemp);
        }
        printf(sendBuf);
        iRet = mosquitto_publish(mosq, &mid_sent, mqttInfo->strTopic, strlen(sendBuf), sendBuf, qos, retain);
        usleep(300*1000);
        printf("\n1111****getDownDriver1()=%d****getDownDriver2()=%d\n",getDownDriver1(),getDownDriver2());
    }
    if( iRet)
    {
            printf("iRet = %d,  my_mosquitto_publish error.\n", iRet);
            int rc1 ;
                mosquitto_reconnect(mosq);//MOSQ_ERR_NO_CONN=4 -      if the client isn't connected to a broker.
                char cTest[255];
                i_count_mqtt++;
                sprintf(cTest," echo %d >%s/test2.txt",i_count_mqtt,PATH);
                printf("%s\n",cTest);
                system(cTest);
                rc1 = mosquitto_publish(mosq, &mid_sent, mqttInfo->strTopic, strlen(sendBuf), sendBuf, qos, retain);
                if(rc1)
                {
                    printf("**rc1 = %d,  my_mosquitto_publish error.\n", rc1);
                    send_fault_counts++;
                    bHistoryFlag = false;
                }
                else
                {
                    printf("**rc1 = %d,  my_mosquitto_publish success!!!.\n", rc1);
                }
                 usleep(200*1000);
        if(send_fault_counts>=5)
        {
            printf("33333\n");
            send_fault_counts = 0;
            bConnectFlag =true;
            setNetworkFlat(0);
            system("echo '1' > /sys/class/gpio/gpio27/value");
        }
        if(bConnectFlag)
        {
                    bSaveAndSend = true;
                    iInsertLocalIndex= SelectCurrentData(SORT_DESC);
                    iInsertLocalIndex++;
                    printf("Local Insert iInsertLocalIndex=%d!!!!!\n",iInsertLocalIndex);
                    InsertToDatarouteDb(iInsertLocalIndex, upData->iDeviceID, upData->cDeviceTypes, sendBuf);
                    printf("*********ifup -a\n");
                    system("ifup -a");
        }
        //return -1;
        usleep(100*1000);
    }
    else
    {
        send_fault_counts = 0;
        bConnectFlag =false;
        bHistoryFlag =true;
         setNetworkFlat(1);
        system("echo '0' > /sys/class/gpio/gpio27/value");
        printf("iRet = %d,  my_mosquitto_publish success!!!.\n", iRet);
    }
    upData->iUpAddrNum = 0;
    printf(" upData->iUpAddrNum=%d\n", upData->iUpAddrNum);
    int DeviceNum;
    DeviceNum =getiHandAddr() - 1;
    printf("device = %d****DeviceNum=%d\n",device,DeviceNum);
#ifdef DRIVE_COM1
    if( getDownDriver1()==2 &&  device == DeviceNum )
    {
        printf("setDownDriver1(0)!!!!!!!\n");
        setDownDriver1(0);
    }
#else
    if( getDownDriver2()==2 &&  device == DeviceNum)
    {
        printf("setDownDriver2(0)!!\n");
        setDownDriver2(0);
    }
#endif
    usleep(100*1000);
    return 0;
}

void  SendHistoryData()
{
    printf("SendHistoryData!!!\n");
    if(bHistoryFlag)
    {
        bSaveAndSend = false;
        printf("bUSBflag is %d***1111\n",bUsbFlag);
        if(bUsbFlag)
        {
            if( iHistoryIndex == 0 )
            {
                int iDBMinUsb = GetFileMaxAndMinNum(cUSBDbPath,NUM_MIN);
                int iDBMinLocal = GetFileMaxAndMinNum(PATHDB,NUM_MIN);
                if(iDBMinUsb < iDBMinLocal)
                {
                    int iDbNum = GetFileNum(cUSBDbPath);
                    if(iDbNum > 0)
                    {
                        char cCpDBCmd[255];
                        sprintf(cCpDBCmd,"cp -a %s/dataroute_%d.db %s",cUSBDbPath,iDBMinUsb,PATHDB);
                        printf("cCpDBCmd = %s\n",cCpDBCmd);
                        system(cCpDBCmd);
                        char cRmDBCmd[255];
                        sprintf(cRmDBCmd,"rm %s/dataroute_%d.db",cUSBDbPath,iDBMinUsb);
                         printf("cRmDBCmd = %s\n",cRmDBCmd);
                        system(cRmDBCmd);
                    }
                }
            }
        }

            int iDBMinNumLocal = GetFileMaxAndMinNum(PATHDB,NUM_MIN);
            CreateConnectionToHisDb(PATHDB,iDBMinNumLocal);
            iHistoryIndex = SelectLastData();
        if(iHistoryIndex > 0)
        {
            printf("iHistoryIndex=%d\n",iHistoryIndex);
            unsigned char sendHisBuf[UPDATAMAX*100] = {0};
            SelectHistoryData(iHistoryIndex,sendHisBuf);
            int rc = -1;
            rc = mosquitto_publish(mosq, &mid_sent, mqttInfo->strTopic, strlen(sendHisBuf), sendHisBuf, qos, retain);
            if( rc )
            {
                printf("rc = %d ***mosquitto_publish History Data  failed!!",rc);
            }
            else
            {
                printf("rc = %d ***mosquitto_publish History Data  success!!",rc);
                DeleteDataroute(iHistoryIndex);
            }
            usleep(200*1000);
        }
        else
        {
            CloseHisDB();
            int iFileNumLocal = GetFileNum(PATHDB);
            if(iFileNumLocal > 0)
            {
                 char cRmDBCmd[255];
                sprintf(cRmDBCmd,"rm %s/dataroute_%d.db",PATHDB,iDBMinNumLocal);
                printf("%s\n",cRmDBCmd);
                system(cRmDBCmd);
            }

        }
        //usleep(100*1000);
 CloseHisDB();
    }

}

void destroy_mqtt()
{
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}
#endif

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    printf("my_log_callback %s\n", str);
}

void print_usage(void)
{
    printf("\nSee http://mosquitto.org/ for more information.\n\n");
}

int mqtt_init()
{
    port = mqttInfo->iPort;
    bind_address = mqttInfo->strBindAddtess;
    host = mqttInfo->strAddress;
    id = mqttInfo->strClientID;
    qos = 0;
    if(qos<0 || qos>2)
    {
        fprintf(stderr, "Error: Invalid QoS given: %d\n", qos);
        return 1;
    }
    topic = mqttInfo->strTopic;
    username = mqttInfo->strUseName;
    password = mqttInfo->strPwd;
    keepalive= mqttInfo->iKeepalive;

    if(!topic)
    {
        fprintf(stderr, "Error:  topic  must be supplied.\n");
        return 1;
    }

    if(password && !username)
    {
        if(!quiet) fprintf(stderr, "Warning: Not using password since username not set.\n");
    }
    mosquitto_lib_init();
    mosq = mosquitto_new(id, true, NULL);

    if(!mosq)
    {
        switch(errno){
        case ENOMEM:
            fprintf(stderr, "Error: Out of memory.\n");
            break;
        case EINVAL:
            fprintf(stderr, "Error: Invalid id.\n");
            break;
        }
        mosquitto_lib_cleanup();
        return 1;
    }

    if(username && mosquitto_username_pw_set(mosq, username, password))
    {
        if(!quiet) fprintf(stderr, "Error: Problem setting username and password.\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    mosquitto_max_inflight_messages_set(mosq, max_inflight);
    if(debug)
    {
        mosquitto_log_callback_set(mosq, my_log_callback);
    }
    if(use_srv)
    {

        rc = mosquitto_connect_srv(mosq, host, /*keepalive*/15, bind_address);
    }else
    {
        //printf("keepalive****************\n");
        rc = mosquitto_connect_bind(mosq, host, port, /*keepalive*/15, bind_address);
    }
    if(rc)
    {
        if(!quiet)
        {
            if(rc == MOSQ_ERR_ERRNO)
            {
                fprintf(stderr, "Error: %s\n", err);
            }
            else
            {
                fprintf(stderr, "Unable to connect (%d).\n", rc);
            }
        }
        mosquitto_lib_cleanup();
        return rc;
    }
    return rc;
}

char* getusbdevice(char* path)
{
     printf("getusbdevice1\n");
    int i,j;
    int iFileExist;
    char cFileCmd[CMDMAX];
    char cFileName[CMDMAX]={"no usb"};
    printf("getusbdevice2\n");
    for(i = 'a'; i <= 'z'; i++)
    {
       // printf("getusbdevice3\n");
        char cDeviceChar[2];
        cDeviceChar[0] = i;
        cDeviceChar[1] = '\0';
        for(j = 0; j <= 9; j++)
        {
            //printf("getusbdevice4\n");
            sprintf(cFileCmd,"%s/sd%s%d",path,cDeviceChar,j);
            //printf("cFileCmd = %s\n",cFileCmd);
            if( access(cFileCmd,F_OK) == 0 )
            {
                sprintf(cFileName,"sd%s%d",cDeviceChar,j);
                printf("it is mounted %s!!\n",cFileName);
                return cFileName;
            }

        }
    }
    printf("getusbdevice5\n");
    return cFileName;
    printf("getusbdevice6\n");
}

int mountdevice()
{
    printf("mountdevice\n");
    char* cDeviceName;
     char cUmountCmd[CMDMAX];
    cDeviceName = getusbdevice("/dev");
     if(strstr(cDeviceName,"no usb"))
         return -1;
    sprintf(cDevPath,"/dev/%s",cDeviceName);
    sprintf(cMntPath,"/mnt/%s",cDeviceName);
    if(access(cDevPath,F_OK) == 0)
    {
        printf("4444444444444\n");
        if((access(cMntPath,F_OK) != 0))
        {
            sprintf(cMkdirCmd,"mkdir -p %s",cMntPath);
            printf("%s\n",cMkdirCmd);
            system(cMkdirCmd);
        }
            char cUSBDbFile[CMDMAX];
            sprintf(cUSBDbFile,"/mnt/%s/DB",cDeviceName);
            printf("%s\n",cUSBDbFile);
            if((access(cUSBDbFile,F_OK) != 0))
            {
                sprintf( cUmountCmd,"umount -vl %s",cMntPath);
                printf("%s\n",cUmountCmd);
                system(cUmountCmd);
                usleep(500*1000);
                sprintf( cMountCmd,"mount  %s %s",cDevPath,cMntPath);
                printf("%s\n",cMountCmd);
                int mount_status = system(cMountCmd);
                usleep(900*1000);
                if(mount_status == 0)
                {
                    char cMkdirDBCmd[255];
                    sprintf( cMkdirDBCmd,"mkdir -p %s",cUSBDbFile);
                    printf("%s\n",cMkdirDBCmd);
                    system(cMkdirDBCmd);
                    mntpath =cMntPath;
                    return 0;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                 mntpath =cMntPath;
                return 0;
            }
    }
    else
    {
                    sprintf( cUmountCmd,"umount -f %s",cMntPath);
                    printf("%s\n",cUmountCmd);
                    system(cUmountCmd);
                    usleep(500*1000);
                    char rmcmd[255];
                    sprintf( rmcmd,"rm -rf /mnt/*");
                    printf("%s\n",rmcmd);
                    system(rmcmd);
                    usleep(500*1000);

        return -1;
    }
   printf("it is out mountdevice\n");
}

void usbDBmanage()
{
    char cCheckFileSize[CMDMAX];
    char cFileName[LASTMAX];
    char cFileSize[LASTMAX];

    int iUSBSize;
    int iDBMaxFile;
    int iDBMinFile;

    printf("usbDBmanage()\n");
    if(mntpath ==NULL)
        return;
    sprintf(cFileName,"%s",mntpath);//获取U盘容量
    sprintf(cCheckFileSize,"df -k %s |awk '{print $4}' | sed -n '2p'>%s/usbsize.txt",mntpath,PATH);
    printf("USB cCheckFileSize = %s\n",cCheckFileSize);
    system(cCheckFileSize);

    FILE* fp = fopen("/usr/bin/dataroute/usbsize.txt","r");
    if(fgets(cFileSize,MAX_LINE_SIZE,fp)!=NULL)
    {
            printf("****\n");
            char *result = NULL;
            int isize;
            result = strtok(cFileSize, " ");
            if(result != NULL)
                isize = atoi(result)-1000;
            iUSBMax = isize;
    }
    printf("iUSBMax = %d\n",iUSBMax);
    fclose(fp);

    sprintf(cUSBDbPath,"%s/DB",mntpath);
    if(!(access(cUSBDbPath,F_OK) == 0))//如果数据库文件路径不存在，建立文件路径
    {
        printf("it is first create pDB file!!!\n");
        char cDBfile[LASTMAX];
        sprintf(cDBfile,"mkdir -p %s",cUSBDbPath);
        printf("it is create file:%s\n",cUSBDbPath);
        system(cDBfile);
    }

    if(iUSBMax < USB_MAX)
        iUSBSize = iUSBMax;
    else
        iUSBSize = USB_MAX;

    //iInsertLocalIndex = 0;
    if(GetFileNum(cUSBDbPath) == 0)
        return;
    iDBMaxFile = GetFileMaxAndMinNum(cUSBDbPath,NUM_MAX);
//    printf("return************************\n");
//    return;
    iDBMinFile = GetFileMaxAndMinNum(cUSBDbPath,NUM_MIN);
    iInsertUSBIndex = 0;
    if((iDBMaxFile - iDBMinFile)*DBFILESIZE>= iUSBSize)
    {
        char cRmDBCmd[255];
        sprintf(cRmDBCmd,"rm %s/dataroute_%d.db",cUSBDbPath,iDBMinFile);
        printf("%s\n",cRmDBCmd);
        system(cRmDBCmd);
    }
    printf("Exit usbDBmanage()\n");
}


void localDBmanage()
{
     printf("localDBmanage()!!!!\n");
     char cCheckFileSize[CMDMAX];
     char cFileName[LASTMAX];
     char cFileSize[LASTMAX];
     int isize;
      int iDBMaxFile;
      int iDBMinFile;
      CloseDB();
        if(!(access(PATHDB,F_OK) == 0))//如果数据库文件路径不存在，建立文件路径
        {
            printf("it is first create pDB file!!!\n");
            char cDBfile[LASTMAX];
            sprintf(cDBfile,"mkdir %s",PATHDB);
            printf("it is create file:%s\n",PATHDB);
            system(cDBfile);
            iDBMaxFile = 1;
            CreateConnectionToDb(PATHDB,iDBMaxFile);
        }
        else//否则连接排序最大的数据库文件
        {
            iDBMaxFile = GetFileMaxAndMinNum(PATHDB,NUM_MAX);
            printf("it is the %d create pDB file!!!\n",iDBMaxFile);
            CreateConnectionToDb(PATHDB,iDBMaxFile);
//            printf("return************************\n");
//            return;
        }
        sprintf(cFileName,"%s/dataroute_%d.db",PATHDB,iDBMaxFile);
        sprintf(cCheckFileSize,"du -sk %s>%s/dbsize.txt",cFileName,PATH);
        printf("%s\n",cCheckFileSize);
        system(cCheckFileSize);
        FILE* fp = fopen("/usr/bin/dataroute/dbsize.txt","r");
        if(fp==NULL)
        {
            printf("open dataroute_%d.db error!!\n",iDBMaxFile);
        }
        else
        {
             printf("open dataroute_%d.db sucessfully!!\n",iDBMaxFile);
             if ( fgets(cFileSize,MAX_LINE_SIZE,fp) )
             {
                 printf("cFileSize=%scorrect!!\n",cFileSize);
                 if( strstr(cFileSize,cFileName) )
                 {
                     char *result = NULL;
                     result = strtok(cFileSize, " ");
                     if(result !=NULL)
                         isize = atoi(result);
                     if (isize >= DBFILESIZE)
                     {
                         //iInsertLocalIndex = 0;
                         CloseDB();
                         iDBMaxFile = GetFileMaxAndMinNum(PATHDB,NUM_MAX);
                         iDBMinFile = GetFileMaxAndMinNum(PATHDB,NUM_MIN);
                         iInsertUSBIndex = 0;
                         if(bUsbFlag)
                         {
                             if(bSaveAndSend)
                             {
                                 printf("iDBMaxFile = %d*iDBMinFile = %d*iDBMaxFile - iDBMinFile = %d\n",iDBMaxFile,iDBMinFile,iDBMaxFile - iDBMinFile);
                                 while(iDBMaxFile - iDBMinFile >= LOCALDBNUM)
                                 {
                                     printf("it is moving DB file to USB**iDBSecMinFile = %d\n",iDBMinFile);
                                     printf("bUsbFlag is true!!");
                                     char cCpDBCmd[255];
                                     sprintf(cCpDBCmd,"cp -a %s/dataroute_%d.db %s",PATHDB,iDBMinFile,cUSBDbPath);
                                     printf("%s\n",cCpDBCmd);
                                     system(cCpDBCmd);
                                     char cRmDBCmd[255];
                                     sprintf(cRmDBCmd,"rm %s/dataroute_%d.db",PATHDB,iDBMinFile);
                                     printf("%s\n",cRmDBCmd);
                                     system(cRmDBCmd);
                                     iDBMaxFile = GetFileMaxAndMinNum(PATHDB,NUM_MAX);
                                     iDBMinFile = GetFileMaxAndMinNum(PATHDB,NUM_MIN);
                                 }
                             }
                         }
                         else
                         {
                             if(iDBMaxFile - iDBMinFile >= LOCALDBNUM)
                             {
                                 printf("bUsbFlag is false!!");
                                 char cRmDBCmd[255];
                                 sprintf(cRmDBCmd,"rm %s/dataroute_%d.db",PATHDB,iDBMinFile);
                                 printf("%s\n",cRmDBCmd);
                                 system(cRmDBCmd);
                             }
                         }
                         //CloseDB();
                         iDBMaxFile++;
                         CreateConnectionToDb(PATHDB,iDBMaxFile);
                         printf("it is the %d create pDB file!!!\n",iDBMaxFile);
                     }
                 }

             }
        }
        fclose(fp);
   // printf("iLocalDBIndex = %d\n",iLocalDBIndex);
}


int GetFileMaxAndMinNum(char* path,int NumType)
{
        printf("GetFileMaxAndMinNum!!!\n");
        DIR * dir;
        struct dirent * ptr;
        DIR * dir1;
        struct dirent * ptr1;
        int i = 0;
        int k = 0;
        int result;
        int *iDbNum;
        char DbName[255];
        dir = opendir(path);
        while((ptr = readdir(dir)) != NULL)
        {
            printf("111d_name : %s\n", ptr->d_name);
            if(strcmp(ptr->d_name,".") == 0)
                continue;
            else if(strcmp(ptr->d_name,"..") == 0)
                continue;
            else if(strstr(ptr->d_name,".db-journal")!=NULL)
                continue;
            else
            {
            i++;
            }
        }
        if (i == 0)
        {
            closedir(dir);
 //           closedir(dir1);
            return 1;
        }
        printf("i = %d\n",i);
        dir1 = opendir(path);
        iDbNum = (int *) malloc(i * sizeof(int));
        while((ptr1 = readdir(dir1)) != NULL)
        {
            printf("222d_name : %s\n", ptr1->d_name);
            if(strcmp(ptr1->d_name,".") == 0)
                continue;
            else if(strcmp(ptr1->d_name,"..") == 0)
                continue;
            else if(strstr(ptr1->d_name,".db-journal")!=NULL)//2016.10.15有修改
                continue;
            else
            {
                memset(DbName,0,255);
                int j=0;
                char *ptr_name = ptr1->d_name;

                while(*ptr_name != '\0')
                {
//                    printf("%s\n",ptr_name);
                    if(*ptr_name>='0'&&*ptr_name<='9')
                    {
                        DbName[j] = *ptr_name;
                         j++;
//                        printf("%s 11\n ",DbName );
                    }
                   ptr_name++;
                }
                printf("%s 22\n ",DbName );
                iDbNum[k] = atoi(DbName);
                printf(" iDbNum[%d] = %d\n",k, iDbNum[k] );
                k++;
            }

        }
//        printf("k= %d\n",k);
        if ( i == k)//文件排序
        {
            int m, n;
            for(m = 0; m <  i; m++ )
            {
                for(n = m+1; n <  i;n++ )
                {
                    int Name_temp;
                    if(iDbNum[m] > iDbNum[n])
                    {
                        Name_temp = iDbNum[n];
                        iDbNum[n] = iDbNum[m];
                        iDbNum[m] = Name_temp;
                    }
                }
            }
        }
        else
        {
            printf("it is error!!!\n");
        }
                int t;
                for(t=0;t<k;t++)
                printf("iDbNum[%d]  =  %d  ", t,iDbNum[t]);
                printf("\n");
                closedir(dir);
                closedir(dir1);
        if (NumType == 1)
        {
            printf("iDbNum[i - 1] = %d\n",iDbNum[i - 1]);
            result = iDbNum[i - 1];
            free(iDbNum);
            return result;
        }
        else if(NumType == 0)
        {
            printf("iDbNum[0] = %d\n",iDbNum[0]);
            result = iDbNum[0];
            free(iDbNum);
            return result;
        }
        else if(NumType == 2)
        {
            if(i > 2)
            {
            printf("iDbNum[1] = %d\n",iDbNum[1]);
            result = iDbNum[1];
            }
            else
            {
             result = iDbNum[0];
            }
            free(iDbNum);
            return result;
        }

}

int GetFileNum(char* path)
{
        printf("GetFileNum!!!\n");
        DIR * dir2;
        struct dirent * ptr;
        int i = 0;
        dir2 = opendir(path);
        while((ptr = readdir(dir2)) != NULL)
        {
           // printf("111d_name : %s\n", ptr->d_name);
            if(strcmp(ptr->d_name,".") == 0)
                continue;
            else if(strcmp(ptr->d_name,"..") == 0)
                continue;
            else
            {
                i++;
            }
        }
        closedir(dir2);
        return i;
}

long getfileindex(char* IndexName)
{
    long iIndex;
    char cFileIndex[MAX_LINE_SIZE];
    FILE* fp = fopen("filesize.txt","r");
    if(fp!=NULL)
    {
        while( fgets(cFileIndex,MAX_LINE_SIZE,fp) )
        {
//            printf("cFileSize=%scorrect!!\n",cFileIndex);
            if( strstr(cFileIndex,IndexName) )
            {
//                printf("******************\n");
                char *result = NULL;
                result = strtok(cFileIndex, " ");
//                printf("result = %s\n",result);
                if(result !=NULL)
                {
                    iIndex = atol(result);
//                     printf("iIndex = %lu\n",iIndex);
                    return iIndex;
                }
                else
                    return 0;
            }
        }
    }
    else
        return 0;
}


long getlocalhostip()
{
    int  MAXINTERFACES=16;
    long ip;
    int fd, intrface, retn = 0;
    struct ifreq buf[MAXINTERFACES]; ///if.h
    struct ifconf ifc; ///if.h
    ip = -1;
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) //socket.h
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) //ioctl.h
        {
            intrface = ifc.ifc_len / sizeof (struct ifreq);
            while (intrface-- > 0)
            {
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
                {
                    ip=inet_addr( inet_ntoa( ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr) );//types
                    break;
                }

            }
        }
        close (fd);
    }
    return ip;
}
