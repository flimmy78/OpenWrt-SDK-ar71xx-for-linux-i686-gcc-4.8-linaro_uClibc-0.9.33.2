#include "common.h"

#ifdef SIGNAL_ADDR
DeviceDataFrame* allDeviceDataFrame;
#endif

SerialConfig* SerialInfo;        //串口信息

//读取地址配置表,实际已经不是csv格式的
int ReadComConfig(const char* path, EN_DRIVE_NUM enDrive)
{
    SerialInfo = (SerialConfig*)malloc(sizeof(SerialConfig));
    memset(SerialInfo, 0, sizeof(SerialConfig));

    FILE* fp;
    int i, j;
    char strLine[MAX_LINE_SIZE];

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    memset(strLine, 0, MAX_LINE_SIZE);
    i = 0;
    fseek(fp, 0, SEEK_SET);
    bool bFlag =false;
    while(fgets(strLine, MAX_LINE_SIZE, fp))
    {
        if (enDrive == EN_COM1)
        {
            strcpy(SerialInfo->sComNum, "COM1");
            if (!bFlag)
            {
                 if (strstr(strLine, "RS485-1"))
                 {
                     bFlag = true;
                 }
                 continue;
            }
        }
        else
        {
            strcpy(SerialInfo->sComNum, "COM2");
            if (!bFlag)
            {
                 if (strstr(strLine, "RS485-2"))
                 {
                     bFlag = true;
                 }
                 continue;
            }
        }

        if (strstr(strLine, "Baud"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iBaudRate = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "Data"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iDataBit = atoi(result);
                // printf("****SerialInfo->iDataBit=%d\n",SerialInfo->iDataBit);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "Stop"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iStopBit = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "Parity"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                strcpy(SerialInfo->sVerifyBit, result);
                if(0 == strcmp(result, "Odd"))               //奇校验
                    SerialInfo->iVerifyBit = 1;
                else if(0 == strcmp(result, "Even"))     //偶校验
                    SerialInfo->iVerifyBit = 2;
                else                   //None                                             //无校验
                    SerialInfo->iVerifyBit = 0;
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "HisSave"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iHisDateSave = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "Timeout"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iTimeout = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "Resend"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iResend = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "HandAddr"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iHandAddr = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "HandTime"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iHandTime = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "UpAllTime"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->iUpAllDataTime = atoi(result)*60;
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "jsonType"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                 trim (result);
                 SerialInfo->ijsonType = atoi(result);
                 i++;
                break;
            }
        }
        else if (strstr(strLine, "jsonAddrKey"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                strcpy(SerialInfo->cjsonAddrKey, result);
                 i++;
                break;
            }
        }
        memset(strLine, 0, MAX_LINE_SIZE);

        if (12 == i) //读完本串口的所有配置就要结束
        {
            break;
        }
    }

    fclose(fp);
    return 0;
}

int ReadXmlFile(const char* path)
{
    FILE* fp = NULL;
    char* sEleValue = NULL;

    sEleValue = (char*)malloc(20);
    memset(sEleValue, 0, 10);

    SerialInfo = (SerialConfig*)malloc(sizeof(SerialConfig));
    memset(SerialInfo, 0, sizeof(SerialConfig));

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    /* <sComNum> 串口号 */
    GetXmlElement(fp, "sComNum", sEleValue);
    strcpy(SerialInfo->sComNum, sEleValue);

    /* <iBaudRate>  波特率 */
    GetXmlElement(fp, "iBaudRate", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iBaudRate);

    /* <iDataBit> 数据位 */
    GetXmlElement(fp, "iDataBit", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iDataBit);

    /* <iStopBit> 停止位 */
    GetXmlElement(fp, "iStopBit", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iStopBit);

    /* <sVerifyBit> 校验位 */
    GetXmlElement(fp, "sVerifyBit", sEleValue);
    strcpy(SerialInfo->sVerifyBit, sEleValue);
    if(0 == strcmp(sEleValue, "Odd"))               //奇校验
        SerialInfo->iVerifyBit = 1;
    else if(0 == strcmp(sEleValue, "Even"))     //偶校验
        SerialInfo->iVerifyBit = 2;
    else                   //None                                             //无校验
        SerialInfo->iVerifyBit = 0;

    /* <iTaskTime> 普通任务读周期时间 */
    GetXmlElement(fp, "iTaskTime", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iTaskTime);

    /* <iTimeOut> 超时时间 */
    GetXmlElement(fp, "iTimeOut", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iTimeout);

    /* <iHandAddr> 握手地址 */
    GetXmlElement(fp, "iHandAddr", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iHandAddr);

    /* <iHandTime> 握手时间 */
    GetXmlElement(fp, "iHandTime", sEleValue);
    sscanf(sEleValue, "%d", &SerialInfo->iHandTime);

    free(sEleValue);
    sEleValue = NULL;
    fclose(fp);

    return 0;
}

int GetXmlElement(FILE* fp, const char* sName, char* sValue)
{
    char sStart[64] = {0};
    char sEnd[64] = {0};
    char sSource[2048]  = {0};
    char* p1 = NULL;
    char* p2 = NULL;
    int iNameLen = 0;
    int iValueLen = 0;

    /* 重置FILE指针位置 */
    fseek(fp, 01, SEEK_SET);

    if(NULL == sName)
    {
        printf("sName is NULL\n");
        return -1;
    }

    /* 开始字符串 */
    memset(sStart, 0, 64);
    sprintf(sStart, "<%s>", sName);

    /* 结束字符串 */
    memset(sEnd, 0, 64);
    sprintf(sEnd, "</%s>", sName);

    /* 源字符串 */
    memset(sSource, 0, 2048);
    fread(sSource, 2048, 1, fp);

    /* 查找目标字符串 */
    p1 = strstr(sSource, sStart);
    if(NULL == p1)
    {
        printf("Can not find %s\n", sStart);
        return -1;
    }
    p2 = strstr(sSource, sEnd);
    if(NULL == p2)
    {
        printf("Can not find %s\n", sEnd);
        return -1;
    }

    iNameLen = strlen(sStart);
    iValueLen = p2 - p1 - iNameLen;

    /* 返回读取树数值 */
    memcpy(sValue, p1 + iNameLen, iValueLen);
    sValue[iValueLen] = '\0';

    return iValueLen;
}

