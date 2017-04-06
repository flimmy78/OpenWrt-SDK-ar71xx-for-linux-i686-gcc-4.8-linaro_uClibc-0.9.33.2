#include "qcommunicatemanage.h"
#include <pthread.h>
void SFunCallback_RecvNormal(UCHAR *pRecvBuf, ULONG addr, int len)
{
    Serial_DealWith_RecvNormal(pRecvBuf, addr, len);
}

void UpdateCurrentTask(enTaskState state)//更新正在执行的任务(只有基类Recv接收线程的回调函数会执行该函数)
{
    pthread_mutex_lock(&work_mutex);//注意:此处务必要有线程临界区保护
    m_TaskCurrent.m_enTaskState = state;
    int iTaskType = m_TaskCurrent.m_enTaskType;

    if (m_list_TaskStorage[iTaskType]->iAllNum!= 0)
    {
        tagTaskItem *taskItemHeader;//定义链表的首个元素
        taskItemHeader = &m_list_TaskStorage[iTaskType]->taskItem[0];

        switch (taskItemHeader->m_enTaskState)
        {
        case TS_Running:
        case TS_Error:
        case TS_Finish:
            //修改任务的状态,并填写任务的缓冲区
            taskItemHeader->m_enTaskState = state;
            memcpy(taskItemHeader->m_usDataBuf, m_TaskCurrent.m_usDataBuf, sizeof(USHORT)*m_TaskCurrent.m_ulDataNum);
            break;

        case TS_Pause:
        case TS_Delete:
        case TS_Ready:
            //如果用户中途改变了任务的状态(暂停/删除/重启),必须保持这个状态
            break;

        default:
            break;
        }
    }

    CleanCurrentTask();
    pthread_mutex_unlock(&work_mutex);
}

void CleanCurrentTask(void)//清除正在执行的任务
{
    memset(&m_TaskCurrent, 0, sizeof(tagTaskItem));
    m_TaskCurrent.m_enTaskState = TS_Finish;//代表任务为空
}

USHORT GetCRC(UCHAR *pChar, int iNum)//得到CRC校验值
{
    unsigned int val = 0xffff;
    int i = 0;

    while (iNum--)
    {
        val ^= *pChar;
        pChar++;

        for (i = 0; i < 8; i++)
        {
            if (val & 0x0001)
            {
                val = (val >> 1) ^ 0xa001;
            }
            else
            {
                val = val >> 1;
            }
        }
    }

    //C语言手动将发送的CRC高8位和低8位调换位置,低位在前高位在后
    unsigned int valH = 0;
    unsigned int valL = 0;
    valH = ((val & 0xFFFF) >> 8) & 0xFF;
    valL = val & 0xFF;
    val = ((valL << 8) & 0xFF00) + valH;
    return (USHORT)val;//返回值是16位的
}

int CRC_Value(unsigned char *pBuf, int iLen)//CRC校验
{
    int i;
    int crc_value = 0xffff;

    while (iLen--)
    {
        crc_value ^= *pBuf++;

        for (i = 0; i < 8; i++)
        {
            if (crc_value & 0x0001)
            {
                crc_value = (crc_value >> 1) ^ 0xa001;
            }
            else
            {
                crc_value = crc_value >> 1;
            }
        }
    }

    int temp;
    temp = ((crc_value) >> 8) & 0xff;
    temp += ((crc_value << 8) & 0xff00);
    crc_value = temp;
    return crc_value;
}

