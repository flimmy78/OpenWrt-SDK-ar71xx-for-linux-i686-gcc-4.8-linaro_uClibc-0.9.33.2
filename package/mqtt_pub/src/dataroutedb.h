#ifndef DATAROUTEDB_H
#define DATAROUTEDB_H
#include "stdio.h"
#include "sqlite3.h"
#include<time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define INSERTMAX 1000
#define LASTMAX 255
#define CMDMAX 512
#define FILEMAXLEN 8
#define MAX_LINE_SIZE 255
//int iRecordCount = 0;
#define PATH "usr/bin/dataroute"
sqlite3* pDB;
sqlite3* pDBHistory;
//sqlite3 *pDB;
int CreateConnectionToDb(char* path,int seq);
int CreateConnectionToHisDb(char* path,int seq);
int InsertToDatarouteDb(int seq ,int iDeviceAddr, char* cDeviceName, char* cSendBuffer);
int UpdateToDatarouteDb(int seq ,int iDeviceAddr, char* cDeviceName, char* cSendBuffer);
int DeleteDataroute(int seq);
int SelectHistoryData(int seq, char* sendbuf);
int SelectLastData();
int CloseDB();
int CloseHisDB();
int VacuumDataroute();
int SelectCurrentData( int SortMethod);

#endif // DATAROUTEDB_H
