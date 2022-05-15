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

char MQTT_BROKERADDRESS[128]; 		//IOT地址//"xxx.iot-as-mqtt.cn-shanghai.aliyuncs.com"
int  MQTT_BROKERPORT;				//IOT端口

char MQTT_CLIENTID[128]; 			//CLIENT ID "***|securemode=3,signmethod=hmacsha1|"
int  CLIENTID_LEN;
char MQTT_USERNAME[128]; 			//用户名"***&xxx"
int  USERNAME_LEN;
char MQTT_PASSWD[128]; 				//密码   
int  PASSWD_LEN;


unsigned char  mqtt_rx_buf[R_NUM][BUFF_UNIT];            //数据的接收缓冲区,所有服务器发来的数据，存放在该缓冲区,缓冲区第一个字节存放数据长度
unsigned char *mqtt_rx_inptr;                            //指向接收缓冲区存放数据的位置
unsigned char *mqtt_rx_outptr;                           //指向接收缓冲区读取数据的位置
unsigned char *mqtt_rx_endptr;                           //指向接收缓冲区结束的位置

unsigned char  mqtt_tx_buf[T_NUM][BUFF_UNIT];            //数据的发送缓冲区,所有发往服务器的数据，存放在该缓冲区,缓冲区第一个字节存放数据长度
unsigned char *mqtt_tx_inptr;                            //指向发送缓冲区存放数据的位置
unsigned char *mqtt_tx_outptr;                           //指向发送缓冲区读取数据的位置
unsigned char *mqtt_tx_endptr;                           //指向发送缓冲区结束的位置

unsigned char  mqtt_cmd_buf[C_NUM][BUFF_UNIT];               //命令数据的接收缓冲区
unsigned char *mqtt_cmd_inptr;                               //指向命令缓冲区存放数据的位置
unsigned char *mqtt_cmd_outptr;                              //指向命令缓冲区读取数据的位置
unsigned char *mqtt_cmd_endptr;                              //指向命令缓冲区结束的位置



/*----------------------------------------------------------*/
/*函数名：阿里云初始化参数，得到客户端ID，用户名和密码      */
/*参  数：无                                                */
/*返回值：无                                                */
/*----------------------------------------------------------*/
void AliIoT_Parameter_Init(void)
{	
	char temp[128];                                                       //计算加密的时候，临时使用的缓冲区

	memset(MQTT_CLIENTID,128,0);                                               //客户端ID的缓冲区全部清零
	sprintf(MQTT_CLIENTID,"%s|securemode=3,signmethod=hmacsha1|",DEVICENAME);  //构建客户端ID，并存入缓冲区
	CLIENTID_LEN = strlen(MQTT_CLIENTID);                                      //计算客户端ID的长度
	
	memset(MQTT_USERNAME,128,0);                                               //用户名的缓冲区全部清零
	sprintf(MQTT_USERNAME,"%s&%s",DEVICENAME,PRODUCTKEY);                      //构建用户名，并存入缓冲区
	USERNAME_LEN = strlen(MQTT_USERNAME);                                      //计算用户名的长度
	
	memset(temp,128,0);                                                                      //临时缓冲区全部清零
	sprintf(temp,"clientId%sdeviceName%sproductKey%s",DEVICENAME,DEVICENAME,PRODUCTKEY);     //构建加密时的明文   
	utils_hmac_sha1(temp, strlen(temp), DEVICESECRE, DEVICESECRE_LEN, MQTT_PASSWD);                 //以DeviceSecret为秘钥对temp中的明文，进行hmacsha1加密，结果就是密码，并保存到缓冲区中
	PASSWD_LEN = strlen(MQTT_PASSWD);                                                         //计算用户名的长度
	
	memset(MQTT_BROKERADDRESS,128,0);  
	sprintf(MQTT_BROKERADDRESS,"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com",PRODUCTKEY);                  //构建服务器域名
	MQTT_BROKERPORT = 1883;                                                                       //服务器端口号1883
	
	printf("\r\n");
	printf("服 务 器：%s:%d\r\n",MQTT_BROKERADDRESS,MQTT_BROKERPORT); //串口输出调试信息
	printf("客户端ID：%s\r\n",MQTT_CLIENTID);               //串口输出调试信息
	printf("用 户 名：%s\r\n",MQTT_USERNAME);               //串口输出调试信息
	printf("密    码：%s\r\n",MQTT_PASSWD);               //串口输出调试信息
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

//MQTT鍙戦�佹暟鎹�
void mqtt_send_bytes(uint8_t *buf,uint32_t len)
{
    esp8266_send_bytes(buf,len);
}

//鍙戦�佸績璺冲寘
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
		
		wait=3000;//等待3s时间
		
		while(wait--)
		{
			delay_ms(1);

			//检查心跳响应固定报头
			if((g_esp8266_rx_buf[0]==0xD0) && (g_esp8266_rx_buf[1]==0x00)) 
			{
				printf("心跳响应确认成功，服务器在线\r\n");
				return 0;
			}
		}
	}
	printf("心跳响应确认失败，服务器离线\r\n");
	return -1;