void Serial_DealWith_SendNormal(void)//串口:构造需要发送的内容     //lf_debug 组帧modbus指令采集函数
{
    m_iRecvLen = 0;//接收的长度清零
    memset(&m_ucRecvBuf, 0, sizeof(m_ucRecvBuf)); //接收的内容清空
    memset(&m_ucSendBuf, 0, sizeof(m_ucSendBuf)); //发送的内容清空
    m_TaskCurrent.m_ulSendBufSize = 0;//任务发送的实际有效数据的长度清零

    switch (m_TaskCurrent.m_ucCommand)
    {
    case SERIAL_COMMAND_READ:
        {
            tagSerialNormalReadBody body = {0};
            body.m_ucAddr = (UCHAR)m_TaskCurrent.m_ulDeviceAddr;
            body.m_ucCommand = (UCHAR)m_TaskCurrent.m_ucCommand;
            body.m_ucHStartAddr = ((m_TaskCurrent.m_ulDataAddr) >> 8) & 0xff; //高字节右移8位,然后取低8位.
            body.m_ucLStartAddr = m_TaskCurrent.m_ulDataAddr & 0xff;
            body.m_ucHReadNum = ((m_TaskCurrent.m_ulDataNum) >> 8) & 0xff;
            body.m_ucLReadNum = m_TaskCurrent.m_ulDataNum & 0xff;
            m_TaskCurrent.m_ulSendBufSize = sizeof(body);
            memcpy(m_ucSendBuf, &body, sizeof(body));
        }
        break;

    case SERIAL_COMMAND_WRITE_ONE:
        {
            //写一个字(==2个字节)
            tagSerialNormalWriteOneBody body = {0};
            body.m_ucAddr = (UCHAR)m_TaskCurrent.m_ulDeviceAddr;
            body.m_ucCommand = (UCHAR)m_TaskCurrent.m_ucCommand;
            body.m_ucHWriteAddr = ((m_TaskCurrent.m_ulDataAddr) >> 8) & 0xff; //高字节右移8位,然后取低8位.
            body.m_ucLWriteAddr = m_TaskCurrent.m_ulDataAddr & 0xff;
            body.m_ucHWriteContent = ((m_TaskCurrent.m_usDataBuf[0]) >> 8) & 0xff;
            body.m_ucLWriteContent = m_TaskCurrent.m_usDataBuf[0] & 0xff;
            m_TaskCurrent.m_ulSendBufSize = sizeof(body);
            memcpy(m_ucSendBuf, &body, sizeof(body));
        }
        break;

    case SERIAL_COMMAND_WRITE_MULTI:
        {
            //写多个字
            tagSerialNormalWriteMultiBody body = {0};
            body.m_ucAddr = (UCHAR)m_TaskCurrent.m_ulDeviceAddr;
            body.m_ucCommand = (UCHAR)m_TaskCurrent.m_ucCommand;
            body.m_ucHWriteAddr = ((m_TaskCurrent.m_ulDataAddr) >> 8) & 0xff; //高字节右移8位,然后取低8位.
            body.m_ucLWriteAddr = m_TaskCurrent.m_ulDataAddr & 0xff;
            body.m_ucHWriteNum = ((m_TaskCurrent.m_ulDataNum) >> 8) & 0xff; //高字节右移8位,然后取低8位.
            body.m_ucLWriteNum = m_TaskCurrent.m_ulDataNum & 0xff;
            body.m_ucByteNum = (UCHAR)m_TaskCurrent.m_ulDataNum * sizeof(USHORT);
            m_TaskCurrent.m_ulSendBufSize = sizeof(body) + body.m_ucByteNum;
            //填写消息主体addr
//            memcpy(m_ucSendBuf, &body, sizeof(body));
            //填写实际内容
//            memcpy(m_ucSendBuf + sizeof(body), m_TaskCurrent.m_usDataBuf, body.m_ucByteNum);
//            m_TaskCurrent.m_ulSendBufSize = sizeof(body) + body.m_ucByteNum;
            m_ucSendBuf[0] = body.m_ucAddr;
            m_ucSendBuf[1] = body.m_ucCommand;
            m_ucSendBuf[2] = body.m_ucHWriteAddr;
            m_ucSendBuf[3] = body.m_ucLWriteAddr;
            m_ucSendBuf[4] = body.m_ucHWriteNum;
            m_ucSendBuf[5] = body.m_ucLWriteNum;
            m_ucSendBuf[6] = body.m_ucByteNum;
            int k = 0;
            int i = 0;
            for (i =0; i <  body.m_ucByteNum; i+=2)
            {
                m_ucSendBuf[7 + i] = ((m_TaskCurrent.m_usDataBuf[k] >> 8) & 0xFF);//数据内容高位
                m_ucSendBuf[7 + i + 1] = (m_TaskCurrent.m_usDataBuf[k] & 0xFF);//数据内容低位
                k++;
            }
        }

    default:
        break;
    }

    //填写CRC
    USHORT CRC = GetCRC(m_ucSendBuf, m_TaskCurrent.m_ulSendBufSize);
    memcpy(m_ucSendBuf + m_TaskCurrent.m_ulSendBufSize, &CRC, sizeof(CRC));
    //发送buf
    m_TaskCurrent.m_ulSendBufSize += sizeof(CRC);//CRC占2个子节
    SendData(/*(char *)*/m_ucSendBuf, m_TaskCurrent.m_ulSendBufSize);
}

void Serial_DealWith_RecvNormal(UCHAR *pRecvBuf, ULONG addr, int len) //串口:处理接收到的数据
{
    //0.addr参数无用处,为了避免编译警告,这里赋值为0.
    addr = 0;

    //1.判断当前任务状态是否执行中
    if (m_TaskCurrent.m_enTaskState != TS_Running)
    {
        m_iRecvLen = 0;
        return;
    }

    //2.判断len的大小是否超出阈值
    if (len > MAX_RECVNORMAL_BUFFER_SIZE)
    {
        m_iRecvLen = 0;
        return;
    }

    //3.串口与以太网通信不同,接收数据不一定一次就完成.有可能需要好几次才全部接收完成.
    memcpy(m_ucRecvBuf + m_iRecvLen, pRecvBuf, len);
    m_iRecvLen += len;//当前已接收到的长度

    if (m_iRecvLen == 1)
    {
        //判断数据源的从机地址是否一致
        if (m_ucRecvBuf[0] != m_TaskCurrent.m_ulDeviceAddr)
        {
            m_iRecvLen = 0;
            return;
        }
    }
    else if (m_iRecvLen >= 2)
    {
        //判断数据源的从机地址是否一致
        if (m_ucRecvBuf[0] != m_TaskCurrent.m_ulDeviceAddr)
        {
            m_iRecvLen = 0;
            return;
        }

        //判断数据源的命令码类型是否一致
        UCHAR cmd1 = m_TaskCurrent.m_ucCommand;//正确应答的命令码
        UCHAR cmd2 = m_TaskCurrent.m_ucCommand + 0x80;//错误应答的命令码

        if (m_ucRecvBuf[1] == cmd1)
        {
            //串口接收的是正确应答数据
            Serial_DealWith_RecvNormal_Correct(pRecvBuf);
        }
        else if (m_ucRecvBuf[1] == cmd2)
        {
            //串口接收的是错误应答数据
            Serial_DealWith_RecvNormal_Fault(pRecvBuf);
        }
        else
        {
            m_iRecvLen = 0;
            return;
        }
    }
}