#ifdef SIGNAL_ADDR
//读取地址配置表,实际已经不是csv格式的
int ReadAddrConfigFile(const char* path, EN_DRIVE_NUM enDrive)
{
    FILE* fp;
    int i, j;
    unsigned int  flag;      //flag:重复标识
    char strLine[MAX_LINE_SIZE];

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    unsigned int uVarNum ;                  //地址个数
    uVarNum = GetAddrNum(fp) ;

    if(uVarNum <1)
    {
        printf("File %s is null!\n", path);
        return -1;
    }

    if (enDrive == EN_COM1)
    {
        allDeviceConfigInfoCOM1 = (DeviceConfigInfo*)malloc(uVarNum * sizeof(DeviceConfigInfo));
        memset(allDeviceConfigInfoCOM1, 0, uVarNum * sizeof(DeviceConfigInfo));
    }
    else
    {
        allDeviceConfigInfoCOM2 = (DeviceConfigInfo*)malloc(uVarNum * sizeof(DeviceConfigInfo));
        memset(allDeviceConfigInfoCOM2, 0, uVarNum * sizeof(DeviceConfigInfo));
    }

    memset(strLine, 0, MAX_LINE_SIZE);
//    fgets(strLine, MAX_LINE_SIZE, fp);              //读取第一行：寄存器地址/参数说明/读写类型
//    for(i = 0; i < uVarNum; i++)
//    {
//        flag = 0;
//        memset(strLine, 0, MAX_LINE_SIZE);
//        if(fgets(strLine, MAX_LINE_SIZE, fp));
//        {
//            char *result = NULL;
//            int iIndex = 0;
//            result = strtok(strLine, ",");
//            while(NULL != result)
//            {
//                iIndex++;
//                result = strtok(NULL, ",");
//                if (1 == iIndex)
//                {
//                    if (enDrive == EN_COM1)
//                    {
//                        allDeviceConfigInfoCOM1[i].iDeviceID = atoi(result);
//                    }
//                    else
//                    {
//                        allDeviceConfigInfoCOM2[i].iDeviceID = atoi(result);
//                    }
//                }
//                else if (2 == iIndex)
//                {
//                    if (enDrive == EN_COM1)
//                    {
//                        allDeviceConfigInfoCOM1[i].usVarAddr = atoi(result);
//                    }
//                    else
//                    {
//                        allDeviceConfigInfoCOM2[i].usVarAddr = atoi(result);
//                    }
////                    printf("[%d].iDeviceID = %d, [%d].usVarAddr = %d \n",
////                           i,allDeviceConfigInfoCOM1[i].iDeviceID,
////                           i,allDeviceConfigInfoCOM1[i].usVarAddr);
//                    break;
//                }
//            }
//        }
//    }

    i = 0;
    fseek(fp, 0, SEEK_SET);
    while(fgets(strLine, MAX_LINE_SIZE, fp))
    {
        if (strstr(strLine, "dev_addr"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].iDeviceID = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].iDeviceID = atoi(result);
                }
                break;
            }
        }
        else if (strstr(strLine, "register"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");

                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].usVarAddr = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].usVarAddr = atoi(result);
                }

#ifdef DEBUG_INFO
                printf("[%d].iDeviceID = %d, [%d].usVarAddr = %d \n",
                       i,allDeviceConfigInfoCOM1[i].iDeviceID,
                       i,allDeviceConfigInfoCOM1[i].usVarAddr);
#endif

                i++;
                break;
            }
        }
        memset(strLine, 0, MAX_LINE_SIZE);
    }

    if (i != uVarNum)
    {
        printf("read addr num is wrong! \n");
    }

    fclose(fp);

    if (enDrive == EN_COM1)
    {
        IncreaseSort(allDeviceConfigInfoCOM1, uVarNum, enDrive);
    }
    else
    {
        IncreaseSort(allDeviceConfigInfoCOM2, uVarNum, enDrive);
    }
    return 0;
}

