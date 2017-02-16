/*
Copyright (c) 2009-2013 Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include "mqtt_sub.h"
#include "common.h"
#define MQTT_XML_PATH "/etc/config/broker"
#define SERIAL_XML_PATH "/etc/config/comm"
#define DOWN_THREAD_OPEN
#define SUB_PARSE_STRINGS
#define MQTT_DEBUG

#define SOFTWARE_UPDATE_DIR "/usr/bin/dataroute/"
#define CONFIG_UPDATE_DIR "/etc/config/"
#define UPDATE_MD5_DIR "/usr/bin/dataroute/update.md5"
#define CONFIG_UPDATE_DIR "/etc/config/"
#define FTP_LEN 1024
#define MD5_LEN 32
#define MAX_LEN 255

/* This struct is used to pass data to callbacks.
 * An instance "ud" is created in main() and populated, then passed to
 * mosquitto_new(). */
struct userdata
{
    char **topics;
    int topic_count;
    int topic_qos;
    char **filter_outs;
    int filter_out_count;
    char *username;
    char *password;
    int verbose;
    bool quiet;
    bool no_retain;
    bool eol;
};

typedef struct  ttdataroute_update_info
{
    char dataroute_ftp_address[16];//14.215.130.186
    char dataroute_route[50];//vsftp/gprs/update
    char dataroute_filename[30];//dataroute.tar.gz
    char dataroute_version[20] ;//V1.00
    char dataroute_md5[33];//678a1cd5d78f5910ea57e48d661f98d2
}dataroute_update_info;
dataroute_update_info update_info;
dataroute_update_info config_info;
//{"Control1":"010620010001120A++010620020001E20A++010620030001B3CA"}
//{"Control1":"0106004A0200a97c"}

#ifdef SUB_PARSE_STRINGS
int char_to_number(unsigned char input);
unsigned char * string_to_hex(unsigned char *src,unsigned char *dest);
int parse_down_modbus(char *data ,int length);
int parse_up_all_data(char *data ,int length);
update_run();
int parse_down_update(char *data_temp);
int parse_down_strings(char *data, int length);
#endif

void my_message_callback(struct mosquitto *mosq, void *obj,
                                               const struct mosquitto_message *message);
void my_connect_callback(struct mosquitto *mosq, void *obj, int result);
void my_subscribe_callback(struct mosquitto *mosq, void *obj,
                                                int mid, int qos_count, const int *granted_qos);
void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str);
void print_usage(void);