void Serial_DealWith_RecvNormal_Correct(UCHAR *pRecvBuf)//串口:处理接收到的数据(应答正确)
{
    //1.求出任务最终需要接收到的长度,并等待接收完毕
    int len = 0;

    switch (m_TaskCurrent.m_ucCommand)
    {
    case SERIAL_COMMAND_READ:
        len = sizeof(tagSerialNormalReadAck) + sizeof(USHORT) + m_TaskCurrent.m_ulDataNum * 2;//CRC占2个子节
        break;

    case SERIAL_COMMAND_WRITE_ONE:
        len = sizeof(tagSerialNormalWriteOneBody) + sizeof(USHORT);//CRC占2个子节
        break;

    case SERIAL_COMMAND_WRITE_MULTI:
        len = sizeof(tagSerialNormalWriteMultiAck) + sizeof(USHORT);//CRC占2个子节
        break;

    default:
        break;
    }

    if (m_iRecvLen != len)
    {
        //接收数据未完成,继续接收
        return;
    }

    memcpy(pRecvBuf, m_ucRecvBuf, len);
    //2.判断CRC校验值是否正确
    USHORT *CRC1 = (USHORT *)(pRecvBuf + len - sizeof(USHORT));
    USHORT CRC2 = GetCRC((UCHAR *)pRecvBuf, len - sizeof(USHORT));

    if (*CRC1 != CRC2)
    {
        ClearComBuf();
        m_iRecvLen = 0;
        m_ucTaskErrReason = ERR_CRC;
        UpdateCurrentTask(TS_Error);
        return;
    }

    //3.判断命令码的类型
    switch (m_TaskCurrent.m_ucCommand)
    {
    case SERIAL_COMMAND_READ:
        {
            tagSerialNormalReadAck *pAck = (tagSerialNormalReadAck *)pRecvBuf;
            int tmp = len - sizeof(tagSerialNormalReadAck) - sizeof(USHORT);//CRC占2个子节

            if (pAck->m_ucReadNum != tmp)
            {
                ClearComBuf();
                m_iRecvLen = 0;
                m_ucTaskErrReason = ERR_Data;
                UpdateCurrentTask(TS_Error);
                return;
            }

            //拷贝数据到buf
            USHORT *pData = (USHORT *)(pRecvBuf + sizeof(tagSerialNormalReadAck));
            memcpy(m_TaskCurrent.m_usDataBuf, pData, tmp);
            //m_usDataBuf高低位交换顺序,从"先高后低"变成"先低后高"

            //C语言和所用机器不用交换，应该是小存储
//            UCHAR high = 0;
//            UCHAR low = 0;
//            tmp = tmp / 2;

//            int i = 0;
//            for (i = 0; i < tmp; i++)
//            {
//                high = ((m_TaskCurrent.m_usDataBuf[i]) >> 8) & 0xff;
//                low = m_TaskCurrent.m_usDataBuf[i] & 0xff;
//                m_TaskCurrent.m_usDataBuf[i] = (low << 8) + high;
//            }
        }
        break;

    case SERIAL_COMMAND_WRITE_ONE:
        {
            //串口modbus的写一个数据:发送和接收应一致
            tagSerialNormalWriteOneBody *pAck = (tagSerialNormalWriteOneBody *)pRecvBuf;
            /*UCHAR ucHWriteAddr = ((m_TaskCurrent.m_ulDataAddr) >> 8) & 0xff;
            UCHAR ucLWriteAddr = m_TaskCurrent.m_ulDataAddr & 0xff;*/
            UCHAR ucHWriteContent = ((m_TaskCurrent.m_usDataBuf[0]) >> 8) & 0xff;
            UCHAR ucLWriteContent = m_TaskCurrent.m_usDataBuf[0] & 0xff;

            if (/*pAck->m_ucHWriteAddr!=ucHWriteAddr || pAck->m_ucLWriteAddr!=ucLWriteAddr ||*/
                pAck->m_ucHWriteContent != ucHWriteContent || pAck->m_ucLWriteContent != ucLWriteContent)
            {
                ClearComBuf();
                m_iRecvLen = 0;
                m_ucTaskErrReason = ERR_Data;
                UpdateCurrentTask(TS_Error);
                return;
            }
        }
        break;

    case SERIAL_COMMAND_WRITE_MULTI:
        {
            //串口modbus的写多个数据
            tagSerialNormalWriteMultiAck *pAck = (tagSerialNormalWriteMultiAck *)pRecvBuf;
            UCHAR ucHWriteAddr = ((m_TaskCurrent.m_ulDataAddr) >> 8) & 0xff;
            UCHAR ucLWriteAddr = m_TaskCurrent.m_ulDataAddr & 0xff;
            UCHAR ucHWriteNum = ((m_TaskCurrent.m_ulDataNum) >> 8) & 0xff;
            UCHAR ucLWriteNum = m_TaskCurrent.m_ulDataNum & 0xff;

            if (pAck->m_ucHWriteAddr != ucHWriteAddr || pAck->m_ucLWriteAddr != ucLWriteAddr ||
                    pAck->m_ucHWriteNum != ucHWriteNum || pAck->m_ucLWriteNum != ucLWriteNum)
            {
                ClearComBuf();
                m_iRecvLen = 0;
                m_ucTaskErrReason = ERR_Data;
                UpdateCurrentTask(TS_Error);
                return;
            }
        }
        break;

    default:
        break;
    }
#ifdef  lf_debug
    //printf(" len = %d \n", len);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", pRecvBuf[i]);
    }
    printf("\n");
