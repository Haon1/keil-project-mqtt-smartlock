#include "stm32f4xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "utils_hmac.h"
#include "includes.h"


int mqtt_connect_broker_flag=0;

uint32_t g_mqtt_tx_len;

char MQTT_BROKERADDRESS[128]; 		//IOT??ַ//"xxx.iot-as-mqtt.cn-shanghai.aliyuncs.com"
int  MQTT_BROKERPORT;				//IOT?˿?

char MQTT_CLIENTID[128]; 			//CLIENT ID "***|securemode=3,signmethod=hmacsha1|"
int  CLIENTID_LEN;
char MQTT_USERNAME[128]; 			//?û???"***&xxx"
int  USERNAME_LEN;
char MQTT_PASSWD[128]; 				//????   
int  PASSWD_LEN;


unsigned char  mqtt_rx_buf[R_NUM][BUFF_UNIT];            //???ݵĽ??ջ?????,???з???????��?????ݣ??????ڸû?????,????????һ???ֽڴ??????ݳ???
unsigned char *mqtt_rx_inptr;                            //ָ?????ջ????????????ݵ?λ??
unsigned char *mqtt_rx_outptr;                           //ָ?????ջ???????ȡ???ݵ?λ??
unsigned char *mqtt_rx_endptr;                           //ָ?????ջ???????????λ??

unsigned char  mqtt_tx_buf[T_NUM][BUFF_UNIT];            //???ݵķ??ͻ?????,???з??????????????ݣ??????ڸû?????,????????һ???ֽڴ??????ݳ???
unsigned char *mqtt_tx_inptr;                            //ָ?????ͻ????????????ݵ?λ??
unsigned char *mqtt_tx_outptr;                           //ָ?????ͻ???????ȡ???ݵ?λ??
unsigned char *mqtt_tx_endptr;                           //ָ?????ͻ???????????λ??

unsigned char  mqtt_cmd_buf[C_NUM][BUFF_UNIT];               //???????ݵĽ??ջ?????
unsigned char *mqtt_cmd_inptr;                               //ָ????????????????ݵ?λ??
unsigned char *mqtt_cmd_outptr;                              //ָ???????????ȡ???ݵ?λ??
unsigned char *mqtt_cmd_endptr;                              //ָ???????????????λ??



