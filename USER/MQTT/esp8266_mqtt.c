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

char MQTT_BROKERADDRESS[128]; 		//IOTµØÖ·//"xxx.iot-as-mqtt.cn-shanghai.aliyuncs.com"
int  MQTT_BROKERPORT;				//IOT¶Ë¿Ú

char MQTT_CLIENTID[128]; 			//CLIENT ID "***|securemode=3,signmethod=hmacsha1|"
int  CLIENTID_LEN;
char MQTT_USERNAME[128]; 			//ÓÃ»§Ãû"***&xxx"
int  USERNAME_LEN;
char MQTT_PASSWD[128]; 				//ÃÜÂë   
int  PASSWD_LEN;


unsigned char  mqtt_rx_buf[R_NUM][BUFF_UNIT];            //Êı¾İµÄ½ÓÊÕ»º³åÇø,ËùÓĞ·şÎñÆ÷·¢À´µÄÊı¾İ£¬´æ·ÅÔÚ¸Ã»º³åÇø,»º³åÇøµÚÒ»¸ö×Ö½Ú´æ·ÅÊı¾İ³¤¶È
unsigned char *mqtt_rx_inptr;                            //Ö¸Ïò½ÓÊÕ»º³åÇø´æ·ÅÊı¾İµÄÎ»ÖÃ
unsigned char *mqtt_rx_outptr;                           //Ö¸Ïò½ÓÊÕ»º³åÇø¶ÁÈ¡Êı¾İµÄÎ»ÖÃ
unsigned char *mqtt_rx_endptr;                           //Ö¸Ïò½ÓÊÕ»º³åÇø½áÊøµÄÎ»ÖÃ

unsigned char  mqtt_tx_buf[T_NUM][BUFF_UNIT];            //Êı¾İµÄ·¢ËÍ»º³åÇø,ËùÓĞ·¢Íù·şÎñÆ÷µÄÊı¾İ£¬´æ·ÅÔÚ¸Ã»º³åÇø,»º³åÇøµÚÒ»¸ö×Ö½Ú´æ·ÅÊı¾İ³¤¶È
unsigned char *mqtt_tx_inptr;                            //Ö¸Ïò·¢ËÍ»º³åÇø´æ·ÅÊı¾İµÄÎ»ÖÃ
unsigned char *mqtt_tx_outptr;                           //Ö¸Ïò·¢ËÍ»º³åÇø¶ÁÈ¡Êı¾İµÄÎ»ÖÃ
unsigned char *mqtt_tx_endptr;                           //Ö¸Ïò·¢ËÍ»º³åÇø½áÊøµÄÎ»ÖÃ

unsigned char  mqtt_cmd_buf[C_NUM][BUFF_UNIT];               //ÃüÁîÊı¾İµÄ½ÓÊÕ»º³åÇø
unsigned char *mqtt_cmd_inptr;                               //Ö¸ÏòÃüÁî»º³åÇø´æ·ÅÊı¾İµÄÎ»ÖÃ
unsigned char *mqtt_cmd_outptr;                              //Ö¸ÏòÃüÁî»º³åÇø¶ÁÈ¡Êı¾İµÄÎ»ÖÃ
unsigned char *mqtt_cmd_endptr;                              //Ö¸ÏòÃüÁî»º³åÇø½áÊøµÄÎ»ÖÃ