#endif

    ClearComBuf();

    //4.任务执行结束
    UpdateCurrentTask(TS_Finish);
}

void Serial_DealWith_RecvNormal_Fault(UCHAR *pRecvBuf)//串口:处理接收到的数据(应答错误)
{
    //1.求出任务最终需要接收到的长度,并等待接收完毕
    int len = sizeof(tagSerialFaultAck) + sizeof(USHORT);//CRC占2个子节

    if (m_iRecvLen != len)
    {
        //接收数据未完成,继续接收
        return;
    }

    memcpy(pRecvBuf, m_ucRecvBuf, len);
    //2.判断CRC校验值是否正确
    USHORT *CRC1 = (USHORT *)(pRecvBuf + len - sizeof(USHORT));
    USHORT CRC2 = GetCRC((UCHAR *)pRecvBuf, len - sizeof(USHORT));

    if (*CRC1 != CRC2)
    {
        ClearComBuf();
        m_iRecvLen = 0;
        m_ucTaskErrReason = ERR_CRC;
        UpdateCurrentTask(TS_Error);
        return;
    }

    //3.得到错误代码
    tagSerialFaultAck *pAck = (tagSerialFaultAck *)pRecvBuf;
    m_ucTaskErrReason = pAck->m_ucFault;
    //4.任务执行结束
    UpdateCurrentTask(TS_Finish);
}

