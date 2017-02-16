#include "upgrade.h"
#include "common.h"
#include <mosquitto.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

struct mosquitto *mosq = NULL;
void * libmqtt_handle = NULL;
int    (*my_mosquitto_publish)(struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain);
int    (*my_mosquitto_loop)(struct mosquitto *mosq, int timeout, int max_packets);
void (*my_mosquitto_destroy)(struct mosquitto *mosq);
int    (*my_mosquitto_lib_cleanup)(void);
int   (*my_mosquitto_reconnect )(struct mosquitto *mosq);

void reconnect()
{
    my_mosquitto_reconnect(mosq);
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if(message->payloadlen){
        printf("%s %s\n", message->topic, message->payload);
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, void *libmqtt_handle,int result)
{
    int i;
    if(!result){
        /* Subscribe to broker information topics on successful connect. */
        int    (*mosquitto_subscribe)(struct mosquitto *mosq, int *mid, const char *sub, int qos);
        mosquitto_subscribe = dlsym(libmqtt_handle,"mosquitto_subscribe");
//        mosquitto_subscribe(mosq, NULL, "$SYS/#", 2);
        mosquitto_subscribe(mosq, NULL, "invt", mqttInfo->iQos);
    }else{
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;

    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
}

int initMqtt()
{
    bool clean_session = true;
    libmqtt_handle = dlopen("libmosquitto.so.1", RTLD_LAZY);
    if (!libmqtt_handle)
    {
        printf("dlopen error:%s.\n",dlerror());
        return -1;
     }

    int   (*mosquitto_lib_init)(void);
    struct mosquitto* (*mosquitto_new)(const char *id, bool clean_session, void *obj);
    int (*mosquitto_username_pw_set)(struct mosquitto *mosq, const char *username, const char *password);
    void (*mosquitto_log_callback_set)(struct mosquitto *mosq, void (*on_log)(struct mosquitto *, void *, int, const char *));
    void (*mosquitto_connect_callback_set)(struct mosquitto *mosq, void (*on_connect)(struct mosquitto *, void *, void *,int));
    void (*mosquitto_message_callback_set)(struct mosquitto *mosq, void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *));
    void (*mosquitto_subscribe_callback_set)(struct mosquitto *mosq, void (*on_subscribe)(struct mosquitto *, void *, int, int, const int *));
    int    (*mosquitto_connect)(struct mosquitto *mosq, const char *host, int port, int keepalive);
    int    (*mosquitto_connect_bind)(struct mosquitto *mosq, const char *host, int port, int keepalive,const char *bind_address);
    int    (*mosquitto_loop_forever)(struct mosquitto *mosq, int timeout, int max_packets);
       int    (*mosquitto_reconnect)(struct mosquitto *mosq);

    mosquitto_lib_init = dlsym(libmqtt_handle,"mosquitto_lib_init");
    mosquitto_new = dlsym(libmqtt_handle,"mosquitto_new");
    mosquitto_username_pw_set = dlsym(libmqtt_handle,"mosquitto_username_pw_set");
    mosquitto_log_callback_set = dlsym(libmqtt_handle,"mosquitto_log_callback_set");
    mosquitto_connect_callback_set = dlsym(libmqtt_handle,"mosquitto_connect_callback_set");
    mosquitto_message_callback_set = dlsym(libmqtt_handle,"mosquitto_message_callback_set");
    mosquitto_subscribe_callback_set = dlsym(libmqtt_handle,"mosquitto_subscribe_callback_set");
    mosquitto_connect = dlsym(libmqtt_handle,"mosquitto_connect");
    mosquitto_connect_bind = dlsym(libmqtt_handle,"mosquitto_connect_bind");
    my_mosquitto_loop = dlsym(libmqtt_handle,"mosquitto_loop");
    mosquitto_loop_forever = dlsym(libmqtt_handle,"mosquitto_loop_forever");
    my_mosquitto_destroy = dlsym(libmqtt_handle,"mosquitto_destroy");
    my_mosquitto_lib_cleanup = dlsym(libmqtt_handle,"mosquitto_lib_cleanup");
    my_mosquitto_publish = dlsym(libmqtt_handle,"mosquitto_publish");
    my_mosquitto_reconnect = dlsym(libmqtt_handle,"mosquitto_reconnect");

    mosquitto_lib_init();
    mosq = mosquitto_new(mqttInfo->strClientID, clean_session, NULL);
    if(!mosq)
    {
        printf("mosquitto_new error.\n");
        return -1;
    }

    if(mosquitto_username_pw_set(mosq, mqttInfo->strUseName, mqttInfo->strPwd) != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_username_pw_set error.\n");
        return -1;
    }
    mosquitto_log_callback_set(mosq, my_log_callback);
    //mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

    //if(mosquitto_connect(mosq, host, port, keepalive) != MOSQ_ERR_SUCCESS)
    if(mosquitto_connect_bind(mosq, mqttInfo->strAddress, mqttInfo->iPort, mqttInfo->iKeepalive, mqttInfo->strBindAddtess) != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_connect_bind error.\n");
        return -1;
    }
}

void destroyMqtt()
{
    my_mosquitto_destroy(mosq);
    my_mosquitto_lib_cleanup();
    dlclose(libmqtt_handle);
}

int send_package_version(char* SendBuff)
{
    int send = 0;
    int iRet = my_mosquitto_publish(mosq, &send, mqttInfo->strTopic, strlen(SendBuff), SendBuff, 2, true);
    if( iRet!= MOSQ_ERR_SUCCESS)
    {
        printf("iRet = %d, my_mosquitto_publish error.\n", iRet);
        return -1;
    }
    my_mosquitto_loop(mosq, mqttInfo->ulTimeout, 1);
    return send;
}

int get_package_version(char* buf)
{
    int i, j;
    char src[255] = {0};
    system("opkg list datarouter > /root/packageversion");
    FILE* fp = fopen("/root/packageversion", "rb");
    if(fp == NULL)
    {
        printf("get_package_version error:%s.\n", strerror(errno));
        return -1;
    }
    fread(src, 255, 1, fp);
    fclose(fp);

    buf = src;
    /*for(i = 0; i < strlen(src); i++)
    {
        if((src[i] == '-') && (src[i+1] == 0x20) && (src[i+2] >= '0') && (src[i+2] <= '9'))
        {
            j = i+2;
            break;
        }
    }
    for(i = 0; i < strlen(src) - j; i++)
    {
        if(src[i+j] == '-')
            break;
        buf[i] = src[i+j];
    }*/

    system("rm ~/packageversion");

    return 0;
}


