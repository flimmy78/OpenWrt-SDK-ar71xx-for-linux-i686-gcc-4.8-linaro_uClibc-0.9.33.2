#include "sqlite3.h"
#include "dataroutedb.h"

//连接到数据库，格式：index+device_name+device_address+sendbuffer
int CreateConnectionToDb(char* path,int seq)
{
    int rc=-1;
    char cFileName[LASTMAX];
    char cCpCmd[CMDMAX];
//    char cTimeBuffer[80];
//    time_t nowtime;
//    struct tm *timeinfo;
//    time( &nowtime );
//    timeinfo = localtime( &nowtime );
//    char  format[] = "%Y_%m_%d_%H_%M_%S";
//    strftime(cTimeBuffer, sizeof(cTimeBuffer), format, timeinfo);
//    printf("cTimeBuffer=%s",cTimeBuffer);
    sprintf(cFileName,"%s/dataroute_%d.db",path,seq);
    if(access(cFileName,F_OK) != 0)
    {
        sprintf(cCpCmd,"cp -a /usr/bin/dataroute/dataroute.db %s",cFileName);
        printf("cCpCmd = %s\n",cCpCmd);
        system(cCpCmd);
        system("sync");
    }
    rc = sqlite3_open(cFileName,&pDB);
    printf("rc = %d",rc);
    if(rc ==SQLITE_OK)
    {
        printf("\nOpen %s sucessful!!!!\n",cFileName);
        return 1;
    }
    else
    {
         printf("\nOpen %s failed!!!!\n",cFileName);
        return 0;
    }

}

int CreateConnectionToHisDb(char* path,int seq)
{
    int rc=-1;
    char cFileName[LASTMAX];
    char cCpCmd[CMDMAX];
//    char cTimeBuffer[80];
//    time_t nowtime;
//    struct tm *timeinfo;
//    time( &nowtime );
//    timeinfo = localtime( &nowtime );
//    char  format[] = "%Y_%m_%d_%H_%M_%S";
//    strftime(cTimeBuffer, sizeof(cTimeBuffer), format, timeinfo);
//    printf("cTimeBuffer=%s",cTimeBuffer);
    sprintf(cFileName,"%s/dataroute_%d.db",path,seq);
    if(access(cFileName,F_OK) != 0)
    {
        sprintf(cCpCmd,"cp -a /usr/bin/dataroute/dataroute.db %s",cFileName);
        printf("cCpCmd = %s\n",cCpCmd);
        system(cCpCmd);
        system("sync");
    }

    //printf("%s*******\n",cFileName);
    rc = sqlite3_open(cFileName,&pDBHistory);
    if(rc ==SQLITE_OK)
    {
        printf("Open History db cFileName=%s sucessful!!!!\n",cFileName);
        return 1;
    }
    else
    {
         printf("\nOpen %s failed!!!!\n",cFileName);
        return 0;
    }

}

int InsertToDatarouteDb(int seq ,int iDeviceAddr, char* cDeviceName, char* cSendBuffer)
{
    printf("iIndex= %d,InsertToDatarouteDb!!!!!!\n",seq);
    char* errMsg=0;
    int rc;
    //char cTimeBuffer[80];
    unsigned char ucInsertBuf[INSERTMAX*120] = {0};
    time_t nowtime;
    struct tm *timeinfo;
    int seconds=time( &nowtime );
    //timeinfo = localtime( &nowtime );
    //char  format[] = "%Y-%m-%d %H:%M:%S";
    //strftime(cTimeBuffer, sizeof(cTimeBuffer), format, timeinfo);
    //printf("Time is %s\n",cTimeBuffer);
    usleep(100*1000);
    sprintf(ucInsertBuf,"insert into dataroute values('%d','%d','%d','%s','%s')",seq,seconds,iDeviceAddr,cDeviceName,cSendBuffer);
    printf("%s\n",ucInsertBuf);
    rc = sqlite3_exec(pDB,ucInsertBuf,0,0,&errMsg);
     if (rc == SQLITE_OK)
      {
         printf("INSERT DATA SUCESS,rc=%d\n",rc);
         return 1;
       }
     else
       {
         printf("INSERT DATA failed,rc=%d\n",rc);
         sqlite3_free(errMsg);
         return 0;
       }
}