#endif	

}

//MQTT无条件断开
void mqtt_disconnect(void)
{
	uint8_t buf[2]={0xe0,0x00};
	//设备下线
    mqtt_send_bytes(buf,2);
	//断开与BROKER连接
	esp8266_disconnect_server();
}


/**
 * @brief 计算剩余长度并返回占用字节数
 * 
 * @param buf 转换后要存放的地址
 * @param length 要转换的字节数
 * @return int 转换后占用的字节数
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
 * @brief 剩余长度转换为十进制数
 * @param buf 剩余长度地址
 * @param length 转成十进制数存放
 * @return int 剩余长度所占字节数
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
	//璁＄畻鏈�鍚庝竴涓瓧鑺�
	*length += buf[bytes] * pow(128,bytes);
	bytes++;

	return bytes;
}

//鍙戦�丮QTT杩炴帴鎶ユ枃
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

//MQTT订阅/取消订阅数据打包函数
//topic       主题
//qos         消息等级
//whether     订阅/取消订阅请求包
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{	
    uint32_t topiclen = strlen(topic);

    uint32_t data_len = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
	
	g_mqtt_tx_len=0;
	
    //固定报头
    //控制报文类型
    if(whether) 
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x82; //消息类型和标志订阅
    else	
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xA2; //取消订阅

    //剩余长度
	g_mqtt_tx_len += mqtt_packet_encode(&g_esp8266_tx_buf[g_mqtt_tx_len], data_len);

    //可变报头
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;				//消息标识符 MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x01;           //消息标识符 LSB
	
    //有效载荷
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topiclen);//主题长度 MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topiclen);//主题长度 LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topiclen);
	
    g_mqtt_tx_len += topiclen;

    if(whether)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = qos;//QoS级别
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

//杩炴帴broker
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
	
	//杩涘叆閫忎紶
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

//mqtt娑堟伅鎺ユ敹澶勭悊
void mqtt_receive_handle(unsigned char *recv_buf)
{
	int	bytes;							//鍓╀綑闀垮害鎵�鍗犲瓧鑺傛暟
	unsigned short recv_length = 0;		//鎺ユ敹娑堟伅鎬婚暱搴�
	int encode_len=0;					//鍓╀綑闀垮害
	
	recv_length = recv_buf[0] | recv_buf[1] << 8;	
	encode_len  = mqtt_packet_decrypt_encode(&recv_buf[3],&bytes);
	
	printf("recv_length  %d \r\n",recv_length);
	printf("receive type %#02x\r\n",recv_buf[2]);
	printf("encode_len  %d\r\n",encode_len);
	
	switch(recv_buf[2])
	{
		case 0x20:		//CONNACK
		{
			if(recv_buf[5] == 0x00)
			{
				printf("The connection has been accepted by the server\r\n");
			}
			else
			{
				switch(recv_buf[5])
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
			switch(recv_buf[6])
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
