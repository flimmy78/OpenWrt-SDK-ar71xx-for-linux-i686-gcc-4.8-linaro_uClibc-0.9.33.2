#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "netCommu.h"

main()
{
    ReadMqttConfig(MQTT_CONFIG) ;

#ifdef MQTT_DEBUG
    printf("mqttInfo->strAddress = %s\n",mqttInfo->strAddress);
    printf("mqttInfo->iPort = %d\n",mqttInfo->iPort);
    printf("mqttInfo->strTopic = %s\n",mqttInfo->strTopic);
    printf("mqttInfo->strUseName = %s\n",mqttInfo->strUseName);
    printf("mqttInfo->strPwd = %s\n",mqttInfo->strPwd);
    printf("mqttInfo->strClientID = %s\n",mqttInfo->strClientID);
    printf("jsonInfo->iType = %d\n",jsonInfo->iType);
    printf("jsonInfo->strAddrKey = %s\n",jsonInfo->strAddrKey);
    printf("mqttInfo->strVer = %s\n",mqttInfo->strVer);
#endif

    initThreadUDP();
    initThreadTCP();
    for(;;){usleep(1000);}
}