void* ThreadProc_Manage(void* pvoid)//管理者线程:负责管理和调度任务链表
{
    tagTaskItem* taskItemHeader;//定义链表的首个元素
    int iTaskType = TT_0TempWrite;//初始化任务类型
    bool bHaveTask = false;//判断所有链表有无任务flag
    int nSleepNum = 0;//周期任务的延时计数

    while (!m_bManagerExit)     //创建线程之后初始化为false, while执行,退出线程时 置 true;   lf_debug
    {
        bHaveTask = false;

        int i = 0;
        for (i = TT_0TempWrite; i < TT_Bottom; i++)
        {
            //ceshi
//            ListList(m_list_TaskStorage[i]);
            //ceshi
            if (0 != m_list_TaskStorage[i]->iAllNum)
            {
                //如果某一任务链表不为空
                bHaveTask = true;
                break;
            }
        }

        if (!bHaveTask)
        {
            //各个链表都没有任务   重新开始while
            sleep(1);
            continue;
        }

        //处理各类任务
        //0.TT_0TempWrite临时写任务(优先级最高)
        //1.TT_1TempRead临时读任务
        //2.TT_2PeriodOscillo周期示波握手任务
        //3.TT_3PeriodRead周期读任务(优先级最低)
        pthread_mutex_lock(&work_mutex);

        if (iTaskType >= TT_Bottom)
        {
            //如果任务类型超出最大范围,从头开始
            iTaskType = TT_0TempWrite;
        }

        if (m_list_TaskStorage[iTaskType]->iAllNum == 0)
        {
            //如果当前任务链表为空,跳过它,然后跟进下一类型的任务
            iTaskType++;
            pthread_mutex_unlock(&work_mutex);
            continue;
        }

        //任务链表的迭代器始终指向链表的首部
        taskItemHeader = &m_list_TaskStorage[iTaskType]->taskItem[0];     //m_list_TaskStorage  ...debug here

        //取出链表的首部任务,判断它的状态
        switch (taskItemHeader->m_enTaskState)
        {
        case TS_Ready:
            {
                //如果任务处于就绪状态
                ULONG msec = UserGetMillisecond();
//                printf("1:UserGetMillisecond return value = %lu \n",  msec);

                switch (iTaskType)
                {
                case TT_0TempWrite:
                case TT_1TempRead:
                    {
                        taskItemHeader->m_dwTaskStartTime = msec;//记录临时任务执行的起始时间
                    }
                    break;

                case TT_2PeriodShake:
                case TT_3PeriodRead:
                    {
                        bool bExpired = false;

//                        printf("100: taskItemHeader->m_dwTaskStartTime  = %lu \n",  taskItemHeader->m_dwTaskStartTime);

                        if (taskItemHeader->m_dwTaskStartTime == 0)
                        {
                            //第一次执行该周期任务
                            taskItemHeader->m_dwTaskStartTime = msec;
                            bExpired = true;
//                            printf("101: taskItemHeader->m_dwTaskStartTime  = %lu \n",  taskItemHeader->m_dwTaskStartTime);
//                          printf("init: taskItemHeader->m_dwTaskStartTime is 0 = %lu \n",  msec);
                        }
                        else
                        {
                            //再次执行该周期任务,判断时间是否已到
                            ULONG err = msec - taskItemHeader->m_dwTaskStartTime;
        //                    printf("1:msec = %d,taskItemHeader->m_usTaskSpan=%d,err= %d \n", msec, taskItemHeader->m_usTaskSpan,err);
                            if (err < 0)
                            {
                                //发生跨天现象时,23:59:59->00:00:00,err会小于零,需要特殊处理.
                                err = err + SPANDAY_SEC;
                            }

                            ULONG span = 0;

                            if (m_ucTaskErrReason == ERR_None)
                            {
                                //任务无误
                                span = taskItemHeader->m_usTaskSpan;
                            }
                            else if (m_ucTaskErrReason == ERR_TimeOut)
                            {
                                //任务超时
                                span = taskItemHeader->m_usTaskOverTime;
                            }
                            else
                            {
                                //任务错误:ERR_CRC,ERR_Data
                                span = 0;
                            }

                            if (err >= span)
                            {
   //                             printf("2:msec = %d,taskItemHeader->m_usTaskSpan=%d,err= %d \n", msec, taskItemHeader->m_usTaskSpan,err);
                                taskItemHeader->m_dwTaskStartTime = msec;
                                bExpired = true;
                            }
                        }

                        if (!bExpired)
                        {
                            //该任务的时间间隔还没到,把首部任务挪到尾部,然后跟进下一类型的任务
                            m_TaskCurrent = *taskItemHeader;
//                            printf("1: pop_front: m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr = %d, m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr = %d \n",
//                                   m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr,
//                                    m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr);
                            pop_front(m_list_TaskStorage[iTaskType]);
                            push_back(m_list_TaskStorage[iTaskType], m_TaskCurrent);
                            iTaskType++;
                            pthread_mutex_unlock(&work_mutex);

                            //为了降低CPU的占用率,以下代码判断当前线程是否需要延时
                            if ((0 == m_list_TaskStorage[TT_0TempWrite]->iAllNum)
                                    && (0 == m_list_TaskStorage[TT_1TempRead]->iAllNum))
                            {
                                //如果临时任务的链表都为空,且iSleepNum大于等于周期任务的总个数就必须延时.
                                //相当于把所有周期任务都遍历一次,再执行延时.既满足了任务实时性需要又降低了CPU占用率.
                                if (nSleepNum >= (m_list_TaskStorage[TT_2PeriodShake]->iAllNum+ m_list_TaskStorage[TT_3PeriodRead]->iAllNum))
                                {
                                    sleep(THREAD_SLEEP_SPAN);
                                    nSleepNum = 0;
                                }

                                nSleepNum++;
                            }

                            continue;
                        }
                    }
                    break;

                default:
                    break;
                }
            }
            break;

        case TS_Running:
            {
                //如果任务正在执行,请等待完成
                ULONG msec = UserGetMillisecond();
//                printf("2:UserGetMillisecond return value = %lu \n",  msec);
                int err = msec - taskItemHeader->m_dwTaskStartTime;

                if (err < 0)
                {
                    //发生跨天现象时,23:59:59->00:00:00,err会小于零,需要特殊处理.
                    err = err + SPANDAY_SEC;
                }

                if (err >= taskItemHeader->m_usTaskOverTime)
                {
                    //如果任务超时了
                    taskItemHeader->m_enTaskState = TS_Error;
                    m_ucTaskErrReason = ERR_TimeOut;
                }

                pthread_mutex_unlock(&work_mutex);
                //此处必须延时等待,否则任务会阻塞
                sleep(THREAD_SLEEP_SPAN);
                continue;
            }
            break;

        case TS_Finish:
            {
                //如果任务已经完成,先回调再删除
                //执行回调函数
                if (taskItemHeader->m_pFunTaskCallback != NULL)
                {
                    taskItemHeader->m_pFunTaskCallback(
                                              taskItemHeader->m_ulTaskID,//任务的身份标识
                                              taskItemHeader->m_ulDeviceAddr,//变频器的通信地址(串口从机地址或以太网IP地址)
                                              taskItemHeader->m_ucCommand,//命令码
                                              taskItemHeader->m_ulDataAddr,//数据地址
                                              taskItemHeader->m_ulDataNum,//读/写数据个数
                                              taskItemHeader->m_usDataBuf,//任务的数据buf,读操作用于存储接收值;写操作用于存储发送值;
                                              taskItemHeader->m_ulReserve,//保留(起预留参数的作用,以备不时之需)
                                              m_ucTaskErrReason);//任务出错的原因
                }

                switch (iTaskType)
                {
                case TT_0TempWrite:
                case TT_1TempRead:
                    {
                        //删除首部任务,链表的后续任务会自动向前挪
//                    printf("2: pop_front: m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr = %d, m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr = %d \n",
//                           m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr,
//                            m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr);

                        pop_front(m_list_TaskStorage[iTaskType]);
                    }
                    break;

                case TT_2PeriodShake:
                case TT_3PeriodRead:
                    {
                        taskItemHeader->m_ucTaskRetry = taskItemHeader->m_ucTaskRetry_Backup;//周期任务需要恢复失败重试的次数
                        m_TaskCurrent = *taskItemHeader;

                        switch (m_TaskCurrent.m_enTaskState)
                        {
                        case TS_Pause:
                        case TS_Delete:
                            //如果用户中途在回调函数改变了任务的状态(暂停/删除),必须保持这个状态
                            break;

                        default:
                            m_TaskCurrent.m_enTaskState = TS_Ready;//周期任务需要恢复状态为就绪
                            break;
                        }

                        //把首部任务挪到尾部,然后跟进下一类型的任务
//                        printf("3: pop_front: m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr = %d, m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr = %d \n",
//                               m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr,
//                                m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr);

                        pop_front(m_list_TaskStorage[iTaskType]);
                        push_back(m_list_TaskStorage[iTaskType], m_TaskCurrent);
                        iTaskType++;
                        //任务出错原因初始化
                        m_ucTaskErrReason = ERR_None;
                        //遍历链表,找出最迫切需要执行的任务,放到链表首部
                        ErgodicList(TT_3PeriodRead);
                    }
                    break;

                default:
                    break;
                }

                pthread_mutex_unlock(&work_mutex);
                continue;
            }
            break;

        case TS_Error:
            {
                //如果任务出错,需要给予重试的机会
                taskItemHeader->m_enTaskState = TS_Ready;
                pthread_mutex_unlock(&work_mutex);
                continue;
            }
            break;

        case TS_Delete:
            {
                //如果任务需要被删除,删除它,然后跟进下一类型的任务
//            printf("4: pop_front: m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr = %d, m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr = %d \n",
//                   m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr,
//                    m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr);

                pop_front(m_list_TaskStorage[iTaskType]);
                iTaskType++;
                pthread_mutex_unlock(&work_mutex);
                continue;
            }
            break;

        case TS_Pause:
            {
                //如果任务需要被暂停,跳过它,把首部任务挪到尾部,然后跟进下一类型的任务
                m_TaskCurrent = *taskItemHeader;
//                printf("5: pop_front: m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr = %d, m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr = %d \n",
//                       m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDataAddr,
//                        m_list_TaskStorage[iTaskType]->taskItem[0].m_ulDeviceAddr);

                pop_front(m_list_TaskStorage[iTaskType]);
                push_back(m_list_TaskStorage[iTaskType], m_TaskCurrent);
                iTaskType++;
                pthread_mutex_unlock(&work_mutex);
                //遍历所有任务,寻找任务链表里是否存在待执行的任务
                bool bSleep = true;

                int i = 0;
                int j = 0;
                for (i = TT_0TempWrite; i < TT_Bottom; i++)
                {
                    for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++ )
                    {
                        if (TS_Ready == m_list_TaskStorage[i]->taskItem[j].m_enTaskState)
                        {
                            //发现待执行的任务,退出for循环
                            i = TT_Bottom;
                            bSleep = false;
                            break;
                        }
                    }
                }

                //如果任务链表里全是被暂停的任务,需要延时,防止CPU被完全占用.
                if (bSleep)
                {
                    sleep(THREAD_SLEEP_SPAN);
                }

                continue;
            }
            break;

        default:
            break;
        }

        if (taskItemHeader->m_ucTaskRetry == 0)
        {
            //如果任务重试的机会已全部结束
            taskItemHeader->m_enTaskState = TS_Finish;
            pthread_mutex_unlock(&work_mutex);
            continue;
        }

        //任务出错原因初始化
        m_ucTaskErrReason = ERR_None;
        //任务重试次数减一
        taskItemHeader->m_ucTaskRetry--;
        //正在执行的任务初始化
        m_TaskCurrent = *taskItemHeader;

        //执行任务的发送
        Serial_DealWith_SendNormal();
//#ifdef DRIVE_COM1
//        //printf("echo '1' > /sys/class/gpio/gpio1/value\n");
//        system("echo '1' > /sys/class/gpio/gpio1/value");
//#else
//        system("echo '1' > /sys/class/gpio/gpio26/value");
//#endif
        //任务执行中
        //数据已发送,数据接收的回调函数开始生效
        taskItemHeader->m_enTaskState = TS_Running;
        m_TaskCurrent.m_enTaskState = TS_Running;
        pthread_mutex_unlock(&work_mutex);
    }
    return;
}