/*----------------------------------------------------------*/
/*º¯ÊıÃû£º°¢ÀïÔÆ³õÊ¼»¯²ÎÊı£¬µÃµ½¿Í»§¶ËID£¬ÓÃ»§ÃûºÍÃÜÂë      */
/*²Î  Êı£ºÎŞ                                                */
/*·µ»ØÖµ£ºÎŞ                                                */
/*----------------------------------------------------------*/
void AliIoT_Parameter_Init(void)
{	
	char temp[128];                                                       //¼ÆËã¼ÓÃÜµÄÊ±ºò£¬ÁÙÊ±Ê¹ÓÃµÄ»º³åÇø

	memset(MQTT_CLIENTID,128,0);                                               //¿Í»§¶ËIDµÄ»º³åÇøÈ«²¿ÇåÁã
	sprintf(MQTT_CLIENTID,"%s|securemode=3,signmethod=hmacsha1|",DEVICENAME);  //¹¹½¨¿Í»§¶ËID£¬²¢´æÈë»º³åÇø
	CLIENTID_LEN = strlen(MQTT_CLIENTID);                                      //¼ÆËã¿Í»§¶ËIDµÄ³¤¶È
	
	memset(MQTT_USERNAME,128,0);                                               //ÓÃ»§ÃûµÄ»º³åÇøÈ«²¿ÇåÁã
	sprintf(MQTT_USERNAME,"%s&%s",DEVICENAME,PRODUCTKEY);                      //¹¹½¨ÓÃ»§Ãû£¬²¢´æÈë»º³åÇø
	USERNAME_LEN = strlen(MQTT_USERNAME);                                      //¼ÆËãÓÃ»§ÃûµÄ³¤¶È
	
	memset(temp,128,0);                                                                      //ÁÙÊ±»º³åÇøÈ«²¿ÇåÁã
	sprintf(temp,"clientId%sdeviceName%sproductKey%s",DEVICENAME,DEVICENAME,PRODUCTKEY);     //¹¹½¨¼ÓÃÜÊ±µÄÃ÷ÎÄ   
	utils_hmac_sha1(temp, strlen(temp), DEVICESECRE, DEVICESECRE_LEN, MQTT_PASSWD);                 //ÒÔDeviceSecretÎªÃØÔ¿¶ÔtempÖĞµÄÃ÷ÎÄ£¬½øĞĞhmacsha1¼ÓÃÜ£¬½á¹û¾ÍÊÇÃÜÂë£¬²¢±£´æµ½»º³åÇøÖĞ
	PASSWD_LEN = strlen(MQTT_PASSWD);                                                         //¼ÆËãÓÃ»§ÃûµÄ³¤¶È
	
	memset(MQTT_BROKERADDRESS,128,0);  
	sprintf(MQTT_BROKERADDRESS,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",PRODUCTKEY);                  //¹¹½¨·şÎñÆ÷ÓòÃû
	MQTT_BROKERPORT = 1883;                                                                       //·şÎñÆ÷¶Ë¿ÚºÅ1883
	
	printf("\r\n");
	printf("·ş Îñ Æ÷£º%s:%d\r\n",MQTT_BROKERADDRESS,MQTT_BROKERPORT); //´®¿ÚÊä³öµ÷ÊÔĞÅÏ¢
	printf("¿Í»§¶ËID£º%s\r\n",MQTT_CLIENTID);               //´®¿ÚÊä³öµ÷ÊÔĞÅÏ¢
	printf("ÓÃ »§ Ãû£º%s\r\n",MQTT_USERNAME);               //´®¿ÚÊä³öµ÷ÊÔĞÅÏ¢
	printf("ÃÜ    Âë£º%s\r\n",MQTT_PASSWD);               //´®¿ÚÊä³öµ÷ÊÔĞÅÏ¢
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

//MQTTå‘é€æ•°æ®
void mqtt_send_bytes(uint8_t *buf,uint32_t len)
{
    esp8266_send_bytes(buf,len);
}

//å‘é€å¿ƒè·³åŒ…
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
		
		wait=3000;//µÈ´ı3sÊ±¼ä
		
		while(wait--)
		{
			delay_ms(1);

			//¼ì²éĞÄÌøÏìÓ¦¹Ì¶¨±¨Í·
			if((g_esp8266_rx_buf[0]==0xD0) && (g_esp8266_rx_buf[1]==0x00)) 
			{
				printf("ĞÄÌøÏìÓ¦È·ÈÏ³É¹¦£¬·şÎñÆ÷ÔÚÏß\r\n");
				return 0;
			}
		}
	}
	printf("ĞÄÌøÏìÓ¦È·ÈÏÊ§°Ü£¬·şÎñÆ÷ÀëÏß\r\n");
	return -1;
#endif	

}

