#include "netCommu.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define DEBUG_INFO //开启调试打印信息

#define PORT_UDP 988 //UDP端口号
#define PORT_TCP 8000 //TCP 端口号
#define MAXDATASIZE 100

//根据传入字符解析是否修改客户ID,并执行
int setIDByTcp(char * buf);

//根据文件路径和文件名称判断文件是否存在并发送给PC助手
void upFileToPC(FILE *stream, int clientSockfd, char* pPathName, char* pFileName);

//UDP连接是将数据采集其作为服务器，上危机作为客户端
void* ThreadUDP(void*  pVoid)
{
    int udpSocket; //socket 套接字句柄
    struct sockaddr_in server_info; //服务器地址信息
    struct sockaddr_in client_info; //客户端地址信息
    socklen_t sin_size;
    int num;
    char recvmsg[MAXDATASIZE]; //接受客户端消息缓存
    char sendmsg[MAXDATASIZE];//向客户端发送消息缓存
    char READID[] = "INVT"; //扫描设备的广播命令

    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)//创建UDP socket
    {
        perror("Creating socket failed.");
        exit(1);
    }

    bzero(&server_info,sizeof(server_info));
    server_info.sin_family=AF_INET;
    server_info.sin_port=htons(PORT_UDP);
    server_info.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind(udpSocket, (struct sockaddr *)&server_info, sizeof(struct sockaddr)) == -1)
    {
        perror("Bind error.");
        exit(1);
    }

    sin_size=sizeof(struct sockaddr_in);
    while (m_bExitUDP)
    {
        num = recvfrom(udpSocket,recvmsg,MAXDATASIZE,0,(struct sockaddr *)&client_info,&sin_size);
        if (num < 0)
        {
            perror("recvfrom error\n");
            exit(1);
        }
        recvmsg[num] = '\0';

#ifdef DEBUG_INFO
        printf("UDP You got a message (%s) from %s\n",recvmsg,inet_ntoa(client_info.sin_addr) ); //打印客户端IP地址
#endif

        if(strcmp(recvmsg,READID)==0)
        {
            //接受的命令是INVT，返回的命令是INVT-<201606210001,1.00> 用户ID加版本号
            ReadMqttConfigOnly(MQTT_CONFIG);
            char cBuf[MAXDATASIZE];
            sprintf(cBuf, "INVT-<%s,%s>", mqttInfo->strClientID, mqttInfo->strVer);
            strcpy(sendmsg, cBuf);
            sendto(udpSocket,sendmsg,strlen(sendmsg),0,(struct sockaddr *)&client_info,sin_size);
        }
        usleep(THREAD_SLEEP_SPAN);
    }

    close(udpSocket);
    pthread_exit(&threadUDP);
}

void initThreadUDP()
{
    m_bExitUDP = true;
    pthread_create(&threadUDP, NULL, ThreadUDP, NULL);
}

//TCP连接是将数据采集其作为服务器，上危机作为客户端