void ErgodicList(enTaskType type)//遍历链表,找出最迫切需要执行的任务,放到链表首部
{
    int i = type;

    if (m_list_TaskStorage[i]->iAllNum == 1)
    {
        //链表如果只有一个任务就无需遍历
        return;
    }

    int err = 0;
    int tmp = 0;
    int max = 0;
    ULONG msec = UserGetMillisecond();
//    printf("3:UserGetMillisecond return value = %lu \n",  msec);
    tagTaskItem taskItemWhere;

    int iIndex = 0;//记录找到的元素的下标,注意从0开始

    int j = 0;
    for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
    {
        err = msec - m_list_TaskStorage[i]->taskItem[j].m_dwTaskStartTime;

        if (err < 0)
        {
            //发生跨天现象时,23:59:59->00:00:00,err会小于零,需要特殊处理.
            err = err + SPANDAY_SEC;
        }

        tmp = err - m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan;

        if (tmp > max)
        {
            iIndex = j;
            taskItemWhere = m_list_TaskStorage[i]->taskItem[j];
            max = tmp;
        }
    }

    if (max > 0)
    {
        //已找到最迫切需要执行的任务,把它从当前所在位置挪到链表首部.
        tagTaskItem task = taskItemWhere;
        DeleteNode(m_list_TaskStorage[i], iIndex);
        push_front(m_list_TaskStorage[i], task);
    }
}

void CreateThread_Manage(void)//创建线程
{
    //新建COM连接
    int i = 0;
    for (i =0; i < TT_Bottom; i++)
    {
        m_list_TaskStorage[i] = CreateList();
    }
    m_bManagerExit = false;
    int res = pthread_mutex_init(&work_mutex, NULL);
    if ( 0 != res)
    {
        printf("Mutex init failed!");
    }
    res = pthread_create(&thread_manager, NULL, ThreadProc_Manage, NULL);
    if ( 0 != res)
    {
        printf("Thread creation failed!");
    }
}

void ExitThread_Manage(void)//退出线程
{
    m_bManagerExit = true;
}

//////////////////////////////////////////////////////////////////////////////////////
//
//                               用户函数接口
//
//////////////////////////////////////////////////////////////////////////////////////
ULONG UserGetMillisecond(void)//得到当前的系统时间
{
    time_t nowtime;
    struct tm *timeinfo;
    time( &nowtime );
    timeinfo = localtime( &nowtime );

    struct   timeval   start;
     gettimeofday(&start,0);

    //把"时,分,秒"统一换算成"毫秒"
    ULONG msec = (ULONG)(start.tv_usec /1000)+ (timeinfo->tm_sec + (timeinfo->tm_min + timeinfo->tm_hour * 60) * 60) * 1000;

//    printf("UserGetMillisecond return value = %lu \n",  msec);

    return msec;
}

