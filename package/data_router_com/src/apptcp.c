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
#include "apptcp.h"

#define DEBUG_INFO //开启调试打印信息
#define PORT_APP_TCP 8011 //该协议用来支持老版的APP手机配置接口

#define MAXDATASIZE 100

//TCP连接是将数据采集其作为服务器，上危机作为客户端
void* ThreadTCP(void* pVoid)
{
#ifdef DEBUG_INFO
        printf("TCP accept TCP client 1112 \n");
#endif

    int server_sockfd;//服务器端套接字
    client_sockfdtcp = 0;
    int len;

    int sin_size;
    char buf[BUFSIZ]; //数据传送的缓冲区
    char sendBuf[BUFSIZ];

    struct sockaddr_in my_addr; //服务器网络地址结构体
    struct sockaddr_in remote_addr; //客户端网络地址结构体
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
    my_addr.sin_family=AF_INET; //设置为IP通信
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
    my_addr.sin_port=htons(PORT_APP_TCP); //服务器端口号

    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        return 1;
    }
    int on =1;
    setsockopt( server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)
    {
        perror("bind");
        return 1;
    }

    while(m_bExitHttp)
    {
        listen(server_sockfd,5);
        sin_size=sizeof(struct sockaddr_in);
        if((client_sockfdtcp=accept(server_sockfd,(struct sockaddr *)&remote_addr,(socklen_t *)&sin_size))<0)
        {
            perror("accept");
            continue;
        }

#ifdef DEBUG_INFO
        printf("TCP accept client %s\n",inet_ntoa(remote_addr.sin_addr));
#endif

        bzero(buf,BUFSIZ);
        while((len=recv(client_sockfdtcp,buf,BUFSIZ,0))>0)
        {
                int i;
            for( i = 0; i < len; i++)
            {
                printf("buf[%d] = %02x\n",i,buf[i]);
            }
            buf[len]='\0';
            printf("TCP is received DATA ********len = %d\n",len);

#ifdef DEBUG_INFO
            printf("********buf = %x TCP OLD DATA 9090909090\n",buf);
#endif

            //接受的字符串为modbus指令,中间没有空格 例如"01031001000A90CD"
            if((buf[1] == 0x03) ) //是读取寄存器地址M
            {
                int iDevice = 0;
                int iAddr = 0;
                int iLen = 0;

//                unsigned char cmdData[20];
//                memset(cmdData, 0 , 20);
//                int iRet = strToHexTCP(buf, cmdData);
                iDevice = buf[0];
                iAddr = ((buf[2] << 8) & 0xFF00) + (buf[3] & 0xFF);
                iLen = ((buf[4] << 8) & 0xFF00) + (buf[5] & 0xFF);
                memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                getUpBufTCP(sendBuf, iDevice, iAddr, iLen);
#ifdef DEBUG_INFO
                printf("\nsendBuf = %s TCP rev DATA 9090909090\n", sendBuf);
#endif
                if(send(client_sockfdtcp,sendBuf,strlen(sendBuf),0)<0)
                {
#ifdef DEBUG_INFO
                    printf("send the value is error! \n");
#endif
                    perror("write");
                }
#ifdef DEBUG_INFO
                    printf("send the value is sucess! \n");
#endif

            }
            else if ((buf[1] == 0x10) )
            {
                //"01101001000204000100022FA2"
                int iDevice = 0;
                int iAddr = 0;
                int iLen = 0;

//                unsigned char cmdData[100];
//                memset(cmdData, 0 , 100);
//                int iRet = strToHexTCP(buf, cmdData);
                iDevice = buf[0];
                iAddr = ((buf[2] << 8) & 0xFF00) + (buf[3] & 0xFF);
                iLen = ((buf[4] << 8) & 0xFF00) + (buf[5] & 0xFF);
#ifdef DEBUG_INFO
                printf("it is writing the value ! \n");
#endif
                printf("iDevice = %d, iAddr = %d, iLen = %d \n", iDevice, iAddr, iLen);

                tagTaskItem taskItem;
                memset(&taskItem, 0 , sizeof(taskItem));
                taskItem.m_pFunTaskCallback = receiveFunTCP;
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
                    taskItem.m_usDataBuf[iNum] = ((buf[7 + 2 * iNum] << 8) & 0xFF00) + (buf[8 + 2* iNum] & 0xFF);
                }
                taskItem.m_ulTaskID = ((taskItem.m_ulDataAddr << 16) & 0xFFFF0000)
                        + ((taskItem.m_ulDataNum<< 8) & 0xFF00) + iDevice;//高16位地址,低16位中的高8位是读取个数，低8位是ID号

                UserAddTask(taskItem);
            }
        }
    }

    close(client_sockfdtcp);
    close(server_sockfd);
    pthread_exit(&threadTCP);
}

void sendTCP(int client_sockfd, char* sendBuf)
{
    send(client_sockfd,sendBuf,strlen(sendBuf),0);
}