int main(int argc, char *argv[])
{
    if (!open_ramrt())
    {
        printf("open ramrt error!\n");
        return;
    }
    else
    {
        printf("open ramrt successful !\n");
    }
    /* 读取配置文件 */
    if(ReadMqttConfig(MQTT_XML_PATH) == -1)
        return -1;
    if(ReadSerialConfig(SERIAL_XML_PATH) == -1)
        return -1;
    strcpy(mqttInfo->strAddress, "14.215.130.180");
    mqttInfo->iPort =1883;
    strcpy(mqttInfo->strTopic , "r");
    strcpy(mqttInfo->strUseName ,  "invtGTerm");
    strcpy(mqttInfo->strPwd ,  "in2015v11t11");
#ifdef MQTT_DEBUG
    printf("mqttInfo->strAddress = %s\n",mqttInfo->strAddress);
    printf("mqttInfo->iPort = %d\n",mqttInfo->iPort);
    printf("mqttInfo->strTopic = %s\n",mqttInfo->strTopic);
    printf("mqttInfo->strUseName = %s\n",mqttInfo->strUseName);
    printf("mqttInfo->strPwd = %s\n",mqttInfo->strPwd);
    printf("mqttInfo->strClientID = %s\n",mqttInfo->strClientID);
    printf("jsonInfo->iType = %d\n",jsonInfo->iType);
    printf("jsonInfo->strAddrKey = %s\n",jsonInfo->strAddrKey);
#endif

#ifdef DOWN_THREAD_OPEN
    if ( (cmd_list=CreateList()) == NULL)
    {
        printf("CreateList error!\n");
        return;
    }
    else
    {
        printf("CreateList successful !\n");
    }

    if (!down_thread_init())
    {
        printf("thread_init error!\n");
        return;
    }
    else
    {
        printf("thread_init successful !\n");
    }

#endif
    char *id = NULL;
    char *id_prefix = NULL;
    int i;
    char *host = NULL;
    int port = mqttInfo->iPort;
    char *bind_address = NULL;
    int keepalive = 60;
    bool clean_session = true;
    bool debug = false;
    struct mosquitto *mosq = NULL;
    int rc;
    char hostname[256];
    char err[1024];
    struct userdata ud;
    int len;
    char *will_payload = NULL;
    long will_payloadlen = 0;
    int will_qos = 0;
    bool will_retain = false;
    char *will_topic = NULL;
    bool insecure = false;
    char *cafile = NULL;
    char *capath = NULL;
    char *certfile = NULL;
    char *keyfile = NULL;
    char *tls_version = NULL;
    char *psk = NULL;
    char *psk_identity = NULL;
    char *ciphers = NULL;
    bool use_srv = false;

    memset(&ud, 0, sizeof(struct userdata));
    ud.eol = true;
    host = mqttInfo->strAddress;
    id = mqttInfo->strClientID;
    ud.topic_count++;
    ud.topics = realloc(ud.topics, ud.topic_count*sizeof(char *));
    ud.topics[ud.topic_count-1] = mqttInfo->strTopic;
    ud.username = mqttInfo->strUseName;
    ud.password = mqttInfo->strPwd;
    ud.verbose = 0;

    printf("main::1111 \n");

    if(clean_session == false && (id_prefix || !id))
    {
        if(!ud.quiet) fprintf(stderr, "Error: You must provide a client id if you are using the -c option.\n");
        return 1;
    }

    if(ud.topic_count == 0)
    {
        fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
        print_usage();
        return 1;
    }

    if(will_payload && !will_topic)
    {
        fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
        print_usage();
        return 1;
    }

    if(will_retain && !will_topic)
    {
        fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
        print_usage();
        return 1;
    }

    if(ud.password && !ud.username)
    {
        if(!ud.quiet) fprintf(stderr, "Warning: Not using password since username not set.\n");
    }

    if((certfile && !keyfile) || (keyfile && !certfile))
    {
        fprintf(stderr, "Error: Both certfile and keyfile must be provided if one of them is.\n");
        print_usage();
        return 1;
    }

    if((cafile || capath) && psk)
    {
        if(!ud.quiet) fprintf(stderr, "Error: Only one of --psk or --cafile/--capath may be used at once.\n");
        return 1;
    }

    if(psk && !psk_identity)
    {
        if(!ud.quiet) fprintf(stderr, "Error: --psk-identity required if --psk used.\n");
        return 1;
    }

    printf("main::2222 \n");

    mosquitto_lib_init();

    if(id_prefix)
    {
        id = malloc(strlen(id_prefix)+10);
        if(!id)
        {
            if(!ud.quiet) fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(id, strlen(id_prefix)+10, "%s%d", id_prefix, getpid());
    }
    else if(!id)
    {
        hostname[0] = '\0';
        gethostname(hostname, 256);
        hostname[255] = '\0';
        len = strlen("mosqsub/-") + 6 + strlen(hostname);
        id = malloc(len);
        if(!id)
        {
            if(!ud.quiet) fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(id, len, "mosqsub/%d-%s", getpid(), hostname);
        if(strlen(id) > MOSQ_MQTT_ID_MAX_LENGTH)
        {
            /* Enforce maximum client id length of 23 characters */
            id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
        }
    }

    printf("main::3333 \n");

    mosq = mosquitto_new(id, clean_session, &ud);
    if(!mosq)
    {
        switch(errno)
        {
        case ENOMEM:
            if(!ud.quiet) fprintf(stderr, "Error: Out of memory.\n");
            break;
        case EINVAL:
            if(!ud.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
            break;
        }
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("main::4444 \n");

    if(debug)
    {
        mosquitto_log_callback_set(mosq, my_log_callback);
    }
    if(will_topic && mosquitto_will_set(mosq, will_topic, will_payloadlen, will_payload, will_qos, will_retain))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting will.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    if(ud.username && mosquitto_username_pw_set(mosq, ud.username, ud.password))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting username and password.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    if((cafile || capath) && mosquitto_tls_set(mosq, cafile, capath, certfile, keyfile, NULL))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting TLS options.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    if(insecure && mosquitto_tls_insecure_set(mosq, true))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting TLS insecure option.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    if(psk && mosquitto_tls_psk_set(mosq, psk, psk_identity, NULL))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting TLS-PSK options.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    if(tls_version && mosquitto_tls_opts_set(mosq, 1, tls_version, ciphers))
    {
        if(!ud.quiet) fprintf(stderr, "Error: Problem setting TLS options.\n");
        mosquitto_lib_cleanup();
        return 1;
    }
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    if(debug)
    {
        mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    }

    printf("main::5555 \n");

    if(use_srv)
    {
        rc = mosquitto_connect_srv(mosq, host, keepalive, bind_address);
    }
    else
    {
        rc = mosquitto_connect_bind(mosq, host, port, keepalive, bind_address);
    }
    if(rc)
    {
        if(!ud.quiet)
        {
            if(rc == MOSQ_ERR_ERRNO)
            {
#ifndef WIN32
                strerror_r(errno, err, 1024);
#else
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errno, 0, (LPTSTR)&err, 1024, NULL);
#endif
                fprintf(stderr, "Error: %s\n", err);
            }
            else
            {
                fprintf(stderr, "Unable to connect (%d).\n", rc);
            }
        }
        mosquitto_lib_cleanup();
        printf("9999999 rc=%d\n",rc);
        while(1)
        {

            int rc1 = mosquitto_connect_bind(mosq, host, port, keepalive, bind_address);
            if(rc1 == 0)
            break;
            sleep(2);
            printf("101010101 rc1=%d\n",rc1);
        }
    }

    printf("main::6666 \n");
    unsigned int reconnect_delay = 1;
    mosquitto_reconnect_delay_set( mosq, reconnect_delay, 10,false);
    printf("main::7777 \n");
    rc = mosquitto_loop_forever(mosq, -1, 1);
    printf("main::8888 \n");
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    printf("main::9999 \n");
    if(rc)
    {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    }

    /*******************************************/
#ifdef DOWN_THREAD_OPEN
    thread_run = 0;
    while (cmd_list != NULL)
        pop_front(cmd_list);
#endif
    return rc;
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    printf("%s\n", str);
}

void print_usage(void)
{
    printf("\nSee http://mosquitto.org/ for more information.\n\n");
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    struct userdata *ud;
    assert(obj);
    ud = (struct userdata *)obj;
    if(!ud->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++)
    {
        if(!ud->quiet) printf(", %d", granted_qos[i]);
    }
    if(!ud->quiet) printf("\n");
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
    int i;
    struct userdata *ud;
    assert(obj);
    ud = (struct userdata *)obj;

    if(!result)
    {
        for(i=0; i<ud->topic_count; i++)
        {
            mosquitto_subscribe(mosq, NULL, ud->topics[i], ud->topic_qos);
        }
    }
    else
    {
        if(result && !ud->quiet)
        {
            fprintf(stderr, "%s\n", mosquitto_connack_string(result));
        }
    }
}

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    struct userdata *ud;
    int i;
    bool res;

    assert(obj);
    ud = (struct userdata *)obj;
    if(message->payloadlen)
    {
        parse_down_strings(message->payload,message->payloadlen);
        printf("****%s\n",message->payload);
    }
}

#ifdef SUB_PARSE_STRINGS
int char_to_number(unsigned char input)
{
    int number = 0;
    if ( '0' <= input && input <= '9')
    {
        number = input - '0';
    }
    else if ('a' <= input && input <= 'f')
    {
        number = (input - 'a') + 10;
    }
    else if ('A' <= input && input <= 'F')
    {
        number = (input - 'A') + 10;
    }
    else
    {
        number = -1;
    }
    return number;
}

unsigned char * string_to_hex(unsigned char *src,unsigned char *dest)
{
    int i;
    int length = strlen(src);
    for (i = 0;i <length;i = i+2)
    {
        src[i] = char_to_number(src[i]);
        src[i+1] = char_to_number(src[i+1]);
        dest[i/2] = src[i]*16+src[i+1];
    }
    return dest;
}

//如果接受的是远程下发指令，调用该函数
int parse_down_modbus(char *data ,int length)
{
    printf( "parse_down_modbus::data = %s, length = %d\n", data, length);
    if (data[0] != '{' || data[length-1] != '}')
    {
        printf("Down command is error!\n");
        return 0;
    }
    else
    {
        tagTaskItem task_item;
        task_item.length = length;
        memset(task_item.data, 0 , DOWD_CMD_LENGTH_MAX);
        memcpy(task_item.data,data,length);
        pthread_mutex_lock(&down_work_mutex);
        push_back(cmd_list,task_item);
        pthread_mutex_unlock(&down_work_mutex);
    }
    return 1;
}

//如果接受的是远程上传全部数据指令，调用该函数
int parse_up_all_data(char *data ,int length)
{
    printf( "parse_up_all_data::data = %s, length = %d\n", data, length);
    if (data[0] != '{' || data[length-1] != '}')
    {
        printf("up all data command is error!\n");
        return 0;
    }
    else
    {
        tagTaskItem task_item;
        task_item.length = length;
        memset(task_item.data, 0 , DOWD_CMD_LENGTH_MAX);
        memcpy(task_item.data,data,length);
        pthread_mutex_lock(&down_work_mutex);
        push_back(cmd_list,task_item);
        pthread_mutex_unlock(&down_work_mutex);
    }
    return 1;
}

update_run()
{
    printf("update_run()!!!!!\n");
    char ftpcmd[FTP_LEN] = {0};
    //char *get_cmd = "qftp get ";
    char *broker_IP = update_info.dataroute_ftp_address;
    printf("broker_IP = %s\n", broker_IP);
    char *broker_username = "invtgprs";
    printf("broker_username = %s\n", broker_username);
    char *broker_pwd = "invt1103";
    printf("broker_pwd = %s\n", broker_pwd);
    char *broker_file_dir = update_info.dataroute_route;
    printf("broker_file_dir = %s\n", broker_file_dir);
    char *broker_file_name = update_info.dataroute_filename;
    printf("broker_file_name = %s\n", broker_file_name);
    char *broker_md5 = update_info.dataroute_md5;
    printf("broker_md5 = %s\n", broker_md5);//68fef19a09282f7994cc9fb9599a6743
    sprintf(ftpcmd, "qftp get %s -l %s -p %s -r %s %s\n", broker_IP, broker_username, broker_pwd,  broker_file_dir, broker_file_name );
    printf("%s\n",ftpcmd);
    int ret = system(ftpcmd);//获取升级压缩包文件
    if( ret )
    {
        printf("obtain software failed!");
        int i ;
        for (i= 0;i < 3;i++)
        {
            int ret = system(ftpcmd);//获取升级压缩包文件
            if(!ret)
                break;
        }
    }
    else
    {
        char md5cmd[FTP_LEN] = { 0 };
        char LineValue[MAX_LEN] = { 0 };
        char md5value[MAX_LEN] = { 0 };
        sprintf(md5cmd, "md5sum %s >%s \n", broker_file_name, UPDATE_MD5_DIR);
        int ret = system(md5cmd);
        printf("ret =%d,  %s\n",ret, md5cmd);

        FILE* fp = NULL;
        fp = fopen(UPDATE_MD5_DIR, "r");
        if(NULL == fp)
        {
            printf("Get File %s Failed!\n", UPDATE_MD5_DIR);
            return -1;
        }
        while( fgets(LineValue, MAX_LEN, fp)!=0)
        {
            if (strstr(LineValue,"dataroute.tar.gz")!=0)
                strncpy(md5value,LineValue,MD5_LEN);
        }
       // fgets(md5value, MD5_LEN, fp);
        printf("md5value checking = %s\n",md5value);
        printf("broker_md5 checking = %s\n",broker_md5);
        if(strncmp(md5value,broker_md5,MD5_LEN)==0 )
        {
            char tarcmd[FTP_LEN] = {0};
            sprintf(tarcmd, "tar -zvxf %s -C %s", broker_file_name, SOFTWARE_UPDATE_DIR);
            printf(" %s\n",tarcmd);
            int ret = system(tarcmd);
            if( ret )
            {
                printf("tar -zvxf dataroute.tar.gz failed!\n");
            }
            else
            {
                system("rm -rf dataroute.tar.gz");
            }

            char chmodcmd[FTP_LEN] = {0};
            sprintf(chmodcmd, "chmod -R 755 %sdataroute_update.sh", SOFTWARE_UPDATE_DIR);
            printf("%s",chmodcmd);
            system(chmodcmd);

            char updatecmd[FTP_LEN] = {0};
            sprintf(updatecmd, "%sdataroute_update.sh", SOFTWARE_UPDATE_DIR);
            printf("%s",updatecmd);
            int ret1 = system(updatecmd);
            if(ret1== 0)
            {
               printf("obtain update file successful!!");
               system("reboot");
            }

            else
            {
                printf("obtain update file failed!!");
            }

        }

    }
    printf("gointo update mode!");
}

update_config()
{
    //printf("update_config()!!!!!\n");
    char ftpcmd[FTP_LEN] = {0};
    //char *get_cmd = "qftp get ";
    char *broker_IP = config_info.dataroute_ftp_address;
   // printf("broker_IP = %s\n", broker_IP);
    char *broker_username = "invtgprs";
    //printf("broker_username = %s\n", broker_username);
    char *broker_pwd = "invt1103";
   // printf("broker_pwd = %s\n", broker_pwd);
    char *broker_file_dir = config_info.dataroute_route;
    //printf("broker_file_dir = %s\n", broker_file_dir);
    char *broker_file_name = config_info.dataroute_filename;
    //printf("broker_file_name = %s\n", broker_file_name);
    char *broker_md5 = config_info.dataroute_md5;
    //printf("broker_md5 = %s\n", broker_md5);//68fef19a09282f7994cc9fb9599a6743
    sprintf(ftpcmd, "qftp get %s -l %s -p %s -r %s %s\n", broker_IP, broker_username, broker_pwd,  broker_file_dir, broker_file_name );
    printf("%s\n",ftpcmd);
    int ret = system(ftpcmd);//获取配置文件压缩包文件

    if( ret )
    {
        printf("obtain software failed!");
          int i ;
        for ( i = 0;i < 3;i++)
        {
            int ret = system(ftpcmd);//获取配置文件压缩包文件
            if(!ret)
                break;
        }
    }
    else
    {
        char md5cmd[FTP_LEN] = { 0 };
        char LineValue[MAX_LEN] = { 0 };
        char md5value[MAX_LEN] = { 0 };
        sprintf(md5cmd, "md5sum %s >%s", broker_file_name, UPDATE_MD5_DIR);
        int ret = system(md5cmd);
        //printf("ret =%d,  %s\n",ret, md5cmd);

        FILE* fp = NULL;
        fp = fopen(UPDATE_MD5_DIR, "r");
        if(NULL == fp)
        {
            printf("Get File %s Failed!\n", UPDATE_MD5_DIR);
            return -1;
        }
        while( fgets(LineValue, MAX_LEN, fp)!=0)
        {
            if (strstr(LineValue,"updateconfig.tar.gz")!=0)
            {
                strncpy(md5value,LineValue,MD5_LEN);
                md5value[33]="\0";
            }
        }
        //fgets(md5value, MD5_LEN, fp);
        //printf("111111!!!\n");
        //printf("md5value checking = %s\n",md5value);
       // printf("broker_md5 checking = %s*******\n",broker_md5);

        if( strncmp(md5value,broker_md5,MD5_LEN)==0 )
        {
            char tarcmd[FTP_LEN] = {0};
            sprintf(tarcmd, "tar -zvxf %s -C %s", broker_file_name, CONFIG_UPDATE_DIR);
            printf(" %s\n",tarcmd);
            int ret = system(tarcmd);
            if( ret )
            {
                printf("tar -zvxf updateconfig.tar.gz failed!\n");
            }
            else
            {
                system("rm -rf updateconfig.tar.gz");
            }
            //printf("2222222!!!\n");
            printf("download configuration file!!!\n");
            //system("reboot");
        }
        // printf("3333333!!!\n");
    }
}

//{"Update":"14.215.130.186++/vsftp/gprs/update++dataroute.tar.gz++V1.00++678a1cd5d78f5910ea57e48d661f98d2"}
int parse_down_update(char *data_temp)
{
    memset(&update_info,0,sizeof(dataroute_update_info));
    char *cmd_data = NULL;
    int cmd_length = 0;
    int line_len = 0;
    int len = 0;
    int res = 0;
    /* Establish string and get the first token: */
    char* token = strtok( data_temp, "\"");
    if (token == NULL)
    {
        printf("cmd token  is error!\n");
        return 0;
    }

    int i = 0;
    while( token != NULL )
    {
        i++;
        /* Get next token: */
        token = strtok( NULL, "\"");
        if (i == 1)
        {
            if (strcmp(token,"Update") != 0)
            {
                printf("modbus command type is error!\n");
                return 0;
            }
            printf( "cmd_tpye = %s\n", token );
        }
        else if (i == 3)
        {
            cmd_data = token;
            //printf( "cmd = %s\n", cmd_data );
            break;
        }
    }

    if (strstr(cmd_data, "++") == NULL)
    {
          printf("update command error!");
          return 0;
    }
    else
    {
        token = strtok( cmd_data, "++");
        i = 0;
        while( token != NULL )
        {
            /* While there are tokens in "string" */

            //printf( "token=%s\c", token );
            switch (i)
            {
            case 0://14.215.130.186
                memcpy(update_info.dataroute_ftp_address,token,sizeof(update_info.dataroute_ftp_address));
#ifdef MQTT_DEBUG
                printf("dataroute_ftp_address = %s\n",update_info.dataroute_ftp_address);
#endif
                break;
            case 1:// /vsftp/gprs/update
                memcpy(update_info.dataroute_route,token,sizeof(update_info.dataroute_route));
#ifdef MQTT_DEBUG
                printf("dataroute_route = %s\n",update_info.dataroute_route);
#endif
                break;
            case 2://dataroute.tar.gz
                memcpy(update_info.dataroute_filename,token,sizeof(update_info.dataroute_filename));
#ifdef MQTT_DEBUG
                printf("dataroute_filename = %s\n",update_info.dataroute_filename);
#endif
                break;
            case 3://V1.00
                memcpy(update_info.dataroute_version,token,sizeof(update_info.dataroute_version));
#ifdef MQTT_DEBUG
                printf("dataroute_version = %s\n",update_info.dataroute_version);
#endif
                char sedCmd[MAX_LEN];
                sprintf(sedCmd,"sed -i '3c        option version '%s' %s",update_info.dataroute_version,MQTT_XML_PATH);
                printf("%s\n",sedCmd);
                system(sedCmd);
                break;
            case 4://678a1cd5d78f5910ea57e48d661f98d2
                memcpy(update_info.dataroute_md5,token,sizeof(update_info.dataroute_md5));
#ifdef MQTT_DEBUG
               printf("dataroute_md5 = %s\n",update_info.dataroute_md5);
#endif
                break;
            default:
                break;
            }

            /* Get next token: */
            token = strtok( NULL, "++");
            i++;
        }
    }

    update_run();
}


//{"Config":"14.215.130.186++/vsftp/gprs/update++updateconfig.tar.gz++V1.00++678a1cd5d78f5910ea57e48d661f98d2"}
int parse_down_config(char* data_temp)
{
    int line_len = 0;
    int len = 0;
    int res = 0;
#ifdef MQTT_DEBUG
    printf("parse_down_config***********************\n");
#endif
    memset(&config_info,0,sizeof(dataroute_update_info));
    char *cmd_data = NULL;
    char* token =strtok(data_temp,"\"");
    if(token == NULL)
    {
        printf("config cmd is error!!");
    }
    int i = 0;
    while(token != NULL)
    {
        i++;
        /* Get next token: */
        token = strtok( NULL, "\"");
        if (i == 1)
        {
            if (strcmp(token,"Config") != 0)
            {
                printf("modbus command type is error!\n");
                return 0;
            }
            printf( "cmd_tpye = %s\n", token );
        }
        else if (i == 3)
        {
            cmd_data = token;
            //printf( "cmd = %s\n", cmd_data );
            break;
        }
    }

    if (strstr(cmd_data, "++") == NULL)
    {
          printf("Config command error!");
          return 0;
    }
    else
    {
        token = strtok( cmd_data, "++");
        i = 0;
        while( token != NULL )
        {
            /* While there are tokens in "string" */

            //printf( "token=%s\c", token );
            switch (i)
            {
            case 0://14.215.130.186
                memcpy(config_info.dataroute_ftp_address,token,sizeof(config_info.dataroute_ftp_address));
#ifdef MQTT_DEBUG
                printf("dataroute_ftp_address = %s\n",config_info.dataroute_ftp_address);
#endif
                break;
            case 1:// /vsftp/gprs/update
                memcpy(config_info.dataroute_route,token,sizeof(config_info.dataroute_route));
#ifdef MQTT_DEBUG
                printf("dataroute_route = %s\n",config_info.dataroute_route);
#endif
                break;
            case 2://updateconfig.tar.gz
                memcpy(config_info.dataroute_filename,token,sizeof(config_info.dataroute_filename));
#ifdef MQTT_DEBUG
                printf("dataroute_filename = %s\n",config_info.dataroute_filename);
#endif
                break;
            case 3://V1.00
                memcpy(config_info.dataroute_version,token,sizeof(config_info.dataroute_version));
#ifdef MQTT_DEBUG
                printf("dataroute_version = %s\n",config_info.dataroute_version);
#endif
                break;
            case 4://678a1cd5d78f5910ea57e48d661f98d2
                memcpy(config_info.dataroute_md5,token,sizeof(config_info.dataroute_md5));
#ifdef MQTT_DEBUG
               printf("dataroute_md5 = %s\n",config_info.dataroute_md5);
#endif
                break;
            default:
                break;
            }

            /* Get next token: */
            token = strtok( NULL, "++");
            i++;
        }
    }

    update_config();

}

//在该函数中区别是远程下发还是远程升级
int parse_down_strings(char *data, int length)
{
    printf("parse_down_strings!!!!\n");
    char data_temp[1024] = {0};
    memcpy(data_temp,data,strlen(data)+1);
   printf("cmd is *******%s!\n",data_temp);
    if (data_temp[0] != '{' || data_temp[length-1] != '}' )
    {
        printf("cmd is error,or cmd is not modbus command !\n");
        return 0;
    }

    if (((strstr(data_temp,"Control") != NULL)) || ((strstr(data_temp,"Control2") != NULL)))
    {
        parse_down_modbus(data_temp, length);
    }
    //payload={"Update":"V1.00++md5"}
    //payload={"Update":"V1.00++e11ebd4dba78473335c7d1433ea13ed5"}
    else if ((strstr(data_temp,"Update") != NULL))
    {
        parse_down_update(data_temp);
    }
    else if((strstr(data_temp,"Config") != NULL))
    {
        parse_down_config(data_temp);
    }
    else if((strstr(data_temp,"UploadAll")!=NULL))
    {
        printf("parse_up_all_data!!!!\n");
        parse_up_all_data(data_temp,length);
    }
}
#endif