//用户手动新增任务
bool UserAddTask(tagTaskItem item)
{
    //1.任务属性的容错判断
    switch (item.m_enTaskType)
    {
    case TT_0TempWrite:
    case TT_1TempRead:
        //临时任务的时间间隔恒等于0
        item.m_usTaskSpan = 0;
        break;

    case TT_2PeriodShake:
    case TT_3PeriodRead:

        //周期任务的时间间隔不建议为0
        if (item.m_usTaskSpan == 0)
        {
            return false;
        }

        break;

    default:
        //异常的任务类型,返回
        return false;
    }

    if (item.m_enTaskState != TS_Ready)
    {
        //任务状态的初始值必须为TS_Ready
        return false;
    }

    if (item.m_dwTaskStartTime != 0)
    {
        //任务上一次执行的起始时间的初始值必须为0
        return false;
    }

    if (item.m_usTaskOverTime == 0)
    {
        //任务超时的许可范围不能为0
        return false;
    }

    if (item.m_ucTaskRetry == 0)
    {
        //任务失败重试的次数不能为0
        return false;
    }

    //备份任务失败重试的次数
    item.m_ucTaskRetry_Backup = item.m_ucTaskRetry;

    switch (item.m_ucCommand)
    {
    case SERIAL_COMMAND_READ:
        if (item.m_ulDataNum > 16)
        {
            //MODBUS协议最多能连续读16个字(==32个字节)
            return false;
        }

        break;

    case SERIAL_COMMAND_WRITE_ONE:
        if (item.m_ulDataNum != 1)
        {
            //MODBUS协议写一个字
            return false;
        }

        break;

    case SERIAL_COMMAND_WRITE_MULTI:
        if (item.m_ulDataNum > 16)
        {
            //MODBUS协议写多个字,最多16个字(==32个字节)
            return false;
        }

        break;

    default:
        //异常的命令码,返回
        return false;
    }
//***************//

    //2.判断任务是否重复
//    pthread_mutex_lock(&work_mutex);

    int  i = 0;
    int j = 0;
    for (i = TT_0TempWrite; i < TT_Bottom; i++)
    {
        for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
        {
            if ((m_list_TaskStorage[i]->taskItem[j].m_ulTaskID == item.m_ulTaskID)
                && (m_list_TaskStorage[i]->taskItem[j].m_ulDeviceAddr == item.m_ulDeviceAddr))
            {
                //如果发现该任务已经存在
                if (m_list_TaskStorage[i]->taskItem[j].m_enTaskState == TS_Pause
                        || m_list_TaskStorage[i]->taskItem[j].m_enTaskState == TS_Delete)
                {
                    //如果该任务状态是暂停或待删除,把它置为就绪
                    m_list_TaskStorage[i]->taskItem[j].m_enTaskState = TS_Ready;
                }

//                pthread_mutex_unlock(&work_mutex);
                return true;//返回真,表示任务添加成功或该任务已存在
            }
        }
    }

    //3.任务正确,可添加
    int iTaskType = item.m_enTaskType;
    push_back( m_list_TaskStorage[iTaskType], item);       //m_list_TaskStorage
//    pthread_mutex_unlock(&work_mutex);
    return true;//返回真,表示任务添加成功或该任务已存在
}

