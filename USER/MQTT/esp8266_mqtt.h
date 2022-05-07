#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



/****************�˴��ǰ����Ʒ������Ĺ���ʵ����½����****************************************/
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

/****************************************************************************************************/

#define	BUFF_UNIT	512		//����������
#define R_NUM		6		//���ջ���������
#define	T_NUM		6		//���ͻ���������
#define	C_NUM		6		//�����������


extern unsigned char  MQTT_RxDataBuf[R_NUM][BUFF_UNIT];        //�ⲿ�������������ݵĽ��ջ�����,���з��������������ݣ�����ڸû�����,��������һ���ֽڴ�����ݳ���
extern unsigned char *MQTT_RxDataInPtr;                        //�ⲿ����������ָ�򻺳���������ݵ�λ��
extern unsigned char *MQTT_RxDataOutPtr;                       //�ⲿ����������ָ�򻺳�����ȡ���ݵ�λ��
extern unsigned char *MQTT_RxDataEndPtr;                       //�ⲿ����������ָ�򻺳���������λ��

extern unsigned char  MQTT_TxDataBuf[T_NUM][BUFF_UNIT];        //�ⲿ�������������ݵķ��ͻ�����,���з��������������ݣ�����ڸû�����,��������һ���ֽڴ�����ݳ���
extern unsigned char *MQTT_TxDataInPtr;                        //�ⲿ����������ָ�򻺳���������ݵ�λ��
extern unsigned char *MQTT_TxDataOutPtr;                       //�ⲿ����������ָ�򻺳�����ȡ���ݵ�λ��
extern unsigned char *MQTT_TxDataEndPtr;                       //�ⲿ����������ָ�򻺳���������λ��

extern unsigned char  MQTT_CMDBuf[C_NUM][BUFF_UNIT];           //�ⲿ�����������������ݵĽ��ջ�����
extern unsigned char *MQTT_CMDInPtr;                           //�ⲿ����������ָ�򻺳���������ݵ�λ��
extern unsigned char *MQTT_CMDOutPtr;                          //�ⲿ����������ָ�򻺳�����ȡ���ݵ�λ��
extern unsigned char *MQTT_CMDEndPtr;                          //�ⲿ����������ָ�򻺳���������λ��


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	

//���Ӳ�����ʼ��
void AliIoT_Parameter_Init(void);

//��������ʼ��
void mqtt_buffer_init(void);

void TxDataBuf_Deal(unsigned char *data, int size);
void RxDataBuf_Deal(unsigned char *data, int size);

//����ʣ�೤��,���������ֽ���
int mqtt_packet_encode(unsigned char *buf, int length);

//MQTT���ӷ�����
int32_t mqtt_connect_packet(void);

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
