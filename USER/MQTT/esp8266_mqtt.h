#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



/***************************姝ゅ涓洪樋閲屼簯瀹炰緥閰嶇疆  娉ㄦ剰鏇挎崲鎴愯嚜宸辩殑****************************************/
#define  PRODUCTKEY           "a1EExFlVDfn"
#define  PRODUCTKEY_LEN       strlen(PRODUCTKEY)
#define  DEVICENAME           "D001"
#define  DEVICENAME_LEN       strlen(DEVICENAME)
#define  DEVICESECRE          "e6eaac3c4a814b4b13cc3a6a78a4deb3"
#define  DEVICESECRE_LEN      strlen(DEVICESECRE)               

extern char MQTT_BROKERADDRESS[128]; 	
extern int  MQTT_BROKERPORT;	

extern char MQTT_CLIENTID[128];
extern int  CLIENTID_LEN;

extern char MQTT_USERNAME[128];
extern int  USERNAME_LEN;

extern char MQTT_PASSWD[128]; 
extern int  PASSWD_LEN;

#define MQTT_PUBLISH_TOPIC      "/sys/a1EExFlVDfn/D001/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC    "/sys/a1EExFlVDfn/D001/thing/service/property/set"

/****************************************************************************************************/


extern int mqtt_connect_broker_flag;


#define	BUFF_UNIT	512
#define R_NUM		6
#define	T_NUM		6
#define	C_NUM		6


extern	unsigned char  mqtt_rx_buf[R_NUM][BUFF_UNIT];  
extern	unsigned char *mqtt_rx_inptr;                  
extern	unsigned char *mqtt_rx_outptr;                 
extern	unsigned char *mqtt_rx_endptr;                 

extern	unsigned char  mqtt_tx_buf[T_NUM][BUFF_UNIT];  
extern	unsigned char *mqtt_tx_inptr;                  
extern	unsigned char *mqtt_tx_outptr;                 
extern	unsigned char *mqtt_tx_endptr;                 

extern	unsigned char  mqtt_cmd_buf[C_NUM][BUFF_UNIT]; 
extern	unsigned char *mqtt_cmd_inptr;                 
extern	unsigned char *mqtt_cmd_outptr;                
extern	unsigned char *mqtt_cmd_endptr;                


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

//连接代理
int32_t esp8266_connect_ali_broker(void);

//MQTT断开连接
void mqtt_disconnect(void);

//上报设备状态
void mqtt_report_devices_status(void);


//mqtt娑堟伅鎺ユ敹澶勭悊
void mqtt_receive_handle(unsigned char *recv_buf);

#endif
