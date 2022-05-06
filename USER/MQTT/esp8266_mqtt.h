#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



//�˴��ǰ����Ʒ������Ĺ���ʵ����½����-------------------------------------ע���޸�Ϊ�Լ����Ʒ����豸��Ϣ��������
#define  PRODUCTKEY           "a1EExFlVDfn"                                 //��ƷID
#define  PRODUCTKEY_LEN       strlen(PRODUCTKEY)                            //��ƷID����
#define  DEVICENAME           "D001"                                        //�豸��  
#define  DEVICENAME_LEN       strlen(DEVICENAME)                            //�豸������
#define  DEVICESECRE          "e6eaac3c4a814b4b13cc3a6a78a4deb3"            //�豸��Կ   
#define  DEVICESECRE_LEN      strlen(DEVICESECRE)                           //�豸��Կ����

extern char MQTT_BROKERADDRESS[128]; 	
extern int  MQTT_BROKERPORT;	

extern char MQTT_CLIENTID[128];
extern int  CLIENTID_LEN;
extern char MQTT_USERNAME[128];
extern int  USERNAME_LEN;
extern char MQTT_PASSWD[128]; 
extern int  PASSWD_LEN;

#define	MQTT_PUBLISH_TOPIC 		"/sys/a10tC4OAAPc/smartdevice/thing/event/property/post"	//��������
#define MQTT_SUBSCRIBE_TOPIC 	"/sys/a10tC4OAAPc/smartdevice/thing/service/property/set"	//��������


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	

extern int	 mqtt_connect_flag;				//���ӵ�MQTT��־λ
	

//���Ӳ�����ʼ��
void AliIoT_Parameter_Init(void);

//MQTT���ӷ�����
int32_t mqtt_connect(char *client_id,char *user_name,char *password);

//MQTT��Ϣ����
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);

//MQTT��Ϣ����
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos);

//MQTT����������
int32_t mqtt_send_heart(void);

int32_t esp8266_mqtt_init(void);

//MQTT�Ͽ�����
void mqtt_disconnect(void);

//�ϱ��豸״̬
void mqtt_report_devices_status(void);

#endif