//获取csv文件总行数,因为配置表已经不是CSV所以该函数没有用到
unsigned int GetCsvTotalLine(FILE* fp)
{
    unsigned int i = 0;
    char strLine[MAX_LINE_SIZE];
    fseek(fp, 0, SEEK_SET);
    while(fgets(strLine, MAX_LINE_SIZE, fp))
        i++;
    fseek(fp, 0, SEEK_SET);
    return i;
}
#else
//第一个参数为输入参数，第二个为输出参数，第三个参数为输入参数个数,返回地址个数
int changeConfigContinuToSignal(DeviceMulConfigInfo* pConfig, int iNum, EN_DRIVE_NUM enDrive)
{
    int iAll = 0;
    int i = 0;
    for (i = 0 ; i < iNum; i++)
    {
        iAll += pConfig[i].iNum;
    }

    DeviceConfigInfo *pTempConfig= (DeviceConfigInfo*)malloc(sizeof(DeviceConfigInfo) * iAll);
    memset(pTempConfig, 0, sizeof(DeviceConfigInfo) * iAll);
    if (EN_COM1 == enDrive)
    {
        allDeviceCOM1 = (DeviceConfigInfo*)malloc(sizeof(DeviceConfigInfo) * iAll);
        memset(allDeviceCOM1, 0, sizeof(DeviceConfigInfo) * iAll);
    }
    else
    {
        allDeviceCOM2 = (DeviceConfigInfo*)malloc(sizeof(DeviceConfigInfo) * iAll);
        memset(allDeviceCOM2, 0, sizeof(DeviceConfigInfo) * iAll);
    }

    int j = 0;
    int k = 0;
    for (i = 0 ; i < iNum; i++)
    {
        for (j = 0; j < pConfig[i].iNum; j++)
        {
            pTempConfig[k].iDeviceID =  pConfig[i].iDeviceID;
            pTempConfig[k].usVarAddr = pConfig[i].usVarAddr + j;
            k++;
        }
    }

    if (k != iAll)
    {
        printf("changeConfigContinuToSignal:: k != iAll \n");
    }

    //找出相同的ID和地址 删除只保留一个
    int iRet = iAll;
    int iBit = 0, iflag=0;
    for (i = 0; i < iAll; i++)
    {
        int iTem = 0;
        if (EN_COM1 == enDrive)
        {
            while (allDeviceCOM1[iTem].iDeviceID !=0)
            {
                if ((allDeviceCOM1[iTem].iDeviceID == pTempConfig[i].iDeviceID)
                    && (allDeviceCOM1[iTem].usVarAddr == pTempConfig[i].usVarAddr))
                {
                    printf("lf_debug (iDeviceID == iDeviceID && usVarAddr == usVarAddr) = %d\n", ++iflag);
                    iRet--;
                    break;
                }
                iTem++;
            }
            allDeviceCOM1[iBit].iDeviceID =pTempConfig[i].iDeviceID;
            allDeviceCOM1[iBit].usVarAddr =pTempConfig[i].usVarAddr;
        }
        else
        {
            while (allDeviceCOM2[iTem].iDeviceID !=0)
            {
                if ((allDeviceCOM2[iTem].iDeviceID == pTempConfig[i].iDeviceID)
                    && (allDeviceCOM2[iTem].usVarAddr == pTempConfig[i].usVarAddr))
                {
                    iRet--;
                    break;
                }
                iTem++;
            }
            allDeviceCOM2[iBit].iDeviceID =pTempConfig[i].iDeviceID;
            allDeviceCOM2[iBit].usVarAddr =pTempConfig[i].usVarAddr;
        }
        iBit++;
    }

#ifdef DEBUG_INFO
    if (EN_COM1 == enDrive)
    {
        int iTem = 0;
        while (allDeviceCOM1[iTem].iDeviceID !=0)
        {
            printf("allDeviceCOM1[%d].iDeviceID = %d, allDeviceCOM1[%d].usVarAddr = %d \n",
                   iTem, allDeviceCOM1[iTem].iDeviceID, iTem, allDeviceCOM1[iTem].usVarAddr);
            iTem++;
        }
    }
    else
    {
        int iTem = 0;
        while (allDeviceCOM2[iTem].iDeviceID !=0)
        {
            printf("allDeviceCOM2[%d].iDeviceID = %d, allDeviceCOM2[%d].usVarAddr = %d \n",
                   iTem, allDeviceCOM2[iTem].iDeviceID, iTem, allDeviceCOM2[iTem].usVarAddr);
            iTem++;
        }
    }
#endif

    return iRet;
}

int ReadAddrConfigFile(const char* path, EN_DRIVE_NUM enDrive)
{
    FILE* fp;
    int i, j;
    unsigned int  flag;      //flag:重复标识
    char strLine[MAX_LINE_SIZE];

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    unsigned int uVarNum ;                  //地址个数
    uVarNum = GetAddrNum(fp) ;
#ifdef lf_debug
    printf("lf_debug aVarNum= %d\n", uVarNum);
 #endif
    if(uVarNum <1)
    {
        printf("File %s is null!\n", path);
        return -1;
    }

    if (enDrive == EN_COM1)
    {
        allDeviceConfigInfoCOM1 = (DeviceMulConfigInfo*)malloc(uVarNum * sizeof(DeviceMulConfigInfo));
        memset(allDeviceConfigInfoCOM1, 0, uVarNum * sizeof(DeviceMulConfigInfo));
    }
    else
    {
        allDeviceConfigInfoCOM2 = (DeviceMulConfigInfo*)malloc(uVarNum * sizeof(DeviceMulConfigInfo));
        memset(allDeviceConfigInfoCOM2, 0, uVarNum * sizeof(DeviceMulConfigInfo));
    }
    memset(strLine, 0, MAX_LINE_SIZE);
    i = 0;
    fseek(fp, 0, SEEK_SET);
    while(fgets(strLine, MAX_LINE_SIZE, fp))
    {
        if (strstr(strLine, "dev_addr"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].iDeviceID = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].iDeviceID = atoi(result);
                }
                break;
            }
        }
        else if (strstr(strLine, "register"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");

                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].usVarAddr = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].usVarAddr = atoi(result);
                }
                break;
            }
        }
        else if (strstr(strLine, "reg_num"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");

                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].iNum = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].iNum = atoi(result);
                }
                break;
            }
        }
        else if (strstr(strLine, "sampling"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");

                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].iSampleTime = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].iSampleTime = atoi(result);
                }
                break;
            }
        }
        else if (strstr(strLine, "readflag"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");

                if (enDrive == EN_COM1)
                {
                    trim (result);
                    allDeviceConfigInfoCOM1[i].readflag = atoi(result);
                }
                else
                {
                    trim (result);
                    allDeviceConfigInfoCOM2[i].readflag = atoi(result);
                }
                printf("lf_debug   [%d].iDeviceID = %d, [%d].usVarAddr = %d [%d].iNum = %d, [%d].iSampleTime = %d,  [%d].readflag=%d\n",\
                       i,allDeviceConfigInfoCOM1[i].iDeviceID,\
                       i,allDeviceConfigInfoCOM1[i].usVarAddr,\
                      i,allDeviceConfigInfoCOM1[i].iNum,\
                       i,allDeviceConfigInfoCOM1[i].iSampleTime,\
                       i,allDeviceConfigInfoCOM1[i].readflag);
                i++;
                break;
            }
        }

        memset(strLine, 0, MAX_LINE_SIZE);
    }

    if (i != uVarNum)
    {
        printf("read addr num is wrong! Path = %s, i = %d, uVarNum = %d \n",path, i, uVarNum);
    }

    fclose(fp);

    if (enDrive == EN_COM1)
    {
        iConfigCom1Num = uVarNum;
        int iNum = changeConfigContinuToSignal(allDeviceConfigInfoCOM1, uVarNum, enDrive);
        IncreaseSort(allDeviceCOM1, iNum, enDrive);    //分辨出有几台设备 将每台设备的地址依次从小到大排列好
    }
    else
    {
        iConfigCom2Num = uVarNum;
        int iNum = changeConfigContinuToSignal(allDeviceConfigInfoCOM2, uVarNum, enDrive);
        IncreaseSort(allDeviceCOM2, iNum, enDrive);
    }
    return 0;
}
#endif