/*----------------------------------------------------------*/
/*?????�����???Ƴ?ʼ?????????õ??ͻ???ID???û?????????      */
/*??  ??????                                                */
/*????ֵ????                                                */
/*----------------------------------------------------------*/
void AliIoT_Parameter_Init(void)
{	
	char temp[128];                                                       //???????ܵ?ʱ??????ʱʹ?õĻ?????

	memset(MQTT_CLIENTID,128,0);                                               //?ͻ???ID?Ļ?????ȫ??????
	sprintf(MQTT_CLIENTID,"%s|securemode=3,signmethod=hmacsha1|",DEVICENAME);  //?????ͻ???ID???????뻺????
	CLIENTID_LEN = strlen(MQTT_CLIENTID);                                      //?????ͻ???ID?ĳ???
	
	memset(MQTT_USERNAME,128,0);                                               //?û????Ļ?????ȫ??????
	sprintf(MQTT_USERNAME,"%s&%s",DEVICENAME,PRODUCTKEY);                      //?????û??�����???뻺????
	USERNAME_LEN = strlen(MQTT_USERNAME);                                      //?????û????ĳ???
	
	memset(temp,128,0);                                                                      //??ʱ??????ȫ??????
	sprintf(temp,"clientId%sdeviceName%sproductKey%s",DEVICENAME,DEVICENAME,PRODUCTKEY);     //????????ʱ??????   
	utils_hmac_sha1(temp, strlen(temp), DEVICESECRE, DEVICESECRE_LEN, MQTT_PASSWD);                 //??DeviceSecretΪ??Կ??temp?е????ģ?????hmacsha1???ܣ????????????룬?????浽????????
	PASSWD_LEN = strlen(MQTT_PASSWD);                                                         //?????û????ĳ???
	
	memset(MQTT_BROKERADDRESS,128,0);  
	sprintf(MQTT_BROKERADDRESS,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",PRODUCTKEY);                  //??????????????
	MQTT_BROKERPORT = 1883;                                                                       //???????˿ں?1883
	
	printf("\r\n");
	printf("?? ?? ????%s:%d\r\n",MQTT_BROKERADDRESS,MQTT_BROKERPORT); //??????????????Ϣ
	printf("?ͻ???ID??%s\r\n",MQTT_CLIENTID);               //??????????????Ϣ
	printf("?? ?? ????%s\r\n",MQTT_USERNAME);               //??????????????Ϣ
	printf("??    ?룺%s\r\n",MQTT_PASSWD);               //??????????????Ϣ
	printf("\r\n");
}

void mqtt_buffer_init(void)
{
	mqtt_rx_inptr  = mqtt_rx_buf[0];
	mqtt_rx_outptr = mqtt_rx_inptr;
    mqtt_rx_endptr = mqtt_rx_buf[R_NUM-1];

	mqtt_tx_inptr  = mqtt_tx_buf[0];
	mqtt_tx_outptr = mqtt_tx_inptr;
    mqtt_tx_endptr = mqtt_tx_buf[R_NUM-1];
	
	mqtt_cmd_inptr  = mqtt_cmd_buf[0];
	mqtt_cmd_outptr = mqtt_cmd_inptr;
    mqtt_cmd_endptr = mqtt_cmd_buf[R_NUM-1];
}




void mqtt_tx_buf_deal(unsigned char *data, int size)
{
	memcpy(&mqtt_tx_inptr[2],data,size);
	mqtt_tx_inptr[0] = BYTE0(size);
	mqtt_tx_inptr[1] = BYTE1(size);
	mqtt_tx_inptr+=BUFF_UNIT;
	if(mqtt_tx_inptr==mqtt_tx_endptr)
		mqtt_tx_inptr = mqtt_tx_buf[0];
}


void mqtt_rx_buf_deal(unsigned char *data, int size)
{
	memcpy(&mqtt_rx_inptr[2],data,size);	
	mqtt_rx_inptr[0] = BYTE0(size);
	mqtt_rx_inptr[1] = BYTE1(size);
	mqtt_rx_inptr+=BUFF_UNIT;
	if(mqtt_rx_inptr==mqtt_rx_endptr)
		mqtt_rx_inptr = mqtt_rx_buf[0];
}

//MQTT发送数据
void mqtt_send_bytes(uint8_t *buf,uint32_t len)
{
    esp8266_send_bytes(buf,len);
}

//发送心跳包
int32_t mqtt_send_heart(void)
{	
	uint8_t buf[2]={0xC0,0x00};
    uint32_t cnt=2;
    uint32_t wait=0;	
	
#if 0	
	mqtt_send_bytes(buf,2);
	return 0;
#else
    while(cnt--)
    {	
		mqtt_send_bytes(buf,2);
		memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		g_esp8266_rx_cnt=0;	
		
		wait=3000;//?ȴ?3sʱ??
		
		while(wait--)
		{
			delay_ms(1);

			//??????????Ӧ?̶???ͷ
			if((g_esp8266_rx_buf[0]==0xD0) && (g_esp8266_rx_buf[1]==0x00)) 
			{
				printf("??????Ӧȷ?ϳɹ?????????????\r\n");
				return 0;
			}
		}
	}
	printf("??????Ӧȷ??ʧ?ܣ???????????\r\n");
	return -1;
#endif	

}

//MQTT???????Ͽ?
void mqtt_disconnect(void)
{
	uint8_t buf[2]={0xe0,0x00};
	//?豸????
    mqtt_send_bytes(buf,2);
	//?Ͽ???BROKER��??
	esp8266_disconnect_server();
}


/**
 * @brief ????ʣ?೤?Ȳ?????ռ???ֽ???
 * 
 * @param buf ת????Ҫ???ŵĵ?ַ
 * @param length Ҫת?????ֽ???
 * @return int ת????ռ?õ??ֽ???
 */
int mqtt_packet_encode(unsigned char *buf, int length)
{
    int bytes = 0;
 
    do {
        char d = length % 128;
        length /= 128;
        /* if there are more digits to encode, set the top bit of this digit */
        if (length > 0) {
            d |= 0x80;
        }
        buf[bytes++] = d;
    } while (length > 0);
    return bytes;
}

/**
 * @brief ʣ?೤??ת??Ϊʮ??????
 * @param buf ʣ?೤?ȵ?ַ
 * @param length ת??ʮ??????????
 * @return int ʣ?೤????ռ?ֽ???
 */
int mqtt_packet_decrypt_encode(const unsigned char *buf, int *length)
{
	int bytes = 0;
	char ch, tmp;
	*length = 0;

	do
	{
		tmp = ch = buf[bytes];
		if(tmp & 0x80)
		{
			tmp ^= 0x80;
		}
		*length += tmp * pow(128,bytes);
		bytes++;
	}while(ch & 0x80);
	//计算最后一个字节
	*length += buf[bytes] * pow(128,bytes);
	bytes++;

	return bytes;
}

//发送MQTT连接报文
int32_t mqtt_connect_packet(void)
{
    uint32_t data_len;
    g_mqtt_tx_len=0;
	
	
    data_len = 10 + (CLIENTID_LEN+2) + (USERNAME_LEN+2) + (PASSWD_LEN+2);

	g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x10;		//MQTT Message Type CONNECT

	g_mqtt_tx_len += mqtt_packet_encode(&g_esp8266_tx_buf[g_mqtt_tx_len], data_len);
	

	g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;        	// Protocol Name Length MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 4;        	// Protocol Name Length LSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'M';        // ASCII Code for M
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'Q';        // ASCII Code for Q
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'T';        // ASCII Code for T
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'T';        // ASCII Code for T

    g_esp8266_tx_buf[g_mqtt_tx_len++] = 4;        	// MQTT Protocol version = 4

    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xc2;        // conn flags
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;        	// Keep-alive Time Length MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 60;        	// Keep-alive Time Length LSB  60S

    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(CLIENTID_LEN);// Client ID length MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(CLIENTID_LEN);// Client ID length LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],MQTT_CLIENTID,CLIENTID_LEN);
    g_mqtt_tx_len += CLIENTID_LEN;

    if(USERNAME_LEN > 0)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(USERNAME_LEN);		//user_name length MSB
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(USERNAME_LEN);    	//user_name length LSB
        memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],MQTT_USERNAME,USERNAME_LEN);
        g_mqtt_tx_len += USERNAME_LEN;
    }

    if(PASSWD_LEN > 0)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(PASSWD_LEN);		//password length MSB
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(PASSWD_LEN);    	//password length LSB
        memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],MQTT_PASSWD,PASSWD_LEN);
        g_mqtt_tx_len += PASSWD_LEN;
    }

	memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
	g_esp8266_rx_cnt=0;
	
	mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
		
	
    return 0;
}

