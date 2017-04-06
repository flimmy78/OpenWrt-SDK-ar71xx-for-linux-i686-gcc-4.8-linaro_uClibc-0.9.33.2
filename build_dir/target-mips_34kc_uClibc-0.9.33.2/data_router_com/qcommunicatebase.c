#include "qcommunicatebase.h"
#include <pthread.h>
#include "common.h"
//////////////////////////////////////////////////////////////////////////////////////////
//
//                                      通信类基类
//
//////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread_Receive

int speed_arr[] = {B921600, B460800, B230400, B115200, B57600, B38400, B19200,B9600,B4800,B2400,B1200,B300,B38400,B19200,B9600,B4800,B2400,B1200,B300,};
int name_arr[] = {921600, 460800,230400,115200,57600,38400,19200,9600, 4800, 2400,1200,300,38400,19200,9600,4800,2400,1200,300,};

int SerialInit()
{

    /* 读取变量地址文件并排序 */
#ifdef DRIVE_COM1
    if(ReadAddrConfigFile(ADDR_CONFIG_PATH_COM1, EN_COM1) == -1)     //lf_debug   读取采集策略文件配置
        return -1;
#else
    if(ReadAddrConfigFile(ADDR_CONFIG_PATH_COM2, EN_COM2) == -1)
        return -1;
#endif

#ifdef SIGNAL_ADDR
    /* 读取地址列表并将地址分帧存储 */
    CreateDataFrame();
#endif

    /* 读取串口配置文件并设置串口 */
#ifdef DRIVE_COM1
    if(ReadComConfig(SERIAL_CONFIG_PATH, EN_COM1) == -1)
        return -1;
#else
    if(ReadComConfig(SERIAL_CONFIG_PATH, EN_COM2) == -1)
        return -1;
#endif

//#ifdef DEBUG_INFO
    printf("sComNum=%s\n", SerialInfo->sComNum);
    printf("iBaudRate=%d\n", SerialInfo->iBaudRate);
    printf("iDataBit=%d\n", SerialInfo->iDataBit);
    printf("iStopBit=%d\n", SerialInfo->iStopBit);
    printf("iTimeOut=%d\n", SerialInfo->iTimeout);
    printf("sVerifyBit=%s\n", SerialInfo->sVerifyBit);
    printf("iTaskTime=%d\n", SerialInfo->iTaskTime);
    printf("iResend=%d\n", SerialInfo->iResend);
    printf("iHandAddr=%d\n", SerialInfo->iHandAddr);
    printf("iHandTime=%d\n", SerialInfo->iHandTime);
    printf("iUpAllDataTime=%d\n", SerialInfo->iUpAllDataTime);
    printf("iHisDateSave=%d\n", SerialInfo->iHisDateSave);

//#endif

    return 0;
}

int OpenCom(const char* dev)
{
    int fd;
    fd = open( dev, O_RDWR);
    if (-1 == fd)
    {
      perror("Can't Open Serial Port");
      exit(1);
    }
    return fd;
}

int SetSpeed(int fd, int speed)
{
    int i;
    struct termios options;

    if (tcgetattr( fd, &options) != 0)
     {
          perror("SetSpeed:tcgetattr");
          return -1;
     }

    for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++)
    {
        if (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            if(tcsetattr(fd, TCSANOW, &options) != 0)
            {
                perror("SetSpeed:tcsetattr");
                return -1;
            }
            tcflush(fd, TCIOFLUSH);
            break;
        }
    }

    return 0;
}

int SetParity(int fd, int databits, int stopbits, int parity,int iTimeOut)
{
    struct termios options;

    if(tcgetattr( fd, &options) != 0)
    {
        perror("SetParity:tcgetattr");
        return -1;
    }

    /* 数据位 */
    options.c_cflag &= ~CSIZE ;
    switch (databits)
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Unsupported data size\n");
            return -1;
    }

    /* 停止位 */
    switch(stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr,"Unsupported stop bits\n");
            return -1;
    }

    /* 校验位 */
    switch(parity)
    {
        case 0:
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 1:
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 2:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return -1;
    }

    options.c_cflag |= CLOCAL |CREAD;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG );
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);\

    if (parity != 0)
        options.c_iflag |= INPCK;

    options.c_cc[VTIME] = iTimeOut;
    options.c_cc[VMIN] = 0;

    tcflush(fd, TCIFLUSH);

    if(tcsetattr(fd,TCSANOW, &options) !=0)
    {
        perror("SetParity:tcsetattr");
        return -1;
    }

    return 0;
}

