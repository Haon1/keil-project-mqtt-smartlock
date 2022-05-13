#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



/****************此处是阿里云服务器的公共实例登陆配置****************************************/
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

/****************************************************************************************************/

#define	BUFF_UNIT	512		//缓冲区长度
#define R_NUM		6		//接收缓冲区个数
#define	T_NUM		6		//发送缓冲区个数
#define	C_NUM		6		//命令缓冲区个数


extern	unsigned char  mqtt_rx_buf[R_NUM][BUFF_UNIT];            //数据的接收缓冲区,所有服务器发来的数据，存放在该缓冲区,缓冲区第一个字节存放数据长度
extern	unsigned char *mqtt_rx_inptr;                            //指向接收缓冲区存放数据的位置
extern	unsigned char *mqtt_rx_outptr;                           //指向接收缓冲区读取数据的位置
extern	unsigned char *mqtt_rx_endptr;                           //指向接收缓冲区结束的位置

extern	unsigned char  mqtt_tx_buf[T_NUM][BUFF_UNIT];            //数据的发送缓冲区,所有发往服务器的数据，存放在该缓冲区,缓冲区第一个字节存放数据长度
extern	unsigned char *mqtt_tx_inptr;                            //指向发送缓冲区存放数据的位置
extern	unsigned char *mqtt_tx_outptr;                           //指向发送缓冲区读取数据的位置
extern	unsigned char *mqtt_tx_endptr;                           //指向发送缓冲区结束的位置

extern	unsigned char  mqtt_cmd_buf[C_NUM][BUFF_UNIT];               //命令数据的接收缓冲区
extern	unsigned char *mqtt_cmd_inptr;                               //指向命令缓冲区存放数据的位置
extern	unsigned char *mqtt_cmd_outptr;                              //指向命令缓冲区读取数据的位置
extern	unsigned char *mqtt_cmd_endptr;                              //指向命令缓冲区结束的位置


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	

//连接参数初始化
void AliIoT_Parameter_Init(void);

//缓冲区初始化
void mqtt_buffer_init(void);

void mqtt_tx_buf_deal(unsigned char *data, int size);
void mqtt_rx_buf_deal(unsigned char *data, int size);

//计算剩余长度,返回所用字节数
int mqtt_packet_encode(unsigned char *buf, int length);
//剩余长度解析,返回所用字节数,转成十进制数存放到length中
int mqtt_packet_decrypt_encode(const unsigned char *buf, int *length);

//MQTT连接服务器
int32_t mqtt_connect_packet(void);

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