//根据传入的设备号和地址，读取长度返回上传的报文
void getUpBufTCP(char* destBuf, int id, int iAddr, int iLen)
{
    printf("getUpBufTCP:: id = %d, iAddr = %d, iLen = %d \n", id, iAddr, iLen);
    char sendBuf[BUFSIZ]={0};
    UCHAR m_ucBuf[MAX_SENDNORMAL_BUFFER_SIZE];
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
                        //UCHAR m_ucBuf[MAX_SENDNORMAL_BUFFER_SIZE];
                        m_ucBuf[4 + 2 * (k - 1) + 1] = ((drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue >> 8) & 0xFF);
                        m_ucBuf[4 + 2 * (k - 1) + 2] = (drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue & 0xFF);
                        int iBit = 0;
                        for (iBit = 0; iBit < 2; iBit++)
                        {
                            char cTemp[10];
                            memset(cTemp, 0, 10);
                            sprintf(cTemp, "%02x", m_ucBuf[4 + 2 * (k - 1) + 1 + iBit]);
                            strcat(sendBuf, cTemp);
                        }
                        k++;
                    }
                    else if (k == (iLen - 1))
                    {
                        //UCHAR m_ucBuf[MAX_SENDNORMAL_BUFFER_SIZE];
                        m_ucBuf[4 + 2 * (k - 1) + 1] = ((drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue >> 8) & 0xFF);
                        m_ucBuf[4 + 2 * (k - 1) + 2] = (drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue & 0xFF);
                        int iBit = 0;
                        for (iBit = 0; iBit < 2; iBit++)
                        {
                            char cTemp[10];
                            memset(cTemp, 0, 10);
                            sprintf(cTemp, "%02x", m_ucBuf[4 + 2 * (k - 1) + 1 + iBit]);
                            strcat(sendBuf, cTemp);
                        }
                        printf("111*********sendBuf =%s: strlen = %d:%d\n",sendBuf,strlen(sendBuf),4 + 2 * (k - 1)  + 2 + 1);
                        USHORT CRC = GetCRC((UCHAR*)m_ucBuf, 4 + 2 * (k - 1)  + 2 + 1);
                        char cTemp[10];
                        memset(cTemp, 0, 10);
                        sprintf(cTemp, "%02x", ((CRC >> 8) & 0xFF));
                        printf("22*%02x\n", ((CRC >> 8) & 0xFF));
                        strcat(sendBuf, cTemp);
                        memset(cTemp, 0, 10);
                        sprintf(cTemp, "%02x", CRC & 0xFF);
                        strcat(sendBuf, cTemp);
                        memcpy(destBuf, sendBuf, strlen(sendBuf));
                        printf("\n*********sendBuf = %s\n",sendBuf);
                        return;
                    }
                }
                if (iAddr == drive_data[EN_COM1].deviceInfo[i].deviceData[j].usVarAddr)
                {
                    printf("11:iAddr is find! \n");
                    printf("iAddr + iLen -1=%d \ndrive_data[EN_COM1].deviceInfo[i].deviceData[j].usVarAddr= %d\n",iAddr + iLen -1,drive_data[EN_COM1].deviceInfo[i].deviceData[j].usVarAddr);
                    printf("drive_data[EN_COM1].deviceInfo[i].deviceData[j + iLen - 1].usVarAddr=%d \n",drive_data[EN_COM1].deviceInfo[i].deviceData[j + iLen - 1].usVarAddr);
                    if ((iAddr + iLen -1) == drive_data[EN_COM1].deviceInfo[i].deviceData[j + iLen - 1].usVarAddr)
                    {
                        printf("22:iAddr Len is find! \n");
                        bFlag = true;
                        if (iLen > 1)
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            memset(m_ucBuf, 0 , sizeof(UCHAR) * MAX_SENDNORMAL_BUFFER_SIZE);
                            m_ucBuf[0] = (UCHAR)id;
                            m_ucBuf[1] = 0x03;
                            m_ucBuf[2] = iLen * 2;
                            m_ucBuf[3] = ((drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue >> 8) & 0xFF);
                            m_ucBuf[4] = (drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue & 0xFF);
                            int iBit = 0;
                            for (iBit = 0; iBit < 5; iBit++)
                            {
                                char cTemp[10];
                                memset(cTemp, 0, 10);
                                sprintf(cTemp, "%02x", m_ucBuf[iBit]);
                                strcat(sendBuf, cTemp);
                            }
                            k++;
                        }
                        else //只读一个地址
                        {
                            memset(sendBuf, 0 , sizeof(char) * BUFSIZ);
                            memset(m_ucBuf, 0 , sizeof(UCHAR) * MAX_SENDNORMAL_BUFFER_SIZE);
                            m_ucBuf[0] = (UCHAR)id;
                            m_ucBuf[1] = 0x03;
                            m_ucBuf[2] = 0x02;
                            m_ucBuf[3] = ((drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue >> 8) & 0xFF);
                            m_ucBuf[4] = (drive_data[EN_COM1].deviceInfo[i].deviceData[j].sVarValue & 0xFF);
                            USHORT CRC = GetCRC(m_ucBuf, 5);
                            m_ucBuf[5] = (CRC & 0xFF);
                            m_ucBuf[6] = ((CRC >> 8) & 0xFF);
                            int iBit = 0;
                            for (iBit = 0; iBit < 7; iBit++)
                            {
                                char cTemp[10];
                                memset(cTemp, 0, 10);
                                sprintf(cTemp, "%02x", m_ucBuf[iBit]);
                                strcat(sendBuf, cTemp);
                            }
                            memcpy(destBuf, sendBuf, strlen(sendBuf));
                            return;
                        }
                    }
                }
            }
        }
    }
    memset(m_ucBuf, 0 , sizeof(UCHAR) * MAX_SENDNORMAL_BUFFER_SIZE);
    m_ucBuf[0] = (UCHAR)id;
    m_ucBuf[1] = 0x83;
    m_ucBuf[2] = 0x02;
    USHORT CRC = GetCRC(m_ucBuf, 3);
    m_ucBuf[3] = (CRC & 0xFF);
    m_ucBuf[4] = ((CRC >> 8) & 0xFF);
    int iBit = 0;
    for (iBit = 0; iBit < 5; iBit++)
    {
        char cTemp[10];
        memset(cTemp, 0, 10);
        sprintf(cTemp, "%02x", m_ucBuf[iBit]);
        strcat(sendBuf, cTemp);
    }
    memcpy(destBuf, sendBuf, strlen(sendBuf));
}