void* ThreadTCP(void*  pVoid)
{
    int server_sockfd;//服务器端套接字
    int client_sockfd;//客户端套接字
    int len;
    struct sockaddr_in my_addr; //服务器网络地址结构体
    struct sockaddr_in remote_addr; //客户端网络地址结构体
    int sin_size;
    char buf[BUFSIZ]; //数据传送的缓冲区
    char READID[] = "INVT"; //查询设备ID号的命令
    char READFILE[] = "up file";//上传所有配置文档命令
    char sendBuf[MAXDATASIZE];
    char downFile[MAXDATASIZE];//保存下载文件名
    FILE *stream;//读写文件流
    int iFlagFile = 0; //0:无动作 1:发送了下传文件指令
    char strPathName[BUFSIZ];//写文件的路径名称
    int iFlagWrite = 0;//写文件初始化

    memset(downFile, 0 , MAXDATASIZE);
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零
    my_addr.sin_family=AF_INET; //设置为IP通信
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
    my_addr.sin_port=htons(PORT_TCP); //服务器端口号

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

    while(m_bExitTCP)
    {
        listen(server_sockfd,5);
        sin_size=sizeof(struct sockaddr_in);
        if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
        {
            perror("accept");
            return 1;
        }

#ifdef DEBUG_INFO
        printf("TCP accept client %s\n",inet_ntoa(remote_addr.sin_addr));
#endif

        //该TCP用于修改用户ID和下载配置文件
        //读取客户ID命令是:INVT  (PC 发送数据采集器接受)
        //修改客户ID命令是:SETID*ID*，解析中间的ID (PC 发送数据采集器接受)
        //下载文件命令:down start+ "*文件名*"  "down start *comm*" (PC 发送数据采集器接受)
        //                        down end + "*文件名*" "down end *comm*" (PC 发送数据采集器接受)
        //上传所有配置文件 up file (PC 发送数据采集器接受)
        //                                up start+ "*文件名,文件长度*"  "up start *comm,650*" (数据采集器 发送 PC 接受)
        //                                up end+ "*文件名*"  "up end *comm*" (数据采集器 发送 PC 接受)

        bzero(buf,BUFSIZ);
        while((len=recv(client_sockfd,buf,BUFSIZ,0))>0)
        {
            buf[len]='\0';

#ifdef DEBUG_INFO
            printf("%s\n",buf);
#endif

            if (len < 50) //传指令
            {
                if(0 == strcmp(buf,READID)) //如果是读取设备ID命令
                {
                    //接受查询ID的命令是INVT，返回的命令是INVT-<201606210001,1.00> 用户ID加版本号
                    ReadMqttConfigOnly(MQTT_CONFIG);
                    char cBuf[MAXDATASIZE];
                    sprintf(cBuf, "INVT-<%s,%s>",mqttInfo->strClientID, mqttInfo->strVer);
                    strcpy(sendBuf, cBuf);

#ifdef DEBUG_INFO
                    printf("sendBuf = %s, len = %d \n", sendBuf, strlen(sendBuf));
#endif

                    if(send(client_sockfd,sendBuf,strlen(sendBuf),0)<0)
                    {
                        perror("write");
                    }
                }
                else if(0 == strcmp(buf,READFILE)) //上传所有配置文档
                {
                    upFileToPC(stream, client_sockfd, SERIAL_CONFIG, SERIAL_CONFIG_NAME);
                    upFileToPC(stream, client_sockfd, MQTT_CONFIG, MQTT_CONFIG_NAME);
                    upFileToPC(stream, client_sockfd, COM1_CONFIG, COM1_CONFIG_NAME);
                    upFileToPC(stream, client_sockfd, COM2_CONFIG, COM2_CONFIG_NAME);
                }
                else
                {
                    if (1 == setIDByTcp(buf)) //设置数据采集器的用户ID
                    {
                        continue;
                    }

                    char *p;
                    p=strstr(buf, "down start"); //下载文件命令:down start+ "*文件名*"  "down start*comm*"
                    if(p)
                    {
                        iFlagFile = 1;
                        char* token = strtok( buf, "*");
                        if (token == NULL)
                        {
#ifdef DEBUG_INFO
                            printf("down start cmd  is error!\n");
#endif
                            return 0;
                        }
                        int i = 0;
                        while( token != NULL )
                        {
                            i++;
                            token = strtok( NULL, "*");
                            if (i == 1)
                            {
#ifdef DEBUG_INFO
                                printf( "down file name = %s\n", token );//token 就是下载的文件名
#endif
                                memcpy(downFile, token, strlen(token));
                                break;
                            }
                        }
                        continue;
                    }

                    p=strstr(buf, "down end"); //下载文件命令:down end+ "*文件名*"  "down end*comm*"
                    if(p)
                    {
#ifdef DEBUG_INFO
                        printf("down end 0000! \n");
#endif
                        iFlagFile = 0;
                        iFlagWrite = 0;

#ifdef DEBUG_INFO
                        printf("down end 1111! \n");
#endif
                        if (stream)
                        {
                            fclose(stream);
                            stream = NULL;
                        }

#ifdef DEBUG_INFO
                        printf("down end 2222! \n");
#endif
                        memset(downFile, 0 , MAXDATASIZE);

#ifdef DEBUG_INFO
                        printf("down end 3333! \n");
#endif
                        system("sync");

#ifdef DEBUG_INFO
                        printf( "down end is finish! \n");
#endif
                        continue;
                    }

                    if (1 == iFlagFile) //有写操作
                    {
                        if (0 == iFlagWrite)//第一次写
                        {
                            iFlagWrite = 1;
                            //接受文件
                            if (0 != strlen(downFile))//判断是否有文件名
                            {
                                sprintf(strPathName, "/etc/config/%s", downFile);

#ifdef DEBUG_INFO
                                printf("short first write strPathName is: \n");
                                printf(strPathName);
                                printf("\n");
#endif

                                if((access(strPathName,F_OK))==0) //文件存在先清空
                                {
                                    remove(strPathName);
                                }
                                system("sync");

                                if((stream=fopen(strPathName,"w"))==NULL) //没有就创建
                                {
#ifdef DEBUG_INFO
                                    printf("file open  strPathName error\n");
#endif
                                    exit(1);
                                }

//                                fwrite(buf,sizeof(char),len,stream);//将数据写入文件中
                                fwrite(buf, sizeof(char), strlen(buf), stream);//将数据写入文件中
                            }
                        }
                        else
                        {
//                            fwrite(buf,sizeof(char),len,stream);//将数据写入文件中
                            fwrite(buf, sizeof(char), strlen(buf), stream);//将数据写入文件中
                        }
                    }
                }
            }
            else //传文件
            {
                if (1 == iFlagFile) //有写操作
                {
                    if (0 == iFlagWrite)//第一次写
                    {
                        iFlagWrite = 1;
                        //接受文件
                        if (0 != strlen(downFile))//判断是否有文件名
                        {
                            sprintf(strPathName, "/etc/config/%s", downFile);

#ifdef DEBUG_INFO
                            printf("long first write strPathName is: \n");
                            printf(strPathName);
                            printf("\n");
#endif

                            if((access(strPathName,F_OK))==0) //文件存在先清空
                            {
                                remove(strPathName);
                            }
                            system("sync");

                            if((stream=fopen(strPathName,"w"))==NULL) //没有就创建
                            {
#ifdef DEBUG_INFO
                                printf("file open  strPathName error\n");
#endif
                                exit(1);
                            }

//                            fwrite(buf,sizeof(char),len,stream);//将数据写入文件中
                            fwrite(buf, sizeof(char), strlen(buf), stream);//将数据写入文件中
                        }
                    }
                    else
                    {
//                        fwrite(buf,sizeof(char),len,stream);//将数据写入文件中
                        fwrite(buf, sizeof(char), strlen(buf), stream);//将数据写入文件中
                    }
                }
            }
        }
    }

    close(client_sockfd);
    close(server_sockfd);
    pthread_exit(&threadTCP);
}

