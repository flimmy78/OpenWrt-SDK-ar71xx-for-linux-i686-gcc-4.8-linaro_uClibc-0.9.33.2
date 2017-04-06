#ifndef UPGRADE_H
#define UPGRADE_H

#define NULL 0
#define true 1

int initMqtt();
void destroyMqtt();
int send_package_version(char* SendBuff);
int get_package_version(char* buf);
void reconnect();

#endif // UPGRADE_H