int UpdateToDatarouteDb(int seq ,int iDeviceAddr, char* cDeviceName, char* cSendBuffer)
{
    printf("updateiIndex= %d,UpdateToDatarouteDb!!!!!!\n",seq);
    char* errMsg=0;
    int rc;
    char cTimeBuffer[80];
    unsigned char ucInsertBuf[INSERTMAX*120] = {0};
    time_t nowtime;
    struct tm *timeinfo;
    int seconds=time( &nowtime );
    //timeinfo = localtime( &nowtime );
    //char  format[] = "%Y-%m-%d %H:%M:%S";
    //strftime(cTimeBuffer, sizeof(cTimeBuffer), format, timeinfo);
   // printf("Time is %s\n",cTimeBuffer);
    sprintf(ucInsertBuf,"update dataroute set dev_time='%d',dev_addr='%d',dev_name='%s',send_buffer='%s' where id = '%d'",seconds,iDeviceAddr,cDeviceName,cSendBuffer,seq);
    printf("%s\n",ucInsertBuf);
    rc = sqlite3_exec(pDB,ucInsertBuf,0,0,&errMsg);
     if (rc == SQLITE_OK)
      {
         printf("UPDATE DATA SUCESS,rc=%d\n",rc);
         return 1;
       }
     else
       {
         printf("UPDATE DATA failed,rc=%d\n",rc);
            sqlite3_free(errMsg);
         return 0;
       }
}

int DeleteDataroute(int seq)
{
     printf("222iIndex= %d,DeleteDataroute!!!!!!\n",seq);
    char* errMsg=0;
    int rc;
    unsigned char ucDeleteBuf[INSERTMAX] = {0};
    sprintf(ucDeleteBuf,"delete from dataroute where id= %d",seq);
    printf("%s\n",ucDeleteBuf);
    rc = sqlite3_exec(pDBHistory,ucDeleteBuf,0,0,&errMsg);
    if (rc == SQLITE_OK)
     {
        printf("DELETE %d DATA SUCCESS,rc=%d\n",seq,rc);

        return 1;
      }
    else
      {
        printf("DELETE DATA failed,rc=%d\n",rc);
           sqlite3_free(errMsg);
        return 0;
      }
}

int VacuumDataroute()
{
     printf("333VacuumDataroute!!!!!!\n");
    char* errMsg=0;
    int rc;
    unsigned char ucVacuumBuf[INSERTMAX] = {0};
    //sprintf(ucVacuumBuf,"delete from dataroute where id= %d",783);
    sprintf(ucVacuumBuf,"%s","VACUUM");
    printf("%s\n",ucVacuumBuf);
    printf("******\n");
      rc = sqlite3_exec(pDBHistory,ucVacuumBuf,0,0,&errMsg);
printf("******\n");
    if (rc == SQLITE_OK)
     {
        printf("Vacuum DATA SUCCESS,rc=%d\n",rc);
        return 1;
      }
    else
      {
        printf("Vacuum DATA failed,rc=%d\n",rc);
           sqlite3_free(errMsg);
        return 0;
      }
}