//MQTT????/ȡ?????????ݴ???????
//topic       ????
//qos         ??Ϣ?ȼ?
//whether     ????/ȡ????????????
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{	
    uint32_t topiclen = strlen(topic);

    uint32_t data_len = 2 + (topiclen+2) + (whether?1:0);//?ɱ䱨ͷ?ĳ??ȣ?2?ֽڣ???????Ч?غɵĳ???
	
	g_mqtt_tx_len=0;
	
    //?̶???ͷ
    //???Ʊ???????
    if(whether) 
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x82; //??Ϣ???ͺͱ?־????
    else	
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xA2; //ȡ??????

    //ʣ?೤??
	g_mqtt_tx_len += mqtt_packet_encode(&g_esp8266_tx_buf[g_mqtt_tx_len], data_len);

    //?ɱ䱨ͷ
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;				//??Ϣ??ʶ?? MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x01;           //??Ϣ??ʶ?? LSB
	
    //??Ч?غ?
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topiclen);//???ⳤ?? MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topiclen);//???ⳤ?? LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topiclen);
	
    g_mqtt_tx_len += topiclen;

    if(whether)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = qos;//QoS????
    }


	g_esp8266_rx_cnt=0;
	memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
	mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
		

	
    return 0;
}


uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{
	static uint16_t id=0;	
    uint32_t topicLength = strlen(topic);
    uint32_t messageLength = strlen(message);

    uint32_t data_len;

    g_mqtt_tx_len=0;

    if(qos)	data_len = (2+topicLength) + 2 + messageLength;
    else	data_len = (2+topicLength) + messageLength;

    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x30;    // MQTT Message Type PUBLISH
	
	g_mqtt_tx_len += mqtt_packet_encode(&g_esp8266_tx_buf[g_mqtt_tx_len], data_len);

    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topicLength);
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topicLength);
	
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topicLength);
	
    g_mqtt_tx_len += topicLength;

    if(qos)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(id);
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(id);
        id++;
    }
	
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],message,messageLength);
	
    g_mqtt_tx_len += messageLength;

	mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
	
	
	return g_mqtt_tx_len;
}




