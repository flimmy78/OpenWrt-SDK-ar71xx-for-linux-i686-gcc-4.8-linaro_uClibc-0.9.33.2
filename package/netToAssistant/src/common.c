#include "common.h"

DeviceDataFrame* allDeviceDataFrame;
DeviceConfigInfo* allDeviceConfigInfo;
DeviceAddrInfo allDeviceInfo;
SerialConfig* SerialInfo;        //串口信息
tagMqttInfo* mqttInfo;         //mqtt 信息
unsigned int uVarNum ;        //地址个数
tagJson*  jsonInfo;

int ReadMqttConfig(const char* path)
{
    mqttInfo = (tagMqttInfo*)malloc(sizeof(tagMqttInfo));
    memset(mqttInfo, 0, sizeof(tagMqttInfo));

    jsonInfo = (tagJson*)malloc(sizeof(tagJson));
    memset(jsonInfo, 0, sizeof(tagJson));

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
        if (!bFlag)
        {
            if (strstr(strLine, "broker")) //找到mqtt配置段
            {
                bFlag = true;
            }
            continue;
        }

        if (strstr(strLine, "ip"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strAddress, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "clientID"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strClientID, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "port"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                mqttInfo->iPort = atoi(result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "user"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strUseName, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "pwd"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strPwd, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "topic"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strTopic, result);
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
                jsonInfo->iType = atoi(result);
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
                result = strtok(NULL, "'");
                trim (result);
                strcpy(jsonInfo->strAddrKey, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "version"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strVer, result);
                i++;
                break;
            }
        }

        memset(strLine, 0, MAX_LINE_SIZE);

        if (9 == i) //读完Mqtt的所有配置就要结束
        {
            break;
        }
    }

    fclose(fp);
    return 0;
}

//只读配置表，不分配内存
int ReadMqttConfigOnly(const char* path)
{
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
        if (!bFlag)
        {
            if (strstr(strLine, "broker")) //找到mqtt配置段
            {
                bFlag = true;
            }
            continue;
        }

        if (strstr(strLine, "ip"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strAddress, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "clientID"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strClientID, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "port"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                mqttInfo->iPort = atoi(result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "user"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strUseName, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "pwd"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strPwd, result);
                i++;
                break;
            }
        }
        else if (strstr(strLine, "topic"))
        {
            char *result = NULL;
            result = strtok(strLine, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                trim (result);
                strcpy(mqttInfo->strTopic, result);
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
                jsonInfo->iType = atoi(result);
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
                result = strtok(NULL, "'");
                trim (result);
                strcpy(jsonInfo->strAddrKey, result);
                i++;
                break;
            }
        }

        memset(strLine, 0, MAX_LINE_SIZE);

        if (8 == i) //读完Mqtt的所有配置就要结束
        {
            break;
        }
    }

    fclose(fp);
    return 0;
}

int WriteNewClientID(const char* path, const char* pathTemp, const char* newId)
{
        printf(path);
        printf("\n");
        printf(pathTemp);
        printf("\n");
        printf(newId);
        printf("\n");

    FILE* fp;
    FILE* fpTemp;
    int i, j;
    char strLine[MAX_LINE_SIZE];

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    if((access(pathTemp,F_OK))==0) //文件存在先清空
    {
        remove(pathTemp);
    }
    system("sync");

    if((fpTemp=fopen(pathTemp,"w"))==NULL) //没有就创建
    {
        printf("file open  %s error\n", pathTemp);
        exit(1);
    }

    memset(strLine, 0, MAX_LINE_SIZE);
    i = 0;
    fseek(fp, 0, SEEK_SET);
    while(fgets(strLine, MAX_LINE_SIZE, fp))
    {
        if (!strstr(strLine, "clientID"))
        {
                        printf(strLine);
                        printf("\n");
            //直接写入到临时文件
            fwrite(strLine, sizeof(char), strlen(strLine), fpTemp);//将数据写入文件中
        }
        else
        {
            //修改客户ID再写入到临时文件中
            char strLineTemp[MAX_LINE_SIZE];
            memcpy(strLineTemp, strLine, MAX_LINE_SIZE);
                        printf("before modify ID = %s \n", strLine);
            char *result = NULL;
            result = strtok(strLineTemp, "'");
            while(NULL != result)
            {
                result = strtok(NULL, "'");
                                trim (result);
                                printf("strLine = %s, result = %s, newId = %s", strLine, result, newId);
                replace(strLine, result, newId);
                                printf("after modify ID = %s \n", strLine);
                fwrite(strLine, sizeof(char), strlen(strLine), fpTemp);//将数据写入文件中
                break;
            }
        }
        memset(strLine, 0, MAX_LINE_SIZE);
    }

    fclose(fp);
    fclose(fpTemp);
    remove(path);
    rename(pathTemp,path);
    system("sync");
    return 0;
}