//获取配置表中地址的数量
unsigned int GetAddrNum(FILE* fp)
{
    unsigned int i = 0;
    char strLine[MAX_LINE_SIZE];
    fseek(fp, 0, SEEK_SET);
    while(fgets(strLine, MAX_LINE_SIZE, fp))
    {
        if (strstr(strLine, "register"))
        {
            i++;
        }
    }
    fseek(fp, 0, SEEK_SET);
    return i;
}

#ifdef SIGNAL_ADDR
//分辨出有几台设备 将每台设备的地址依次从小到大排列好
int IncreaseSort(DeviceConfigInfo* deviceInfo, unsigned num, EN_DRIVE_NUM enDrive)
{
    //找出配置表中有几台设备
    int iDeviceID[MAX_DEV_NUM];
    memset(iDeviceID, 0, sizeof(int) * MAX_DEV_NUM);
    int i = 0;
    int iDeviceIDNum = 0;
    for(i = 0; i < num; i++)
    {
        int j = 0;
        bool bHave =false;

        while (0 != iDeviceID[j] )
        {
            if (iDeviceID[j] == deviceInfo[i].iDeviceID)
            {
                bHave = true;
                break;
            }
            j++;
        }
        if (!bHave)
        {
            iDeviceID[j] = deviceInfo[i].iDeviceID;
            printf("iDeviceID[j] = %d, j = %d \n", iDeviceID[j], j);
            iDeviceIDNum++;
        }
    }

    if (enDrive == EN_COM1)
    {
        drive_data[EN_COM1].iDeviceNum = iDeviceIDNum;
        drive_data[EN_COM1].idDriver = 0;
        printf("drive_data[EN_COM1].iDeviceNum = %d \n", drive_data[EN_COM1].iDeviceNum);
    }
    else
    {
        drive_data[EN_COM2].iDeviceNum = iDeviceIDNum;
        drive_data[EN_COM2].idDriver = 0;
        printf("drive_data[EN_COM2].iDeviceNum = %d \n", drive_data[EN_COM2].iDeviceNum);
    }

    //对找出的设备ID号进行排序,从小到大排序
    int j, k,tmp;
    if (enDrive == EN_COM1)
    {
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum - 1; i++)
        {
            k = i;
            for (j = i + 1; j < drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (iDeviceID[k] > iDeviceID[j])
                {
                    k = j;
                }
            }

            if (i !=k)
            {
                tmp = iDeviceID[i];
                iDeviceID[i] = iDeviceID[k];
                iDeviceID[k] = tmp;
            }
        }
    }
    else
    {
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum - 1; i++)
        {
            k = i;
            for (j = i + 1; j < drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (iDeviceID[k] > iDeviceID[j])
                {
                    k = j;
                }
            }

            if (i !=k)
            {
                tmp = iDeviceID[i];
                iDeviceID[i] = iDeviceID[k];
                iDeviceID[k] = tmp;
            }
        }
    }


    //找出每台的机器的读取地址数量
    if (enDrive == EN_COM1)
    {
        deviceNumInfoCOM1 = (SignalDeviceInfo*)malloc(sizeof(SignalDeviceInfo) * drive_data[EN_COM1].iDeviceNum ) ;
        memset(deviceNumInfoCOM1, 0, sizeof(SignalDeviceInfo) * drive_data[EN_COM1].iDeviceNum);
    }
    else
    {
        deviceNumInfoCOM2 = (SignalDeviceInfo*)malloc(sizeof(SignalDeviceInfo) * drive_data[EN_COM2].iDeviceNum ) ;
        memset(deviceNumInfoCOM2, 0, sizeof(SignalDeviceInfo) * drive_data[EN_COM2].iDeviceNum);
    }


    printf("iDeviceID[0] = %d \n", iDeviceID[0]);
    for(i = 0; i < num; i++)
    {
        int j = 0;
        if (enDrive == EN_COM1)
        {
            for (j = 0;  j <  drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (iDeviceID[j] == deviceInfo[i].iDeviceID)
                {
                    deviceNumInfoCOM1[j].iDeviceID = deviceInfo[i].iDeviceID;
                    deviceNumInfoCOM1[j].iNum++;
                    break;
                }
            }
        }
        else
        {
            for (j = 0;  j <  drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (iDeviceID[j] == deviceInfo[i].iDeviceID)
                {
                    deviceNumInfoCOM2[j].iDeviceID = deviceInfo[i].iDeviceID;
                    deviceNumInfoCOM2[j].iNum++;
                    break;
                }
            }
        }
    }
    //将每台机器的读取地址分别保存，并按地址从小到大排列
    if (enDrive == EN_COM1)
    {
        drive_data[EN_COM1].deviceInfo = (SignalDevice*)malloc(sizeof(SignalDevice) * drive_data[EN_COM1].iDeviceNum);
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++ )
        {
            drive_data[EN_COM1].deviceInfo[i].deviceData= (DeviceDataInfo *)malloc(sizeof(DeviceDataInfo) * deviceNumInfoCOM1[i].iNum);
            memset(drive_data[EN_COM1].deviceInfo[i].deviceData, 0, sizeof(DeviceDataInfo) * deviceNumInfoCOM1[i].iNum);
            drive_data[EN_COM1].deviceInfo[i].iDeviceID = deviceNumInfoCOM1[i].iDeviceID;
            drive_data[EN_COM1].deviceInfo[i].iDeviceNum = deviceNumInfoCOM1[i].iNum;
        }
    }
    else
    {
        drive_data[EN_COM2].deviceInfo = (SignalDevice*)malloc(sizeof(SignalDevice) * drive_data[EN_COM2].iDeviceNum);
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++ )
        {
            drive_data[EN_COM2].deviceInfo[i].deviceData= (DeviceDataInfo *)malloc(sizeof(DeviceDataInfo) * deviceNumInfoCOM2[i].iNum);
            memset(drive_data[EN_COM2].deviceInfo[i].deviceData, 0, sizeof(DeviceDataInfo) * deviceNumInfoCOM2[i].iNum);
            drive_data[EN_COM2].deviceInfo[i].iDeviceID = deviceNumInfoCOM2[i].iDeviceID;
            drive_data[EN_COM2].deviceInfo[i].iDeviceNum = deviceNumInfoCOM2[i].iNum;
        }
    }


    int* iIndex;
    if (enDrive == EN_COM1)
    {
        iIndex = (int*)malloc(sizeof(int) * drive_data[EN_COM1].iDeviceNum);
        memset(iIndex, 0, sizeof(int) * drive_data[EN_COM1].iDeviceNum);
    }
    else
    {
        iIndex = (int*)malloc(sizeof(int) * drive_data[EN_COM2].iDeviceNum);
        memset(iIndex, 0, sizeof(int) * drive_data[EN_COM2].iDeviceNum);
    }


    for(i = 0; i < num; i++)
    {
        int j = 0;
        if (enDrive == EN_COM1)
        {
            for (j = 0;  j <  drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (deviceNumInfoCOM1[j].iDeviceID == deviceInfo[i].iDeviceID)
                {
                    drive_data[EN_COM1].deviceInfo[j].deviceData[iIndex[j]].usVarAddr = deviceInfo[i].usVarAddr;
//                    drive_data[EN_COM1].deviceInfo[j].deviceData[iIndex[j]].sVarValue = deviceInfo[i].sVarValue;
                    iIndex[j]++;
                    break;
                }
            }
        }
        else
        {
            for (j = 0;  j <  drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (deviceNumInfoCOM2[j].iDeviceID == deviceInfo[i].iDeviceID)
                {
                    drive_data[EN_COM2].deviceInfo[j].deviceData[iIndex[j]].usVarAddr = deviceInfo[i].usVarAddr;
//                    drive_data[EN_COM2].deviceInfo[j].deviceData[iIndex[j]].sVarValue = deviceInfo[i].sVarValue;
                    iIndex[j]++;
                    break;
                }
            }
        }
    }
    //对每台地址从小到大排序
    DeviceDataInfo temp;
    int m = 0;
    if (enDrive == EN_COM1)
    {
        for (m= 0;  m <  drive_data[EN_COM1].iDeviceNum; m++)
        {
            for (i = 0; i < deviceNumInfoCOM1[m].iNum - 1; i++)
            {
                k = i;
                for (j = i + 1; j < deviceNumInfoCOM1[m].iNum; j++)
                {
                    if (drive_data[EN_COM1].deviceInfo[m].deviceData[k].usVarAddr > drive_data[EN_COM1].deviceInfo[m].deviceData[j].usVarAddr)
                    {
                        k = j;
                    }
                }

                if (i !=k)
                {
                    temp = drive_data[EN_COM1].deviceInfo[m].deviceData[i];
                    drive_data[EN_COM1].deviceInfo[m].deviceData[i] = drive_data[EN_COM1].deviceInfo[m].deviceData[k];
                    drive_data[EN_COM1].deviceInfo[m].deviceData[k] = temp;
                }
            }
        }
    }
    else
    {
        for (m= 0;  m <  drive_data[EN_COM2].iDeviceNum; m++)
        {
            for (i = 0; i < deviceNumInfoCOM2[m].iNum - 1; i++)
            {
                k = i;
                for (j = i + 1; j < deviceNumInfoCOM2[m].iNum; j++)
                {
                    if (drive_data[EN_COM2].deviceInfo[m].deviceData[k].usVarAddr > drive_data[EN_COM2].deviceInfo[m].deviceData[j].usVarAddr)
                    {
                        k = j;
                    }
                }

                if (i !=k)
                {
                    temp = drive_data[EN_COM2].deviceInfo[m].deviceData[i];
                    drive_data[EN_COM2].deviceInfo[m].deviceData[i] = drive_data[EN_COM2].deviceInfo[m].deviceData[k];
                    drive_data[EN_COM2].deviceInfo[m].deviceData[k] = temp;
                }
            }
        }
    }

    if (enDrive == EN_COM1)
    {
        for (m= 0;  m <  drive_data[EN_COM1].iDeviceNum; m++)
        {
            printf("drive_data[EN_COM1].iDeviceNum = %d , deviceNumInfoCOM1[%d].iNum = %d\n",
                   drive_data[EN_COM1].iDeviceNum, m, deviceNumInfoCOM1[m].iNum);

            for (i = 0; i < drive_data[EN_COM1].deviceInfo[m].iDeviceNum; i++)
            {
                printf("drive_data[EN_COM1].deviceInfo[%d].iDeviceID = %d \n", m, drive_data[EN_COM1].deviceInfo[m].iDeviceID);
                printf("drive_data[EN_COM1].deviceInfo[%d].deviceData[%d].usVarAddr = %d \n",
                       m, i ,drive_data[EN_COM1].deviceInfo[m].deviceData[i].usVarAddr);
            }
        }
    }
    else
    {
        for (m= 0;  m <  drive_data[EN_COM2].iDeviceNum; m++)
        {
            printf("drive_data[EN_COM2].iDeviceNum = %d , deviceNumInfoCOM2[%d].iNum = %d\n",
                   drive_data[EN_COM2].iDeviceNum, m, deviceNumInfoCOM2[m].iNum);

            for (i = 0; i < drive_data[EN_COM2].deviceInfo[m].iDeviceNum; i++)
            {
                printf("drive_data[EN_COM2].deviceInfo[%d].iDeviceID = %d \n", m, drive_data[EN_COM2].deviceInfo[m].iDeviceID);
                printf("drive_data[EN_COM2].deviceInfo[%d].deviceData[%d].usVarAddr = %d \n",
                       m, i ,drive_data[EN_COM2].deviceInfo[m].deviceData[i].usVarAddr);
            }
        }
    }
    return 0;
}
#else
int IncreaseSort(DeviceConfigInfo* deviceInfo, unsigned num, EN_DRIVE_NUM enDrive)
{
    //找出配置表中有几台设备
    int iDeviceID[MAX_DEV_NUM];//254
    memset(iDeviceID, 0, sizeof(int) * MAX_DEV_NUM);
    int i = 0;
    int iDeviceIDNum = 0;
    for(i = 0; i < num; i++)
    {
        int j = 0;
        bool bHave =false;

        while (0 != iDeviceID[j] )
        {
            if (iDeviceID[j] == deviceInfo[i].iDeviceID)
            {
                bHave = true;
                break;
            }
            j++;
        }
        if (!bHave)
        {
            iDeviceID[j] = deviceInfo[i].iDeviceID;

#ifdef DEBUG_INFO
            printf("iDeviceID[j] = %d, j = %d \n", iDeviceID[j], j);
#endif

            iDeviceIDNum++;
        }
    }

    if (enDrive == EN_COM1)
    {
        drive_data[EN_COM1].iDeviceNum = iDeviceIDNum;    //确定设备台数赋值 给全局变量
        drive_data[EN_COM1].idDriver = 0;

#ifdef DEBUG_INFO
        printf("drive_data[EN_COM1].iDeviceNum = %d \n", drive_data[EN_COM1].iDeviceNum);
#endif
    }
    else
    {
        drive_data[EN_COM2].iDeviceNum = iDeviceIDNum;
        drive_data[EN_COM2].idDriver = 0;

#ifdef DEBUG_INFO
        printf("drive_data[EN_COM2].iDeviceNum = %d \n", drive_data[EN_COM2].iDeviceNum);
#endif
    }

    //对找出的设备ID号进行排序,从小到大排序
    int j, k,tmp;
    if (enDrive == EN_COM1)
    {
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum - 1; i++)
        {
            k = i;
            for (j = i + 1; j < drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (iDeviceID[k] > iDeviceID[j])
                {
                    k = j;
                }
            }

            if (i !=k)
            {
                tmp = iDeviceID[i];
                iDeviceID[i] = iDeviceID[k];
                iDeviceID[k] = tmp;
            }
        }
    }
    else
    {
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum - 1; i++)
        {
            k = i;
            for (j = i + 1; j < drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (iDeviceID[k] > iDeviceID[j])
                {
                    k = j;
                }
            }

            if (i !=k)
            {
                tmp = iDeviceID[i];
                iDeviceID[i] = iDeviceID[k];
                iDeviceID[k] = tmp;
            }
        }
    }

    //找出每台的机器的读取地址数量
    if (enDrive == EN_COM1)
    {
        deviceNumInfoCOM1 = (SignalDeviceInfo*)malloc(sizeof(SignalDeviceInfo) * drive_data[EN_COM1].iDeviceNum ) ;
        memset(deviceNumInfoCOM1, 0, sizeof(SignalDeviceInfo) * drive_data[EN_COM1].iDeviceNum);
    }
    else
    {
        deviceNumInfoCOM2 = (SignalDeviceInfo*)malloc(sizeof(SignalDeviceInfo) * drive_data[EN_COM2].iDeviceNum ) ;
        memset(deviceNumInfoCOM2, 0, sizeof(SignalDeviceInfo) * drive_data[EN_COM2].iDeviceNum);
    }