//MQTTÎŞÌõ¼ş¶Ï¿ª
void mqtt_disconnect(void)
{
	uint8_t buf[2]={0xe0,0x00};
	//Éè±¸ÏÂÏß
    mqtt_send_bytes(buf,2);
	//¶Ï¿ªÓëBROKERÁ¬½Ó
	esp8266_disconnect_server();
}


/**
 * @brief ¼ÆËãÊ£Óà³¤¶È²¢·µ»ØÕ¼ÓÃ×Ö½ÚÊı
 * 
 * @param buf ×ª»»ºóÒª´æ·ÅµÄµØÖ·
 * @param length Òª×ª»»µÄ×Ö½ÚÊı
 * @return int ×ª»»ºóÕ¼ÓÃµÄ×Ö½ÚÊı
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
 * @brief Ê£Óà³¤¶È×ª»»ÎªÊ®½øÖÆÊı
 * @param buf Ê£Óà³¤¶ÈµØÖ·
 * @param length ×ª³ÉÊ®½øÖÆÊı´æ·Å
 * @return int Ê£Óà³¤¶ÈËùÕ¼×Ö½ÚÊı
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
	//è®¡ç®—æœ€åä¸€ä¸ªå­—èŠ‚
	*length += buf[bytes] * pow(128,bytes);
	bytes++;

	return bytes;
}

//å‘é€MQTTè¿æ¥æŠ¥æ–‡
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

//MQTT¶©ÔÄ/È¡Ïû¶©ÔÄÊı¾İ´ò°üº¯Êı
//topic       Ö÷Ìâ
//qos         ÏûÏ¢µÈ¼¶
//whether     ¶©ÔÄ/È¡Ïû¶©ÔÄÇëÇó°ü
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{	
    uint32_t topiclen = strlen(topic);

    uint32_t data_len = 2 + (topiclen+2) + (whether?1:0);//¿É±ä±¨Í·µÄ³¤¶È£¨2×Ö½Ú£©¼ÓÉÏÓĞĞ§ÔØºÉµÄ³¤¶È
	
	g_mqtt_tx_len=0;
	
    //¹Ì¶¨±¨Í·
    //¿ØÖÆ±¨ÎÄÀàĞÍ
    if(whether) 
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x82; //ÏûÏ¢ÀàĞÍºÍ±êÖ¾¶©ÔÄ
    else	
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xA2; //È¡Ïû¶©ÔÄ

    //Ê£Óà³¤¶È
	g_mqtt_tx_len += mqtt_packet_encode(&g_esp8266_tx_buf[g_mqtt_tx_len], data_len);

    //¿É±ä±¨Í·
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;				//ÏûÏ¢±êÊ¶·û MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x01;           //ÏûÏ¢±êÊ¶·û LSB
	
    //ÓĞĞ§ÔØºÉ
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topiclen);//Ö÷Ìâ³¤¶È MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topiclen);//Ö÷Ìâ³¤¶È LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topiclen);
	
    g_mqtt_tx_len += topiclen;

    if(whether)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = qos;//QoS¼¶±ğ
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

//è¿æ¥broker
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
	
	//è¿›å…¥é€ä¼ 
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

//mqttæ¶ˆæ¯æ¥æ”¶å¤„ç†
void mqtt_receive_handle(unsigned char *recv_buf)
{
	int	bytes;
	unsigned short recv_length = 0;
	unsigned char *p = recv_buf+2;
	int encode_len=0;
	
	
	recv_length = recv_buf[0] | recv_buf[1] << 8;	
	encode_len  = mqtt_packet_decrypt_encode(&p[1],&bytes);
	
	printf("recv_length  %d \r\n",recv_length);
	printf("receive type %#02x\r\n",p[0]);
	printf("encode_len  %d\r\n",encode_len);
	
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
		
	}
}
