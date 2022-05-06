#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



//此处是阿里云服务器的公共实例登陆配置-------------------------------------注意修改为自己的云服务设备信息！！！！
#define  PRODUCTKEY           "a1EExFlVDfn"                                 //产品ID
#define  PRODUCTKEY_LEN       strlen(PRODUCTKEY)                            //产品ID长度
#define  DEVICENAME           "D001"                                        //设备名  
#define  DEVICENAME_LEN       strlen(DEVICENAME)                            //设备名长度
#define  DEVICESECRE          "e6eaac3c4a814b4b13cc3a6a78a4deb3"            //设备秘钥   
#define  DEVICESECRE_LEN      strlen(DEVICESECRE)                           //设备秘钥长度

extern char MQTT_BROKERADDRESS[128]; 	
extern int  MQTT_BROKERPORT;	

extern char MQTT_CLIENTID[128];
extern int  CLIENTID_LEN;
extern char MQTT_USERNAME[128];
extern int  USERNAME_LEN;
extern char MQTT_PASSWD[128]; 
extern int  PASSWD_LEN;

#define	MQTT_PUBLISH_TOPIC 		"/sys/a10tC4OAAPc/smartdevice/thing/event/property/post"	//发布主题
#define MQTT_SUBSCRIBE_TOPIC 	"/sys/a10tC4OAAPc/smartdevice/thing/service/property/set"	//订阅主题


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	

extern int	 mqtt_connect_flag;				//连接到MQTT标志位
	

//连接参数初始化
void AliIoT_Parameter_Init(void);

//MQTT连接服务器
int32_t mqtt_connect(char *client_id,char *user_name,char *password);

//MQTT消息订阅
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);

//MQTT消息发布
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos);

//MQTT发送心跳包
int32_t mqtt_send_heart(void);

int32_t esp8266_mqtt_init(void);

//MQTT断开连接
void mqtt_disconnect(void);

//上报设备状态
void mqtt_report_devices_status(void);

#endif
