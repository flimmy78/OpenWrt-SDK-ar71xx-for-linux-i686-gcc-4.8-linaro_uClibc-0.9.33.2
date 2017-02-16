#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <unistd.h>
#include "httpcomm.h"

#define DEBUG_INFO //开启调试打印信息

#ifdef DRIVE_COM1
#define PORT_HTTP 8001 //http 驱动1 端口号
#else
#define PORT_HTTP 8002 //http 驱动2 端口号
#endif

bool bFlagClose;//如果有写，一定要写返回才能关闭socket，此过程延时处理

#define MAXDATASIZE 100
#define READ_CMD "read"
#define WRITE_CMD "write"

//TCP连接是将数据采集其作为服务器，上危机作为客户端

void* ThreadHttp(void* pVoid)
{
    int server_sockfd;//服务器端套接字
    client_sockfd = 0;
    int len;

    int sin_size;
    char buf[BUFSIZ]; //数据传送的缓冲区
    char sendBuf[BUFSIZ];

    struct sockaddr_in my_addr; //服务器网络地址结构体
    struct sockaddr_in remote_addr; //客户端网络地址结构体
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
    my_addr.sin_family=AF_INET; //设置为IP通信
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
    my_addr.sin_port=htons(PORT_HTTP); //服务器端口号

    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 1;
    }

    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
    {
        perror("bind");
        return 1;
    }

    bFlagClose = false;

    while(m_bExitHttp)
    {
        listen(server_sockfd,5);
        sin_size=sizeof(struct sockaddr_in);
        if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,(socklen_t *)&sin_size))<0)
        {
            perror("accept");
            continue;
        }

#ifdef DEBUG_INFO
        printf("HTTP accept client %s\n",inet_ntoa(remote_addr.sin_addr));
#endif

        bzero(buf,BUFSIZ);
        while((len=recv(client_sockfd,buf,BUFSIZ,0))>0)
        {
            buf[len]='\0';

#ifdef DEBUG_INFO
            printf("%s\n",buf);
#endif
            StrReplace(buf, "%22", "\""); //将%22 转为双引号

            char* token = strtok(buf, "\"");
            if (token == NULL)
            {
#ifdef DEBUG_INFO
                printf("modify client id  is error!\n");
#endif
                close(client_sockfd);
                continue;
            }

            if(NULL != strstr(token, READ_CMD)) //是读取寄存器地址
            {
                // http://172.16.6.207:8001/read?"01031001000A90CD"
                int iDevice = 0;
                int iAddr = 0;
                int iLen = 0;

                token = strtok(NULL, "\"");
                int iLength = strlen(token);
                if (16 != iLength)
                {
                    printf("http read modbus is error! \n");
                    close(client_sockfd);
                    continue;
                }

                unsigned char cmdData[20];
                memset(cmdData, 0 , 20);
                int iRet = strToHex(token, cmdData);
                iDevice = cmdData[0];
                iAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                iLen = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);

                memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                getUpBuf(sendBuf, iDevice, iAddr, iLen);
#ifdef DEBUG_INFO
                printf("sendBuf = %s \n", sendBuf);
#endif
                if(send(client_sockfd,sendBuf,strlen(sendBuf),0)<0)
                {
#ifdef DEBUG_INFO
                    printf("send the value is error! \n");
#endif
                    perror("write");
                }
                close(client_sockfd);
            }
            else if (NULL != strstr(token, WRITE_CMD))
            {
                //http://172.16.6.207:8001/write?"01101001000204000100022FA2"

                int iDevice = 0;
                int iAddr = 0;
                int iLen = 0;

                token = strtok(NULL, "\"");
                unsigned char cmdData[100];
                memset(cmdData, 0 , 100);
                int iRet = strToHex(token, cmdData);
                iDevice = cmdData[0];
                iAddr = ((cmdData[2] << 8) & 0xFF00) + (cmdData[3] & 0xFF);
                iLen = ((cmdData[4] << 8) & 0xFF00) + (cmdData[5] & 0xFF);

                printf("iDevice = %d, iAddr = %d, iLen = %d \n", iDevice, iAddr, iLen);

                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFunHttp;
                taskItem.m_ucCommand = SERIAL_COMMAND_WRITE_MULTI;
                taskItem.m_enTaskType = TT_0TempWrite;
                taskItem.m_enTaskState = TS_Ready;
                taskItem.m_ucTaskRetry = SerialInfo->iResend;
                taskItem.m_usTaskOverTime = SerialInfo->iTimeout;
                taskItem.m_ulDataAddr = iAddr;
                taskItem.m_ulDataNum = iLen;
                taskItem.m_ulDeviceAddr = iDevice;
                taskItem.m_usTaskSpan = 0;
                int iNum = 0;
                for (iNum = 0; iNum < taskItem.m_ulDataNum; iNum++)
                {
                    taskItem.m_usDataBuf[iNum] = ((cmdData[7 + 2 * iNum] << 8) & 0xFF00) + (cmdData[8 + 2* iNum] & 0xFF);
                }
                taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                        + ((taskItem.m_ulDataNum<< 8) & 0xFF00) + iDevice;//高16位地址,低16位中的高8位是读取个数，低8位是ID号

                UserAddTask(taskItem);
                bFlagClose = true;
                while(bFlagClose)
                {
                    usleep(1000);
                }
            }
            else
            {
                if (bFlagClose)
                {
                    while(bFlagClose)
                    {
                        usleep(1000);
                    }
                }
                else
                {
                    close(client_sockfd);
                }
            }
        }
    }

    close(client_sockfd);
    close(server_sockfd);
    pthread_exit(&threadHttp);
}