bool UserModifyTask(tagUserAction action, USHORT val)//用户手动修改任务
{
    //容错判断
    switch (action.para)
    {
    case PA_TaskState://修改任务的状态(删除TS_Delete,暂停TS_Pause,重启TS_Ready)
        {
            enTaskState s = (enTaskState)val;

            if (s != TS_Delete && s != TS_Pause && s != TS_Ready)
            {
                return false;
            }
        }
        break;

    case PA_TaskSpan://修改任务的周期
        {
            if (val == 0)
            {
                //周期任务的时间间隔不建议为0
                return false;
            }
        }
        break;

    case PA_TaskOvertime://修改任务的超时时间
        {
            if (val == 0)
            {
                //任务超时的许可范围不能为0
                return false;
            }
        }
        break;

    case PA_TaskRetry://修改任务的失败重试次数
        {
            if (val == 0)
            {
                //任务失败重试的次数不能为0
                return false;
            }
        }
        break;

    default:
        return false;
    }

    printf("1:UserModifyTask \n");
//    pthread_mutex_lock(&work_mutex);
    printf("2:UserModifyTask \n");

    switch (action.way)
    {
    case MF_All://针对所有
        {
        int i = 0;
        int j = 0;
            for (i = TT_0TempWrite; i < TT_Bottom; i++)
            {
                for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
                {
                    switch (action.para)
                    {
                    case PA_TaskState://修改任务的状态(删除TS_Delete,暂停TS_Pause,重启TS_Ready)
                        if (val == TS_Pause && m_list_TaskStorage[i]->taskItem[j].m_enTaskType <= TT_1TempRead)
                        {
                            //临时任务不能设置为暂停
                            break;
                        }

                        m_list_TaskStorage[i]->taskItem[j].m_enTaskState = (enTaskState)val;
                        break;

                    case PA_TaskSpan://修改任务的周期
                        m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan = val;
                        break;

                    case PA_TaskOvertime://修改任务的超时时间
                        m_list_TaskStorage[i]->taskItem[j].m_usTaskOverTime = val;
                        break;

                    case PA_TaskRetry://修改任务的失败重试次数
                        m_list_TaskStorage[i]->taskItem[j].m_ucTaskRetry_Backup = (UCHAR)val;
                        break;

                    default:
                        break;
                    }
                }
            }
        }
        break;

    case MF_Addr_Type://针对指定地址(串口从机地址或以太网IP地址)和指定任务类型
        {
        int  i = 0;
        int j = 0;
            for (i = TT_0TempWrite; i < TT_Bottom; i++)
            {
                if (action.type != i)
                {
                    continue;
                }

                for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
                {
                    if (m_list_TaskStorage[i]->taskItem[j].m_ulDeviceAddr == action.addr)
                    {
                        switch (action.para)
                        {
                        case PA_TaskState:
                            if (val == TS_Pause && m_list_TaskStorage[i]->taskItem[j].m_enTaskType <= TT_1TempRead)
                            {
                                //临时任务不能设置为暂停
                                break;
                            }

                            m_list_TaskStorage[i]->taskItem[j].m_enTaskState = (enTaskState)val;
                            break;

                        case PA_TaskSpan:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan = val;
                            break;

                        case PA_TaskOvertime:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskOverTime = val;
                            break;

                        case PA_TaskRetry:
                            m_list_TaskStorage[i]->taskItem[j].m_ucTaskRetry_Backup = (UCHAR)val;
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
        break;

    case MF_Addr_AllId://针对指定地址(串口从机地址或以太网IP地址)和所有身份标记
        {
        int i = 0;
        int j = 0;
            for (i = TT_0TempWrite; i < TT_Bottom; i++)
            {
                for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
                {
                    if (m_list_TaskStorage[i]->taskItem[j].m_ulDeviceAddr == action.addr)
                    {
                        switch (action.para)
                        {
                        case PA_TaskState:
                            if (val == TS_Pause && m_list_TaskStorage[i]->taskItem[j].m_enTaskType <= TT_1TempRead)
                            {
                                //临时任务不能设置为暂停
                                break;
                            }

                            m_list_TaskStorage[i]->taskItem[j].m_enTaskState = (enTaskState)val;
                            break;

                        case PA_TaskSpan:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan = val;
                            break;

                        case PA_TaskOvertime:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskOverTime = val;
                            break;

                        case PA_TaskRetry:
                            m_list_TaskStorage[i]->taskItem[j].m_ucTaskRetry_Backup = (UCHAR)val;
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
        break;

    case MF_Addr_OneId://针对指定地址(串口从机地址或以太网IP地址)和指定身份标记(id1)
        {
        int i = 0;
        int j = 0;
            for (i = TT_0TempWrite; i < TT_Bottom; i++)
            {
                for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
                {
                    if (m_list_TaskStorage[i]->taskItem[j].m_ulTaskID == action.id1 && m_list_TaskStorage[i]->taskItem[j].m_ulDeviceAddr == action.addr)
                    {
                        switch (action.para)
                        {
                        case PA_TaskState:
                            if (val == TS_Pause && m_list_TaskStorage[i]->taskItem[j].m_enTaskType <= TT_1TempRead)
                            {
                                //临时任务不能设置为暂停
                                break;
                            }

                            m_list_TaskStorage[i]->taskItem[j].m_enTaskState = (enTaskState)val;
                            break;

                        case PA_TaskSpan:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan = val;
                            break;

                        case PA_TaskOvertime:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskOverTime = val;
                            break;

                        case PA_TaskRetry:
                            m_list_TaskStorage[i]->taskItem[j].m_ucTaskRetry_Backup = (UCHAR)val;
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
        break;

    case MF_Addr_MultiId://针对指定地址(串口从机地址或以太网IP地址)和指定区间(id1~id2)的身份标记
        {
            if (action.id2 < action.id1)
            {
//                pthread_mutex_unlock(&work_mutex);
                return false;
            }

            int i = 0;
            int j = 0;
            for (i = TT_0TempWrite; i < TT_Bottom; i++)
            {
                for (j = 0; j < m_list_TaskStorage[i]->iAllNum; j++)
                {
                    if (m_list_TaskStorage[i]->taskItem[j].m_ulTaskID >= action.id1
                            && m_list_TaskStorage[i]->taskItem[j].m_ulTaskID <= action.id2
                            && m_list_TaskStorage[i]->taskItem[j].m_ulDeviceAddr == action.addr)
                    {
                        switch (action.para)
                        {
                        case PA_TaskState:
                            if (val == TS_Pause && m_list_TaskStorage[i]->taskItem[j].m_enTaskType <= TT_1TempRead)
                            {
                                //临时任务不能设置为暂停
                                break;
                            }

                            m_list_TaskStorage[i]->taskItem[j].m_enTaskState = (enTaskState)val;
                            break;

                        case PA_TaskSpan:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskSpan = val;
                            break;

                        case PA_TaskOvertime:
                            m_list_TaskStorage[i]->taskItem[j].m_usTaskOverTime = val;
                            break;

                        case PA_TaskRetry:
                            m_list_TaskStorage[i]->taskItem[j].m_ucTaskRetry_Backup = (UCHAR)val;
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
        break;

    default:
//        pthread_mutex_unlock(&work_mutex);
        return false;
    }

//    pthread_mutex_unlock(&work_mutex);
    return true;
}