#ifdef DEBUG_INFO
    printf("iDeviceID[0] = %d \n", iDeviceID[0]);
#endif

    for(i = 0; i < num; i++)
    {
        int j = 0;
        if (enDrive == EN_COM1)
        {
            for (j = 0;  j <  drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (iDeviceID[j] == deviceInfo[i].iDeviceID)
                {
                    deviceNumInfoCOM1[j].iDeviceID = deviceInfo[i].iDeviceID;
                    deviceNumInfoCOM1[j].iNum++;
                    break;
                }
            }
        }
        else
        {
            for (j = 0;  j <  drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (iDeviceID[j] == deviceInfo[i].iDeviceID)
                {
                    deviceNumInfoCOM2[j].iDeviceID = deviceInfo[i].iDeviceID;
                    deviceNumInfoCOM2[j].iNum++;
                    break;
                }
            }
        }
    }
    //将每台机器的读取地址分别保存，并按地址从小到大排列
    if (enDrive == EN_COM1)
    {
        drive_data[EN_COM1].deviceInfo = (SignalDevice*)malloc(sizeof(SignalDevice) * drive_data[EN_COM1].iDeviceNum);
        for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++ )
        {
            drive_data[EN_COM1].deviceInfo[i].deviceData= (DeviceDataInfo *)malloc(sizeof(DeviceDataInfo) * deviceNumInfoCOM1[i].iNum);
            memset(drive_data[EN_COM1].deviceInfo[i].deviceData, 0, sizeof(DeviceDataInfo) * deviceNumInfoCOM1[i].iNum);
            drive_data[EN_COM1].deviceInfo[i].iDeviceID = deviceNumInfoCOM1[i].iDeviceID;
            drive_data[EN_COM1].deviceInfo[i].iDeviceNum = deviceNumInfoCOM1[i].iNum;
        }
    }
    else
    {
        drive_data[EN_COM2].deviceInfo = (SignalDevice*)malloc(sizeof(SignalDevice) * drive_data[EN_COM2].iDeviceNum);
        for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++ )
        {
            drive_data[EN_COM2].deviceInfo[i].deviceData= (DeviceDataInfo *)malloc(sizeof(DeviceDataInfo) * deviceNumInfoCOM2[i].iNum);
            memset(drive_data[EN_COM2].deviceInfo[i].deviceData, 0, sizeof(DeviceDataInfo) * deviceNumInfoCOM2[i].iNum);
            drive_data[EN_COM2].deviceInfo[i].iDeviceID = deviceNumInfoCOM2[i].iDeviceID;
            drive_data[EN_COM2].deviceInfo[i].iDeviceNum = deviceNumInfoCOM2[i].iNum;
        }
    }


    int* iIndex;
    if (enDrive == EN_COM1)
    {
        iIndex = (int*)malloc(sizeof(int) * drive_data[EN_COM1].iDeviceNum);
        memset(iIndex, 0, sizeof(int) * drive_data[EN_COM1].iDeviceNum);
    }
    else
    {
        iIndex = (int*)malloc(sizeof(int) * drive_data[EN_COM2].iDeviceNum);
        memset(iIndex, 0, sizeof(int) * drive_data[EN_COM2].iDeviceNum);
    }


    for(i = 0; i < num; i++)
    {
        int j = 0;
        if (enDrive == EN_COM1)
        {
            for (j = 0;  j <  drive_data[EN_COM1].iDeviceNum; j++)
            {
                if (deviceNumInfoCOM1[j].iDeviceID == deviceInfo[i].iDeviceID)
                {
                    drive_data[EN_COM1].deviceInfo[j].deviceData[iIndex[j]].usVarAddr = deviceInfo[i].usVarAddr;
//                    drive_data[EN_COM1].deviceInfo[j].deviceData[iIndex[j]].sVarValue = deviceInfo[i].sVarValue;
                    iIndex[j]++;
                    break;
                }
            }
        }
        else
        {
            for (j = 0;  j <  drive_data[EN_COM2].iDeviceNum; j++)
            {
                if (deviceNumInfoCOM2[j].iDeviceID == deviceInfo[i].iDeviceID)
                {
                    drive_data[EN_COM2].deviceInfo[j].deviceData[iIndex[j]].usVarAddr = deviceInfo[i].usVarAddr;
//                    drive_data[EN_COM2].deviceInfo[j].deviceData[iIndex[j]].sVarValue = deviceInfo[i].sVarValue;
                    iIndex[j]++;
                    break;
                }
            }
        }
    }
    //对每台地址从小到大排序
    DeviceDataInfo temp;
    int m = 0;
    if (enDrive == EN_COM1)
    {
        for (m= 0;  m <  drive_data[EN_COM1].iDeviceNum; m++)
        {
            for (i = 0; i < deviceNumInfoCOM1[m].iNum - 1; i++)
            {
                k = i;
                for (j = i + 1; j < deviceNumInfoCOM1[m].iNum; j++)
                {
                    if (drive_data[EN_COM1].deviceInfo[m].deviceData[k].usVarAddr > drive_data[EN_COM1].deviceInfo[m].deviceData[j].usVarAddr)
                    {
                        k = j;
                    }
                }

                if (i !=k)
                {
                    temp = drive_data[EN_COM1].deviceInfo[m].deviceData[i];
                    drive_data[EN_COM1].deviceInfo[m].deviceData[i] = drive_data[EN_COM1].deviceInfo[m].deviceData[k];
                    drive_data[EN_COM1].deviceInfo[m].deviceData[k] = temp;
                }
            }
        }
    }
    else
    {
        for (m= 0;  m <  drive_data[EN_COM2].iDeviceNum; m++)
        {
            for (i = 0; i < deviceNumInfoCOM2[m].iNum - 1; i++)
            {
                k = i;
                for (j = i + 1; j < deviceNumInfoCOM2[m].iNum; j++)
                {
                    if (drive_data[EN_COM2].deviceInfo[m].deviceData[k].usVarAddr > drive_data[EN_COM2].deviceInfo[m].deviceData[j].usVarAddr)
                    {
                        k = j;
                    }
                }

                if (i !=k)
                {
                    temp = drive_data[EN_COM2].deviceInfo[m].deviceData[i];
                    drive_data[EN_COM2].deviceInfo[m].deviceData[i] = drive_data[EN_COM2].deviceInfo[m].deviceData[k];
                    drive_data[EN_COM2].deviceInfo[m].deviceData[k] = temp;
                }
            }
        }
    }