int SelectHistoryData(int seq, char* sendbuf)
{
    char* errMsg=0;
    int rc;
    char** selectResult;
    int iRows = 0;
    int iColumns = 0;
    unsigned char ucSelectBuf[INSERTMAX] = {0};
    unsigned char ucSelectBufTmp[INSERTMAX*100] = {0};
    sprintf(ucSelectBuf,"select id,send_buffer,dev_time from dataroute where id= %d",seq);
     rc = sqlite3_get_table(pDBHistory, ucSelectBuf, &selectResult, &iRows, &iColumns, &errMsg);
    printf("rc= %d\n",rc);
    if (rc == SQLITE_OK)
    {
       printf("SELECT %d DATA SUCCESS,iRows=%d,iColumns=%d\n",seq,iRows,iColumns);
        int i = 0;
        for(i=1; i < iRows+1; i++)
        {
           // sendbuf = selectResult[i*iColumns+1];
            printf("%s\n",selectResult[i*iColumns+2]);
            char *p;
            int len;
            p = strstr(selectResult[i*iColumns+1],"}");
            len =strlen(selectResult[i*iColumns+1])  - strlen(p); //
            strncat(ucSelectBufTmp, selectResult[i*iColumns+1], len) ;
             //printf("44444%s\n",ucSelectBufTmp);
            sprintf(sendbuf,"%s,\"ts\":%s}", ucSelectBufTmp,selectResult[i*iColumns+2]);
            printf("5555%s\n",sendbuf);

        }
        sqlite3_free_table(selectResult);
     return 1;
    }
    else
    {
        printf("SELECT %d DATA failed!!\n",seq);
        sqlite3_free(errMsg);
        return 0;
    }
}

int SelectCurrentData(int SortMethod)
{
    printf("SelectCurrentData()!!!!!!\n");
    char* errMsg=0;
    int rc;
    char** selectResult;
    int iRows = 0;
    int iColumns = 0;
    int iId;
    unsigned char ucLastBuf[INSERTMAX] = {0};
    if( SortMethod == 1)
    {
        sprintf(ucLastBuf,"select id from dataroute order by id limit 0,1");
        printf("%s\n",ucLastBuf);
        rc = sqlite3_get_table(pDB, ucLastBuf, &selectResult, &iRows, &iColumns, &errMsg);
        if (rc == SQLITE_OK)
        {
            printf("1iRows=%d,iColumns=%d\n",iRows,iColumns);
            int i = 0;
            for(i=1; i < iRows+1; i++)
            {
                printf("%s\n",selectResult[i*iColumns]);
                iId =atoi(selectResult[i*iColumns]);
                sqlite3_free_table(selectResult);
                if(iId > 0)
                    return iId;
                else
                    return 0;
            }

        }
        else
        {
            sqlite3_free(errMsg);
            return 0;
        }
    }
    else if(SortMethod == 0)
    {
        sprintf(ucLastBuf,"select id from dataroute order by id desc limit 0,1");
        printf("%s\n",ucLastBuf);
        rc = sqlite3_get_table(pDB, ucLastBuf, &selectResult, &iRows, &iColumns, &errMsg);
        if (rc == SQLITE_OK)
        {
            printf("1iRows=%d,iColumns=%d\n",iRows,iColumns);
            int i = 0;
            for(i=1; i < iRows+1; i++)
            {
                printf("%s\n",selectResult[i*iColumns]);
                iId =atoi(selectResult[i*iColumns]);
                sqlite3_free_table(selectResult);
                if(iId > 0)
                    return iId;
                else
                    return 0;
            }

        }
        else
        {
            sqlite3_free(errMsg);
            return 0;
        }
    }
}

int SelectLastData()
{
    printf("SelectLastData()!!!!!!\n");
    char* errMsg=0;
    int rc;
    char** selectResult;
    int iRows = 0;
    int iColumns = 0;
    int iId;
    unsigned char ucLastBuf[INSERTMAX] = {0};

    sprintf(ucLastBuf,"select id from dataroute order by id limit 0,1");
    printf("%s\n",ucLastBuf);
     rc = sqlite3_get_table(pDBHistory, ucLastBuf, &selectResult, &iRows, &iColumns, &errMsg);
    if (rc == SQLITE_OK)
    {
        printf("1iRows=%d,iColumns=%d\n",iRows,iColumns);
        int i = 0;
        for(i=1; i < iRows+1; i++)
        {
            printf("%s\n",selectResult[i*iColumns]);
            iId =atoi(selectResult[i*iColumns]);
            sqlite3_free_table(selectResult);
            return iId;
        }
        sqlite3_free_table(selectResult);
        return 0;
    }
    else
    {
        sqlite3_free(errMsg);
        return 0;
    }
}

int CloseDB()
{
    sqlite3_close(pDB);
}

int CloseHisDB()
{
    sqlite3_close(pDBHistory);
}