//int getValueBySplite(char* sour, char split)
//{
//    char cTemp [BUFSIZ] = {0};
//    int i = 0;
//    int j = 0;
//    int iValue = 0;
//    bool bFlag = false;
//    while(sour[i] != '\0')
//    {
//        if (sour[i] != split)
//        {
//            if (bFlag)
//            {
//                cTemp[j] = sour[i];
//                j++;
//            }
//        }
//        else
//        {
//            bFlag = true;
//        }
//        i++;
//    }
//    cTemp[j] = '\0';

//    //这个地方只计算数字
//    j = 0;
//    while((cTemp[j] >= '0') && (cTemp[j] <= '9'))
//    {
//        j++;
//    }
//    cTemp[j] = '\0';
//    iValue = atoi(cTemp);
////    printf ("getValueBySplite::iValue =%d \n", iValue);
//    return iValue;
//}

//创建http线程
void initThreadTCP()
{
    m_bExitHttp = true;
    pthread_create(&threadTCP, NULL, ThreadTCP, NULL);
}

void receiveFunTCP(ULONG ulTaskID, ULONG ulDeviceAddr, UCHAR ucCommand,
                    ULONG ulDataAddr, ULONG ulDataNum, USHORT *pusDataBuf, ULONG ulReserver,
                    UCHAR enReason )
{
    char sendBuf[BUFSIZ] = {0};
    if ( ERR_None ==  enReason)
    {
        UCHAR m_ucBuf[MAX_SENDNORMAL_BUFFER_SIZE];
        m_ucBuf[0] = (UCHAR)ulDeviceAddr;
        m_ucBuf[1] = (UCHAR)ucCommand;
        m_ucBuf[2] = (ulDataAddr >> 8 ) & 0xFF;
        m_ucBuf[3] = ulDataAddr  & 0xFF;
        m_ucBuf[4] = (ulDataNum >> 8 ) & 0xFF;
        m_ucBuf[5] = ulDataNum  & 0xFF;
        //m_ucBuf[6] = 2 * ulDataNum;
       // memcpy(m_ucBuf + 7, pusDataBuf, sizeof(USHORT) * ulDataNum);
        USHORT CRC = GetCRC(m_ucBuf, 6);
        m_ucBuf[6] =  ((CRC >> 8) & 0xFF);
        m_ucBuf[7] = (CRC & 0xFF);
        int iBit = 0;
        for (iBit = 0; iBit < 8; iBit++)
        {
            char cTemp[10];
            memset(cTemp, 0, 10);
            sprintf(cTemp, "%02x", m_ucBuf[iBit]);
            strcat(sendBuf, cTemp);
        }
        printf("*****write recv sendBuf = %s \n", sendBuf);
        sendTCP(client_sockfdtcp, sendBuf);
    }
    else
    {      
        UCHAR m_ucBuf[MAX_SENDNORMAL_BUFFER_SIZE];
        m_ucBuf[0] = (UCHAR)ulDeviceAddr;
        m_ucBuf[1] = 0x90;
        m_ucBuf[2] = 0x02;
        USHORT CRC = GetCRC(m_ucBuf, 3);
        m_ucBuf[3] = (CRC & 0xFF);
        m_ucBuf[4] = ((CRC >> 8) & 0xFF);
        int iBit = 0;
        for (iBit = 0; iBit < 5; iBit++)
        {
            char cTemp[10];
            memset(cTemp, 0, 10);
            sprintf(cTemp, "%02x", m_ucBuf[iBit]);
            strcat(sendBuf, cTemp);
        }
        printf("sendBuf = %s \n", sendBuf);
        sendTCP(client_sockfdtcp, sendBuf);
    }
}

//将字符转化为十进制
int charToHexTCP(char c)
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

int strToHexTCP(char* sour, unsigned char* dest)
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
        t = charToHexTCP(h);
        t1 = charToHexTCP(l);
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