/*将str1字符串中第一次出现的str2字符串替换成str3*/
void replaceFirst(char *str1,char *str2,char *str3)
{
    char str4[strlen(str1)+1];
    char *p;
    strcpy(str4,str1);
    if((p=strstr(str1,str2))!=NULL)/*p指向str2在str1中第一次出现的位置*/
    {
        while(str1!=p&&str1!=NULL)/*将str1指针移动到p的位置*/
        {
            str1++;
        }
        str1[0]='\0';/*将str1指针指向的值变成/0,以此来截断str1,舍弃str2及以后的内容，只保留str2以前的内容*/
        strcat(str1,str3);/*在str1后拼接上str3,组成新str1*/
        strcat(str1,strstr(str4,str2)+strlen(str2));/*strstr(str4,str2)是指向str2及以后的内容(包括str2),strstr(str4,str2)+strlen(str2)就是将指针向前移动strlen(str2)位，跳过str2*/
    }
}
/*将str1出现的所有的str2都替换为str3*/
void replace(char *str1,char *str2,char *str3)
{
    while(strstr(str1,str2)!=NULL)
    {
        replaceFirst(str1,str2,str3);
        break;
    }
}

int ReadXmlFile(const char* path)
{
    FILE* fp = NULL;
    char* sEleValue = NULL;

    sEleValue = (char*)malloc(20);
    memset(sEleValue, 0, 10);

    mqttInfo = (tagMqttInfo*)malloc(sizeof(tagMqttInfo));
    memset(mqttInfo, 0, sizeof(tagMqttInfo));

    jsonInfo = (tagJson*)malloc(sizeof(tagJson));
    memset(jsonInfo, 0, sizeof(tagJson));

    fp = fopen(path, "r");
    if(NULL == fp)
    {
        printf("Get File %s Failed!\n", path);
        return -1;
    }

    /* <mqtt_address> 服务器地址 */
    GetXmlElement(fp, "mqtt_address", sEleValue);
    strcpy(mqttInfo->strAddress, sEleValue);

    /* <mqtt_clientID> 客户端ID */
    GetXmlElement(fp, "mqtt_clientID", sEleValue);
    strcpy(mqttInfo->strClientID, sEleValue);

    /* <mqtt_bind_address> 主板IP */
    GetXmlElement(fp, "mqtt_bind_address", sEleValue);
    strcpy(mqttInfo->strBindAddtess, sEleValue);

    /* <mqtt_port> 端口号 */
    GetXmlElement(fp, "mqtt_port", sEleValue);
    sscanf(sEleValue, "%d", &mqttInfo->iPort);

    /* <mqtt_iqos> qos模式*/
    GetXmlElement(fp, "mqtt_iqos", sEleValue);
    sscanf(sEleValue, "%d", &mqttInfo->iQos);

    /* <mqtt_topic> 标题 */
    GetXmlElement(fp, "mqtt_topic", sEleValue);
    strcpy(mqttInfo->strTopic, sEleValue);

    /* <mqtt_keepalive> keeplive */
    GetXmlElement(fp, "mqtt_keepalive", sEleValue);
    sscanf(sEleValue, "%d", &mqttInfo->iKeepalive);

    /* <mqtt_timeout> 超时时间 */
    GetXmlElement(fp, "mqtt_timeout", sEleValue);
    sscanf(sEleValue, "%d", &mqttInfo->ulTimeout);

    /* <mqtt_usename> 用户名 */
    GetXmlElement(fp, "mqtt_usename", sEleValue);
    strcpy(mqttInfo->strUseName, sEleValue);

    /* <mqtt_pwd> 密码 */
    GetXmlElement(fp, "mqtt_pwd", sEleValue);
    strcpy(mqttInfo->strPwd, sEleValue);

    /* <json_type> 采集器类型 */
    GetXmlElement(fp, "json_type", sEleValue);
    sscanf(sEleValue, "%d", &jsonInfo->iType);

    /* <json_addr_key> 地址类型标记 */
    GetXmlElement(fp, "json_addr_key", sEleValue);
    strcpy(jsonInfo->strAddrKey, sEleValue);

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

int ReadCsvFile(const char* path)
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

    uVarNum = GetCsvTotalLine(fp) ;
    if(uVarNum <= 1)
    {
        printf("File %s is null!\n", path);
        return -1;
    }
    uVarNum = uVarNum - 1;
    allDeviceConfigInfo = (DeviceConfigInfo*)malloc(uVarNum * sizeof(DeviceConfigInfo));
    memset(allDeviceConfigInfo, 0, uVarNum * sizeof(DeviceConfigInfo));
    memset(strLine, 0, MAX_LINE_SIZE);
    fgets(strLine, MAX_LINE_SIZE, fp);              //读取第一行：寄存器地址/参数说明/读写类型
    for(i = 0; i < uVarNum; i++)
    {
        flag = 0;
        memset(strLine, 0, MAX_LINE_SIZE);
        if(fgets(strLine, MAX_LINE_SIZE, fp));
        {
            char *result = NULL;
            int iIndex = 0;
            result = strtok(strLine, ",");
            while(NULL != result)
            {
                iIndex++;
                result = strtok(NULL, ",");
                if (1 == iIndex)
                {
                    allDeviceConfigInfo[i].iDeviceID = atoi(result);
                }
                else if (2 == iIndex)
                {
                    allDeviceConfigInfo[i].usVarAddr = atoi(result);
                    //                    printf("[%d].iDeviceID = %d, [%d].usVarAddr = %d \n",
                    //                           i,allDeviceConfigInfo[i].iDeviceID,
                    //                           i,allDeviceConfigInfo[i].usVarAddr);
                    break;
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

/* 获取csv文件总行数 */
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

//分辨出有几台设备 将每台设备的地址依次从小到大排列好
int IncreaseSort(DeviceConfigInfo* deviceInfo, unsigned num)
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

    allDeviceInfo.iDeviceNum = iDeviceIDNum;

    printf("allDeviceInfo.iDeviceNum = %d \n", allDeviceInfo.iDeviceNum);

    //对找出的设备ID号进行排序,从小到大排序
    int j, k,tmp;
    for (i = 0; i < allDeviceInfo.iDeviceNum - 1; i++)
    {
        k = i;
        for (j = i + 1; j < allDeviceInfo.iDeviceNum; j++)
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

    //找出每台的机器的读取地址数量
    deviceNumInfo = (SignalDeviceInfo*)malloc(sizeof(SignalDeviceInfo) * allDeviceInfo.iDeviceNum ) ;
    memset(deviceNumInfo, 0, sizeof(SignalDeviceInfo) * allDeviceInfo.iDeviceNum);

    printf("iDeviceID[0] = %d \n", iDeviceID[0]);
    for(i = 0; i < num; i++)
    {
        int j = 0;
        for (j = 0;  j <  allDeviceInfo.iDeviceNum; j++)
        {
            if (iDeviceID[j] == deviceInfo[i].iDeviceID)
            {
                deviceNumInfo[j].iDeviceID = deviceInfo[i].iDeviceID;
                deviceNumInfo[j].iNum++;
                break;
            }
        }
    }
    //将每台机器的读取地址分别保存，并按地址从小到大排列
    allDeviceInfo.deviceInfo = (SignalDevice*)malloc(sizeof(SignalDevice) * allDeviceInfo.iDeviceNum);
    for (i = 0; i < allDeviceInfo.iDeviceNum; i++ )
    {
        allDeviceInfo.deviceInfo[i].deviceData= (DeviceDataInfo *)malloc(sizeof(DeviceDataInfo) * deviceNumInfo[i].iNum);
        memset(allDeviceInfo.deviceInfo[i].deviceData, 0, sizeof(DeviceDataInfo) * deviceNumInfo[i].iNum);
        allDeviceInfo.deviceInfo[i].iDeviceID = deviceNumInfo[i].iDeviceID;
        allDeviceInfo.deviceInfo[i].iDeviceNum = deviceNumInfo[i].iNum;
    }

    int* iIndex;
    iIndex = (int*)malloc(sizeof(int) * allDeviceInfo.iDeviceNum);
    memset(iIndex, 0, sizeof(int) * allDeviceInfo.iDeviceNum);

    for(i = 0; i < num; i++)
    {
        int j = 0;
        for (j = 0;  j <  allDeviceInfo.iDeviceNum; j++)
        {
            if (deviceNumInfo[j].iDeviceID == deviceInfo[i].iDeviceID)
            {
                allDeviceInfo.deviceInfo[j].deviceData[iIndex[j]].usVarAddr = deviceInfo[i].usVarAddr;
                allDeviceInfo.deviceInfo[j].deviceData[iIndex[j]].sVarValue = deviceInfo[i].sVarValue;
                iIndex[j]++;
                break;
            }
        }
    }
    //对每台地址从小到大排序
    DeviceDataInfo temp;
    int m = 0;
    for (m= 0;  m <  allDeviceInfo.iDeviceNum; m++)
    {
        for (i = 0; i < deviceNumInfo[m].iNum - 1; i++)
        {
            k = i;
            for (j = i + 1; j < deviceNumInfo[m].iNum; j++)
            {
                if (allDeviceInfo.deviceInfo[m].deviceData[k].usVarAddr > allDeviceInfo.deviceInfo[m].deviceData[j].usVarAddr)
                {
                    k = j;
                }
            }

            if (i !=k)
            {
                temp = allDeviceInfo.deviceInfo[m].deviceData[i];
                allDeviceInfo.deviceInfo[m].deviceData[i] = allDeviceInfo.deviceInfo[m].deviceData[k];
                allDeviceInfo.deviceInfo[m].deviceData[k] = temp;
            }
        }
    }

    for (m= 0;  m <  allDeviceInfo.iDeviceNum; m++)
    {
        printf("allDeviceInfo.iDeviceNum = %d , deviceNumInfo[%d].iNum = %d\n",
               allDeviceInfo.iDeviceNum, m, deviceNumInfo[m].iNum);

        for (i = 0; i < allDeviceInfo.deviceInfo[m].iDeviceNum; i++)
        {
            printf("allDeviceInfo.deviceInfo[%d].iDeviceID = %d \n", m, allDeviceInfo.deviceInfo[m].iDeviceID);
            printf("allDeviceInfo.deviceInfo[%d].deviceData[%d].usVarAddr = %d \n",
                   m, i ,allDeviceInfo.deviceInfo[m].deviceData[i].usVarAddr);
        }
    }

    return 0;
}

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

void CreateDataFrame()
{
    unsigned int i;
    allDeviceDataFrame = (DeviceDataFrame*)malloc(sizeof(DeviceDataFrame) * allDeviceInfo.iDeviceNum);
    for (i = 0; i < allDeviceInfo.iDeviceNum; i++)
    {
        int iNum = GetFrameNum(allDeviceInfo.deviceInfo[i]);
        allDeviceDataFrame[i].dataFrame = (SignalDataFrame*)malloc(sizeof(SignalDataFrame) * iNum);
        memset(allDeviceDataFrame[i].dataFrame, 0 , sizeof(SignalDataFrame) * iNum);
        allDeviceDataFrame[i].iDeviceID = allDeviceInfo.deviceInfo[i].iDeviceID;
        allDeviceDataFrame[i].iFrameNum = iNum;

        int m, tmp;
        int iIndex = 0;

        tmp = allDeviceInfo.deviceInfo[i].deviceData[0].usVarAddr;
        allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = 1;

        for(m = 0; m < allDeviceInfo.deviceInfo[i].iDeviceNum; m++)
        {
            if((allDeviceInfo.deviceInfo[i].deviceData[m].usVarAddr - tmp) >= MAX_READ_LEN)
            {
                allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = allDeviceInfo.deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
                iIndex++;
                tmp = allDeviceInfo.deviceInfo[i].deviceData[m].usVarAddr;
                allDeviceDataFrame[i].dataFrame[iIndex].usAddrStart = tmp;
            }
        }
        allDeviceDataFrame[i].dataFrame[iIndex].ucDataNum = allDeviceInfo.deviceInfo[i].deviceData[m - 1].usVarAddr - tmp + 1;
    }

    for (i = 0; i < allDeviceInfo.iDeviceNum; i++)
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
}

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