void sendHttp(int client_sockfd, char* sendBuf)
{
    send(client_sockfd,sendBuf,strlen(sendBuf),0);
}

//根据传入的设备号和地址，读取长度返回上传的报文
void getUpBuf(char* destBuf, int id, int iAddr, int iLen)
{
    printf("getUpBuf:: id = %d, iAddr = %d, iLen = %d \n", id, iAddr, iLen);
    char sendBuf[BUFSIZ]={0};
#ifdef DRIVE_COM1
    int i = 0;
    for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
    {
        if (id == drive_data[EN_COM1].deviceInfo[i].iDeviceID)
        {
            int j = 0;
            int k = 0;
            bool bFlag = false;//找到了起始地址
            for (j = 0; j < drive_data[EN_COM1].deviceInfo[i].iDeviceNum; j++)
            {
                if (bFlag)
                {
                    if (k < iLen - 1)
                    {
                        char cTemp[100] = {0};
                        sprintf(cTemp, "\"%d\","
                                , drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue);
                        strcat(sendBuf, cTemp);
                        k++;
                    }
                    else if (k == (iLen - 1))
                    {
                        char cTemp[100] = {0};
                        sprintf(cTemp, "\"%d\"]}]}"
                                , drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue);
                        strcat(sendBuf, cTemp);
                        memcpy(destBuf, sendBuf, strlen(sendBuf));
                        return;
                    }
                }
                if (iAddr == drive_data[EN_COM1].deviceInfo[i].deviceData[j].usVarAddr)
                {
                    printf("11:iAddr is find! \n");
                    if ((iAddr + iLen -1) == drive_data[EN_COM1].deviceInfo[i].deviceData[j + iLen - 1].usVarAddr)
                    {
                        printf("22:iAddr Len is find! \n");
                        bFlag = true;
                        if (iLen > 1)
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            sprintf(sendBuf, "{\"series\":[{\"error\":0,\"ID\":\"%d\",\"data\":[", id);
                            char cTemp[100] = {0};
                            sprintf(cTemp, "\"%d\","
                                    , drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue);
                            strcat(sendBuf, cTemp);
                            k++;
                        }
                        else
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            sprintf(sendBuf, "{\"series\":[{\"error\":0,\"ID\":\"%d\",\"data\":[\"%d\"]}]}",
                                    id, drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue);
                            memcpy(destBuf, sendBuf, strlen(sendBuf));
                            return;
                        }
                    }
                }
            }
        }
    }
    sprintf(sendBuf, "{\"series\":[{\"error\":1,\"ID\":\"%d\",\"data\":[]}]}", id);
    memcpy(destBuf, sendBuf, strlen(sendBuf));