//创建TCP线程
void initThreadTCP()
{
    m_bExitTCP = true;
    pthread_create(&threadTCP, NULL, ThreadTCP, NULL);
}

//根据传入字符解析是否修改客户ID,并执行
int setIDByTcp(char * buf)
{
    char *p;
    p=strstr(buf, "SETID"); //修改命令SETID*ID*，解析中间的ID
    if(p)
    {
        char* token = strtok( buf, "*");
        if (token == NULL)
        {
            printf("modify client id  is error!\n");
            return 0;
        }
        int i = 0;
        while( token != NULL )
        {
            i++;
            token = strtok( NULL, "*");
            if (i == 1)
            {
                printf( "new client ID = %s\n", token );//token 就是最新要修改的client ID
                //修改到配置文档中
                WriteNewClientID(MQTT_CONFIG, MQTT_CONFIG_TEMP, token);
                printf("modify new ID is success!");
                return 1;
            }
        }
    }
    return 0;
}

//根据文件路径和文件名称判断文件是否存在并发送给PC助手
void upFileToPC(FILE *stream, int clientSockfd, char* pPathName, char* pFileName)
{
    char upCmd[256];//写文件的路径名称
    memset(upCmd, 0, 256);
    if((access(pPathName,F_OK))==0) //文件存在
    {
        if((stream=fopen(pPathName,"r"))==NULL)//先求出文件长度，发送给PC助手用于校验
        {
            printf("file open  SERIAL_CONFIG error\n");
        }

        fseek(stream, 0, SEEK_SET);
        fseek(stream, 0, SEEK_END);
        long iAllLen = ftell(stream);

#ifdef DEBUG_INFO
        printf("iAllLen = %ld \n", iAllLen);
#endif

        sprintf(upCmd, "up start *%s,%d*", pFileName, iAllLen);

#ifdef DEBUG_INFO
        printf("upCmd = %s \n", upCmd);
#endif

        if (send(clientSockfd,upCmd,strlen(upCmd),0) < 0)//发送上传开始命令
        {
            perror("write");
        }

        fseek(stream, 0, SEEK_SET);
        int iLen = 0;//本次读取的长度
        int iSend = 0;//已经发送的长度
        memset(upCmd, 0, 256);
        while(((iLen = fread(upCmd, 1, 256, stream)) > 0) && (iSend < iAllLen)) //发送文件
        {
            iSend += iLen;

#ifdef DEBUG_INFO
            printf("iSend = %d, iLen = %d \n", iSend, iLen);
#endif

            if (send(clientSockfd,upCmd,strlen(upCmd),0) < 0)//发送文件内容
            {
                perror("write");
            }
            fseek(stream, 0, iSend);
            memset(upCmd, 0, 256);
        }
        fclose(stream);

#ifdef DEBUG_INFO
        printf("iSend = %d \n", iSend);
#endif

        memset(upCmd, 0, 256);
        sprintf(upCmd, "up end *%s*", pFileName);

#ifdef DEBUG_INFO
        printf("upCmd = %s \n", upCmd);
#endif

        if (send(clientSockfd,upCmd,strlen(upCmd),0) < 0)//发送上传结束命令
        {
            perror("write");
        }
    }
}