#ifdef DEBUG_INFO
    if (enDrive == EN_COM1)
    {
        for (m= 0;  m <  drive_data[EN_COM1].iDeviceNum; m++)
        {
            printf("drive_data[EN_COM1].iDeviceNum = %d , deviceNumInfoCOM1[%d].iNum = %d\n",
                   drive_data[EN_COM1].iDeviceNum, m, deviceNumInfoCOM1[m].iNum);

            for (i = 0; i < drive_data[EN_COM1].deviceInfo[m].iDeviceNum; i++)
            {
                printf("drive_data[EN_COM1].deviceInfo[%d].iDeviceID = %d \n", m, drive_data[EN_COM1].deviceInfo[m].iDeviceID);
                printf("drive_data[EN_COM1].deviceInfo[%d].deviceData[%d].usVarAddr = %d \n",
                       m, i ,drive_data[EN_COM1].deviceInfo[m].deviceData[i].usVarAddr);
            }
        }
    }
    else
    {
        for (m= 0;  m <  drive_data[EN_COM2].iDeviceNum; m++)
        {
            printf("drive_data[EN_COM2].iDeviceNum = %d , deviceNumInfoCOM2[%d].iNum = %d\n",
                   drive_data[EN_COM2].iDeviceNum, m, deviceNumInfoCOM2[m].iNum);

            for (i = 0; i < drive_data[EN_COM2].deviceInfo[m].iDeviceNum; i++)
            {
                printf("drive_data[EN_COM2].deviceInfo[%d].iDeviceID = %d \n", m, drive_data[EN_COM2].deviceInfo[m].iDeviceID);
                printf("drive_data[EN_COM2].deviceInfo[%d].deviceData[%d].usVarAddr = %d \n",
                       m, i ,drive_data[EN_COM2].deviceInfo[m].deviceData[i].usVarAddr);
            }
        }
    } 12 1c 00 01 40 b