void* Thread_Receive(void* pvoid)
{
    while(m_bExit)
    {
        int iLen = 0;
        memset(m_pBufRecvNormal, 0, MAX_LEN * sizeof(UCHAR));
       // iLen = read(fd_serail, m_pBufRecvNormal, MAX_LEN);
        int len,fs_sel;
        fd_set fs_read;
        struct timeval time;

        FD_ZERO(&fs_read);//清空串口接收端口集
        FD_SET(fd_serail,&fs_read);//设置串口接收端口集

        time.tv_sec =2;
        time.tv_usec = 0;

        fs_sel = select(fd_serail+1,&fs_read,NULL,NULL,&time);
        if(fs_sel){
            iLen = read(fd_serail,m_pBufRecvNormal,MAX_LEN);
            //return len;
            }
        if (iLen > 0)
        {
            m_pFuncRecvNormal(m_pBufRecvNormal, -1, iLen);     // 实际执行此函数   SFunCallback_RecvNormal  来接收数据
        }
        usleep(THREAD_SLEEP_SPAN);
    }
    pthread_exit(&thread_receive);
}

void QCommuSerial_init(FUNC_RECVSUCCESS pRecv)
{
    m_bExit = true;
    m_pBufRecvNormal =NULL;
    m_pBufSendNormal = NULL;
    SetBufSize_SendNormal(MAX_SENDNORMAL_BUFFER_SIZE);
    SetBufSize_RecvNormal(MAX_RECVNORMAL_BUFFER_SIZE);
    m_pFuncRecvNormal = NULL;
    m_pFuncRecvNormal = pRecv;

//    extern int pthread_create (pthread_t *__restrict __newthread,
//                   const pthread_attr_t *__restrict __attr,
//                   void *(*__start_routine) (void *),
//                   void *__restrict __arg) __THROWNL __nonnull ((1, 3));

    //新建COM连接
    pthread_create(&thread_receive, NULL, Thread_Receive, NULL);
}

void QCommuSerial_free()
{
    SetBufSize_RecvNormal(0);//删除缓冲区
    SetBufSize_SendNormal(0);//删除缓冲区
    Disconnect();
}

void SetBufSize_SendNormal(int iSendBufferSize)
{
    m_iBufSizeSendNormal = iSendBufferSize;

    if (m_pBufSendNormal != NULL)
    {
        free(m_pBufSendNormal);
        //释放原来的缓冲区
        m_pBufSendNormal = NULL;
    }

    if (iSendBufferSize == 0)
    {
        return;
    }
    m_pBufSendNormal = (char*)malloc(iSendBufferSize * sizeof(char));//建立新的缓冲区
    memset(m_pBufSendNormal, 0, iSendBufferSize * sizeof(char));
}

void SetBufSize_RecvNormal(int iRecvBufferSize)//同上
{
    m_iBufSizeRecvNormal = iRecvBufferSize;

    if (m_pBufRecvNormal != NULL)
    {
        free(m_pBufRecvNormal);
        m_pBufRecvNormal = NULL;
    }

    if (iRecvBufferSize == 0)
    {
        return;
    }

    m_pBufRecvNormal = (char	*)malloc(iRecvBufferSize * sizeof(char));
    memset(m_pBufRecvNormal, 0, iRecvBufferSize * sizeof(char));
}

bool Connect(void)
{
    /* 串口初始化 */
    if(SerialInit() == -1)
    {
        printf("Serial Init Error\n");
        exit(1);
    }

    /* 打开串口 */
    if(0 == strcmp(SerialInfo->sComNum, "COM1"))
        fd_serail = OpenCom("/dev/ttyUSB0");// "/dev/ttyUSB0"   // "/dev/ttyS0"
    else if(0 == strcmp(SerialInfo->sComNum, "COM2"))
        fd_serail = OpenCom("/dev/ttyUSB1");// "/dev/ttyUSB1"   // "/dev/ttyS1"

    /* 设置波特率 */
    if(SetSpeed(fd_serail, SerialInfo->iBaudRate) == -1)
    {
        printf("Set Speed Error\n");
        exit(1);
    }

    /* 设置数据位/停止位/校验位/超时时间 */
    if(SetParity(fd_serail, SerialInfo->iDataBit, SerialInfo->iStopBit, SerialInfo->iVerifyBit, SerialInfo->iTimeout) == -1)
    {
        printf("Set Parity Error\n");
        exit(1);
    }

//#ifdef DRIVE_COM1
//   system("echo '1'  > /sys/class/gpio/gpio1/value");
//#else
//   system("echo 1 > /sys/class/gpio/gpio26/value");
//#endif

    //清除缓冲区
    ClearComBuf();
    return true;
}

void Disconnect(void)
{
    if (-1 != fd_serail)
    {
        close(fd_serail);
        fd_serail = -1;
    }
}

void SendData(/*char*/UCHAR *buffer, int len) //发送数据
{
    //printf(" len = %d \n", len);
#ifdef lf_debug
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
#endif
    int iRet =  write(fd_serail, buffer, len);
    if (iRet == -1)
    {
        printf("sendData Err \n");//发送失败
    }
}

void ClearComBuf(void) //清除串口缓冲区
{
    tcflush(fd_serail, TCIOFLUSH);
}

int IsExist()//用户判断此通信属性是否已经存在(串口)
{
    return fd_serail;
}