void mqtt_report_devices_status(void)
{
    uint8_t led_1_sta = GPIO_ReadOutputDataBit(GPIOF,GPIO_Pin_9) ? 0:1;
    uint8_t led_2_sta = GPIO_ReadOutputDataBit(GPIOF,GPIO_Pin_10) ? 0:1;
    uint8_t led_3_sta = GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_13) ? 0:1;

    sprintf((char *)g_esp8266_tx_buf,
            "{\"method\":\"thing.service.property.post\",\"id\":\"0001\",\"params\":{\
		\"temperature\":%d,\
		\"Humidity\":%d,\
		\"switch_led_1\":%d,\
		\"switch_led_2\":%d,\
		\"switch_led_3\":%d,\
	},\"version\":\"1.0.0\"}",
            1,
            2,
            led_1_sta,
            led_2_sta,
            led_3_sta);

    mqtt_publish_data(MQTT_PUBLISH_TOPIC,(char *)g_esp8266_tx_buf,0);
}

//连接broker
int32_t esp8266_connect_ali_broker(void)
{
	int32_t 	rt;
	OS_ERR 		err;
	
	rt =esp8266_connect_server("TCP",MQTT_BROKERADDRESS,1883);
	if(rt)
	{
		printf("esp8266_connect_server fail\r\n");
		return -2;
	}	
	printf("esp8266_connect_server success\r\n");
	
	delay_ms(2000);
	
	//进入透传
	rt =esp8266_entry_transparent_transmission();
	if(rt)
	{
		printf("esp8266_entry_transparent_transmission fail\r\n");
		return -3;
	}	
	printf("esp8266_entry_transparent_transmission success\r\n");
	delay_ms(2000);
	
	
	return 0;
}

//mqtt消息接收处理
void mqtt_receive_handle(unsigned char *recv_buf)
{
	int	bytes;
	unsigned short recv_length = 0;
	unsigned char *p = recv_buf+2;
	int encode_len=0;
	
	
	recv_length = recv_buf[0] | recv_buf[1] << 8;	
	bytes  = mqtt_packet_decrypt_encode(&p[1],&encode_len);
	
	printf("\r\n-------------------------------msg------------------------------\r\n");
	printf("receive length:[%d] type:[%#02x] encode_len:[%d] encode_byte:[%d]\r\n",\
												recv_length,p[0],encode_len,bytes);
	printf("-----------------------------------------------------------------\r\n");
	
	switch(p[0])
	{
		case 0x20:		//CONNACK
		{
			if(p[3] == 0x00)
			{
				printf("The connection has been accepted by the server\r\n");
			}
			else
			{
				switch(p[3])
				{
					case 1:printf("connection refused, unsupported protocol version\r\n");
					break;
					case 2:printf("connection refused, unqualified client identification\r\n");
					break;		
					case 3:printf("connection refused, server is unavailable\r\n");
					break;		
					case 4:printf("connection refused, invalid username or password\r\n");
					break;	
					case 5:printf("connection refused, unauthorized\r\n");
					break;
					default:printf("unknown mistake\r\n");
					break;
				}
			} 
		}break;
		
		case 0x90:		//SUBACK
		{
			switch(p[4])
			{
				case 0:printf("subcribe success - Maximum QoS 0\r\n");
				break;
				case 1:printf("subcribe success - Maximum QoS 1\r\n");
				break;
				case 2:printf("subcribe success - Maximum QoS 2\r\n");
				break;
				case 128:printf("subcribe failure\r\n");
				break;
			}
		}break;
		
		case 0xD0:		//PINGRESP
		{
			printf("get ping response\r\n");
		}break;
	}

	printf("-----------------------------------------------------------------\r\n");
}