#endif
    return 0;
}
#endif

/* 获取单台设备创建查询帧的数目 */
unsigned int GetFrameNum(SignalDevice src)
{
    int i, tmp;
    unsigned int k = 1;

    tmp = src.deviceData[0].usVarAddr;
    for(i = 0; i < src.iDeviceNum; i++)
    {
        if((src.deviceData[i].usVarAddr - tmp) >= MAX_READ_LEN)
        {
            k++;
            tmp = src.deviceData[i].usVarAddr;
        }
    }

    return k;
}

#ifdef SIGNAL_ADDR
void CreateDataFrame()
{
#ifdef DRIVE_COM1
    unsigned int i;
    allDeviceDataFrame = (DeviceDataFrame*)malloc(sizeof(DeviceDataFrame) * drive_data[EN_COM1].iDeviceNum);
    for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
    {
        int iNum = GetFrameNum(drive_data[EN_COM1].deviceInfo[i]);
        allDeviceDataFrame[i].dataFrame = (SignalDataFrame*)malloc(sizeof(SignalDataFrame) * iNum);
        memset(allDeviceDataFrame[i].dataFrame, 0 , sizeof(SignalDataFrame) * iNum);
        allDeviceDataFrame[i].iDeviceID = drive_data[EN_COM1].deviceInfo[i].iDeviceID;
        allDeviceDataFrame[i].iFrameNum = iNum;

        int m, tmp;
        int iIndex = 0;

        tmp = drive_data[EN_COM1].deviceInfo[i].deviceData[0].usVarAddr;
        allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = 1;

        for(m = 0; m < drive_data[EN_COM1].deviceInfo[i].iDeviceNum; m++)
        {
            if((drive_data[EN_COM1].deviceInfo[i].deviceData[m].usVarAddr - tmp) >= MAX_READ_LEN)
            {
                allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = drive_data[EN_COM1].deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
                iIndex++;
                tmp = drive_data[EN_COM1].deviceInfo[i].deviceData[m].usVarAddr;
                allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
            }
        }
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = drive_data[EN_COM1].deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
    }

    for (i = 0; i < drive_data[EN_COM1].iDeviceNum; i++)
    {
        int j = 0;
        printf("allDeviceDataFrame[%d].iDeviceID= %d \n", i, allDeviceDataFrame[i].iDeviceID);
        printf("allDeviceDataFrame[%d].iFrameNum= %d \n", i, allDeviceDataFrame[i].iFrameNum);

        for (j = 0; j < allDeviceDataFrame[i].iFrameNum; j++)
        {
            printf("allDeviceDataFrame[%d].dataFrame[%d].usAddrStart = %d \n", i, j, allDeviceDataFrame[i].dataFrame[j].usAddrStart);
            printf("allDeviceDataFrame[%d].dataFrame[%d].ucDataNum = %d \n", i, j, allDeviceDataFrame[i].dataFrame[j].ucDataNum);
        }
    }
#else
    unsigned int i;
    allDeviceDataFrame = (DeviceDataFrame*)malloc(sizeof(DeviceDataFrame) * drive_data[EN_COM2].iDeviceNum);
    for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
    {
        int iNum = GetFrameNum(drive_data[EN_COM2].deviceInfo[i]);
        allDeviceDataFrame[i].dataFrame = (SignalDataFrame*)malloc(sizeof(SignalDataFrame) * iNum);
        memset(allDeviceDataFrame[i].dataFrame, 0 , sizeof(SignalDataFrame) * iNum);
        allDeviceDataFrame[i].iDeviceID = drive_data[EN_COM2].deviceInfo[i].iDeviceID;
        allDeviceDataFrame[i].iFrameNum = iNum;

        int m, tmp;
        int iIndex = 0;

        tmp = drive_data[EN_COM2].deviceInfo[i].deviceData[0].usVarAddr;
        allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = 1;

        for(m = 0; m < drive_data[EN_COM2].deviceInfo[i].iDeviceNum; m++)
        {
            if((drive_data[EN_COM2].deviceInfo[i].deviceData[m].usVarAddr - tmp) >= MAX_READ_LEN)
            {
                allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = drive_data[EN_COM2].deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
                iIndex++;
                tmp = drive_data[EN_COM2].deviceInfo[i].deviceData[m].usVarAddr;
                allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
            }
        }
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = drive_data[EN_COM2].deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
    }

    for (i = 0; i < drive_data[EN_COM2].iDeviceNum; i++)
    {
        int j = 0;
        printf("allDeviceDataFrame[%d].iDeviceID= %d \n", i, allDeviceDataFrame[i].iDeviceID);
        printf("allDeviceDataFrame[%d].iFrameNum= %d \n", i, allDeviceDataFrame[i].iFrameNum);

        for (j = 0; j < allDeviceDataFrame[i].iFrameNum; j++)
        {
            printf("allDeviceDataFrame[%d].dataFrame[%d].usAddrStart = %d \n", i, j, allDeviceDataFrame[i].dataFrame[j].usAddrStart);
            printf("allDeviceDataFrame[%d].dataFrame[%d].ucDataNum = %d \n", i, j, allDeviceDataFrame[i].dataFrame[j].ucDataNum);
        }
    }
#endif
}
#endif

void ltrim ( char *s )
{
    char *p;
    p = s;
    while ( *p == ' ' || *p == '\t' ) {*p++;}
    strcpy ( s,p );
}

void rtrim ( char *s )
{
    int i;
    i = strlen ( s )-1;
    while ( ( s[i] == ' ' || s[i] == '\t' ) && i >= 0 ) {i--;};
    s[i+1] = '\0';
}

//去掉字符串前后空白字符
void trim ( char *s )
{
    ltrim ( s );
    rtrim ( s );
}