#else
    int i = 0;
    for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
    {
        if (id == drive_data[EN_COM2].deviceInfo[i].iDeviceID)
        {
            int j = 0;
            int k = 0;
            bool bFlag = false;//找到了起始地址
            for (j = 0; j < drive_data[EN_COM2].deviceInfo[i].iDeviceNum; j++)
            {
                if (bFlag)
                {
                    if (k < iLen - 1)
                    {
                        char cTemp[100] = {0};
                        sprintf(cTemp, "\"%d\","
                                , drive_data[EN_COM2].deviceInfo[i].deviceData[j].sVarValue);
                        strcat(sendBuf, cTemp);
                        k++;
                    }
                    else if (k == (iLen - 1))
                    {
                        char cTemp[100] = {0};
                        sprintf(cTemp, "\"%d\"]}]}"
                                , drive_data[EN_COM2].deviceInfo[i].deviceData[j].sVarValue);
                        strcat(sendBuf, cTemp);
                        memcpy(destBuf, sendBuf, strlen(sendBuf));
                        return;
                    }
                }
                if (iAddr == drive_data[EN_COM2].deviceInfo[i].deviceData[j].usVarAddr)
                {
                    printf("11:iAddr is find! \n");
                    if ((iAddr + iLen -1) == drive_data[EN_COM2].deviceInfo[i].deviceData[j + iLen - 1].usVarAddr)
                    {
                        printf("22:iAddr Len is find! \n");
                        bFlag = true;
                        if (iLen > 1)
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            sprintf(sendBuf, "{\"series\":[{\"error\":0,\"ID\":\"%d\",\"data\":[", id);
                            char cTemp[100] = {0};
                            sprintf(cTemp, "\"%d\","
                                    , drive_data[EN_COM2].deviceInfo[i].deviceData[j].sVarValue);
                            strcat(sendBuf, cTemp);
                            k++;
                        }
                        else
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            sprintf(sendBuf, "{\"series\":[{\"error\":0,\"ID\":\"%d\",\"data\":[\"%d\"]}]}",
                                    id, drive_data[EN_COM2].deviceInfo[i].deviceData[j].sVarValue);
                            memcpy(destBuf, sendBuf, strlen(sendBuf));
                            return;
                        }
                    }
                }
            }
        }
    }
    sprintf(sendBuf, "{\"series\":[{\"error\":1,\"ID\":\"%d\",\"data\":[]}]}", id);
    memcpy(destBuf, sendBuf, strlen(sendBuf));
#endif
}

int getValueBySplite(char* sour, char split)
{
    char cTemp [BUFSIZ] = {0};
    int i = 0;
    int j = 0;
    int iValue = 0;
    bool bFlag = false;
    while(sour[i] != '\0')
    {
        if (sour[i] != split)
        {
            if (bFlag)
            {
                cTemp[j] = sour[i];
                j++;
            }
        }
        else
        {
            bFlag = true;
        }
        i++;
    }
    cTemp[j] = '\0';

    //这个地方只计算数字
    j = 0;
    while((cTemp[j] >= '0') && (cTemp[j] <= '9'))
    {
        j++;
    }
    cTemp[j] = '\0';
    iValue = atoi(cTemp);
    printf ("getValueBySplite::iValue =%d \n", iValue);
    return iValue;
}

//创建http线程
void initThreadHttp()
{
    m_bExitHttp = true;
    pthread_create(&threadHttp, NULL, ThreadHttp, NULL);
}

void receiveFunHttp(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                    ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                    UCHAR enReason )
{
    char sendBuf[BUFSIZ] = {0};
    if ( ERR_None ==  enReason)
    {
        sprintf(sendBuf, "{\"series\":[{\"error\":0,\"ID\":\"%d\",\"data\":[\"%d\"]}]}", ulDeviceAddr, *pusDataBuf);
        sendHttp(client_sockfd, sendBuf);
        printf("sendBuf = %s \n", sendBuf);
        close(client_sockfd);
        bFlagClose = false;
    }
    else
        //    else if (ERR_TimeOut == enReason)
    {
        sprintf(sendBuf, "{\"series\":[{\"error\":2,\"ID\":\"%d\",\"data\":[]}]}", ulDeviceAddr, *pusDataBuf);
        sendHttp(client_sockfd, sendBuf);
        printf("sendBuf = %s \n", sendBuf);
        close(client_sockfd);
        bFlagClose = false;
    }
}

//将字符转化为十进制
int charToHex(char c)
{
    if((c >= '0') && (c <= '9'))
        return c - '0';
    else if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    else if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    else
        return 0x10;
}

int strToHex(char* sour, unsigned char* dest)
{
    int t, t1;
    int rlen = 0;
    int len = strlen(sour);
    int i = 0;
    for (i = 0;i < len;)
    {
        char l,h = sour[i];
        if (h == ' ')
        {
            i++;
            continue;
        }
        i++;
        if (i >= len)
        {
            break;
        }
        l = sour[i];
        t = charToHex(h);
        t1 = charToHex(l);
        if ((t == 16) || (t1 == 16))
            break;
        else
            t = t * 16 + t1;
        i++;
        dest[rlen] = (char)t;
        rlen++;
    }
    return rlen;
}

// 将strRes中的t替换为s，替换成功返回1，否则返回0。
int StrReplace(char strRes[],char from[], char to[])
{
    int i,flag = 0;
    char *p,*q,*ts;
    for(i = 0; strRes[i]; ++i)
    {
        if(strRes[i] == from[0])
        {
            p = strRes + i;
            q = from;
            while(*q && (*p++ == *q++));
            if(*q == '\0')
            {
                ts = (char *)malloc(strlen(strRes) + 1);
                strcpy(ts,p);
                strRes[i] = '\0';
                strcat(strRes,to);
                strcat(strRes,ts);
                free(ts);
                flag = 1;
            }
        }
    }
    return flag;
}
