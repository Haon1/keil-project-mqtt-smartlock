#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <time.h>
#include "esp8266.h"

#define RESET_IO(x) PAout(4)=x

uint8_t  		  g_esp8266_tx_buf[512];
volatile uint8_t  g_esp8266_rx_buf[1024];
volatile uint32_t g_esp8266_rx_cnt=0;
volatile uint32_t g_esp8266_rx_end=0;

volatile uint32_t g_esp8266_transparent_transmission_sta=0;



//Ä£¿é³õÊ¼»¯
int32_t esp8266_init(void)
{	
	usart2_init(115200);
	esp8266_reset_io_init();
}


//¸´Î»Òı½Å³õÊ¼»¯
void esp8266_reset_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                      //¶¨ÒåÒ»¸öÉèÖÃIO¶Ë¿Ú²ÎÊıµÄ½á¹¹Ìå
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);   //Ê¹ÄÜPA¶Ë¿ÚÊ±ÖÓ
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                 //×¼±¸ÉèÖÃPA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//ÍÆÃâÊä³ö·½Ê½
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         //ËÙÂÊ50Mhz	  
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		  //ÉèÖÃPA4
	RESET_IO(1);                                              //¸´Î»IOÀ­¸ßµçÆ½
}

//Ä£¿éÍ¨¹ıIO¸´Î»
int esp8266_reset(int timeout)
{
	RESET_IO(0);                                    //¸´Î»IOÀ­µÍµçÆ½
	delay_ms(500);                                  //ÑÓÊ±500ms
	RESET_IO(1);                                    //¸´Î»IOÀ­¸ßµçÆ½	
	if(esp8266_find_str_in_rx_packet("ready",timeout)==0)
		return 0;						//Èç¹û½ÓÊÕµ½ready±íÊ¾¸´Î»³É¹¦
	else 
		return -1;
}

//Çå¿Õ½ÓÊÕÇø²¢·¢ËÍATÖ¸Áî
void esp8266_send_at(char *str)
{
	//Çå¿Õ½ÓÊÕ»º³åÇø
	memset((void *)g_esp8266_rx_buf,0, sizeof g_esp8266_rx_buf);
	
	//Çå¿Õ½ÓÊÕ¼ÆÊıÖµ
	g_esp8266_rx_cnt = 0;	
	
	//´®¿Ú3·¢ËÍÊı¾İ
	usart2_send_str(str);
}


void esp8266_send_bytes(uint8_t *buf,uint32_t len)
{
	usart2_send_bytes(buf,len);

}

void esp8266_send_str(char *buf)
{
	usart2_send_str(buf);

}


int32_t esp8266_find_str_in_rx_packet(char *str,uint32_t timeout)
{
	char *dest = str;
	char *src  = (char *)&g_esp8266_rx_buf;
	
	while((strstr(src,dest)==NULL) && timeout)
	{		
		delay_ms(100);		//100msæŸ¥æ‰¾ä¸€æ¬¡
		timeout -= 100;
	}

	if(timeout) 
		return 0; 
		                    
	return -1; 
}



int32_t  esp8266_self_test(void)
{
	esp8266_send_at("AT\r\n");
	
	return esp8266_find_str_in_rx_packet("OK",1000);
}

/**
 * ¹¦ÄÜ£ºÁ¬½ÓÈÈµã
 * ²ÎÊı£º
 *         ssid:ÈÈµãÃû
 *         pwd:ÈÈµãÃÜÂë
 * ·µ»ØÖµ£º
 *         Á¬½Ó½á¹û,0Á¬½Ó³É¹¦,·Ç0Á¬½ÓÊ§°Ü
 * ËµÃ÷£º 
 *         Ê§°ÜµÄÔ­ÒòÓĞÒÔÏÂ¼¸ÖÖ(UARTÍ¨ĞÅºÍESP8266Õı³£Çé¿öÏÂ)
 *         1. WIFIÃûºÍÃÜÂë²»ÕıÈ·
 *         2. Â·ÓÉÆ÷Á¬½ÓÉè±¸Ì«¶à,Î´ÄÜ¸øESP8266·ÖÅäIP
 */
int32_t __esp8266_connect_ap(char* ssid,char* pswd)
{
#if 0
	char buf[128]={0};
	
	sprintf(buf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);

#endif
	esp8266_send_at("AT+CWMODE_CUR=1\r\n"); 
	
	if(esp8266_find_str_in_rx_packet("OK",1000))
		return -1;


	//sprintf(buf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);
	esp8266_send_at("AT+CWJAP_CUR="); 
	esp8266_send_at("\"");esp8266_send_at(ssid);esp8266_send_at("\"");	
	esp8266_send_at(",");	
	esp8266_send_at("\"");esp8266_send_at(pswd);esp8266_send_at("\"");	
	esp8266_send_at("\r\n");
	
	if(esp8266_find_str_in_rx_packet("WIFI GOT IP\r\n\r\nOK",10000))
		return -2;
	
	return 0;
}

int32_t esp8266_connect_ap(void)
{
	int32_t rt;
	
	rt=esp8266_exit_transparent_transmission();
	
	if(rt)
	{
		printf("esp8266_exit_transparent_transmission fail\r\n");
		return -1;
	}	
	printf("esp8266_exit_transparent_transmission success\r\n");
	delay_ms(2000);

	rt=esp8266_restart();
	if(rt)
	{
		printf("esp8266_restart fail\r\n");
		return -2;
	}
	printf("esp8266_restart success\r\n");
	delay_ms(2000);	
	
	rt=esp8266_enable_echo(0);
	if(rt)
	{
		printf("esp8266_enable_echo(0) fail\r\n");
		return -3;
	}	
	printf("esp8266_enable_echo(0)success\r\n");
	delay_ms(2000);	
	
	rt = __esp8266_connect_ap(WIFI_SSID,WIFI_PASSWORD);
	if(rt)
	{
		printf("esp8266_connect_ap fail\r\n");
		return -4;
	}	
	printf("esp8266_connect_ap success\r\n");
	
	delay_ms(2000);
	return 0;
}


int32_t esp8266_exit_transparent_transmission (void)
{

	esp8266_send_at ("+++");
	
	delay_ms ( 1000 ); 
	
	g_esp8266_transparent_transmission_sta = 0;

	return 0;
}


int32_t  esp8266_entry_transparent_transmission(void)
{
	esp8266_send_at("AT+CIPMODE=1\r\n");  
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	delay_ms(2000);

	esp8266_send_at("AT+CIPSEND\r\n");
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -2;

	g_esp8266_transparent_transmission_sta = 1;
	return 0;
}


/**
 * ¹¦ÄÜ£ºÊ¹ÓÃÖ¸¶¨Ğ­Òé(TCP/UDP)Á¬½Óµ½·şÎñÆ÷
 * ²ÎÊı£º
 *         mode:Ğ­ÒéÀàĞÍ "TCP","UDP"
 *         ip:Ä¿±ê·şÎñÆ÷IP
 *         port:Ä¿±êÊÇ·şÎñÆ÷¶Ë¿ÚºÅ
 * ·µ»ØÖµ£º
 *         Á¬½Ó½á¹û,·Ç0Á¬½Ó³É¹¦,0Á¬½ÓÊ§°Ü
 * ËµÃ÷£º 
 *         Ê§°ÜµÄÔ­ÒòÓĞÒÔÏÂ¼¸ÖÖ(UARTÍ¨ĞÅºÍESP8266Õı³£Çé¿öÏÂ)
 *         1. Ô¶³Ì·şÎñÆ÷IPºÍ¶Ë¿ÚºÅÓĞÎó
 *         2. Î´Á¬½ÓAP
 *         3. ·şÎñÆ÷¶Ë½ûÖ¹Ìí¼Ó(Ò»°ã²»»á·¢Éú)
 */
int32_t esp8266_connect_server(char* mode,char* ip,uint16_t port)
{

#if 0	
	//Ê¹ÓÃMQTT´«µİµÄipµØÖ·¹ı³¤£¬²»½¨ÒéÊ¹ÓÃÒÔÏÂ·½·¨£¬·ñÔòµ¼ÖÂÕ»Òç³ö
	//AT+CIPSTART="TCP","a10tC4OAAPc.iot-as-mqtt.cn-shanghai.aliyuncs.com",1883£¬¸Ã×Ö·û´®Õ¼ÓÃÄÚ´æ¹ı¶àÁË
	
	char buf[128]={0};
	

	sprintf((char*)buf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
	
	esp8266_send_at(buf);
#else
	
	char buf[16]={0};
	esp8266_send_at("AT+CIPSTART=");
	esp8266_send_at("\"");	esp8266_send_at(mode);	esp8266_send_at("\"");
	esp8266_send_at(",");
	esp8266_send_at("\"");	esp8266_send_at(ip);	esp8266_send_at("\"");	
	esp8266_send_at(",");
	sprintf(buf,"%d",port);
	esp8266_send_at(buf);	
	esp8266_send_at("\r\n");
	
#endif
	
	if(esp8266_find_str_in_rx_packet("CONNECT",5000))
		if(esp8266_find_str_in_rx_packet("OK",5000))
			return -1;
	return 0;
}



int32_t esp8266_disconnect_server(void)
{
	esp8266_send_at("AT+CIPCLOSE\r\n");
		
	if(esp8266_find_str_in_rx_packet("CLOSED",5000))
		if(esp8266_find_str_in_rx_packet("OK",5000))
			return -1;
	
	return 0;	
}


int32_t esp8266_get_network_time(void)
{
	int32_t rt;
	
	rt = esp8266_connect_time_server();
	if(rt)
	{
		printf("esp8266_connect_time_server fail\r\n");
		return -1;
	}	
	printf("esp8266_connect_time_server success\r\n");
	
	
	rt =esp8266_entry_transparent_transmission();
	if(rt)
	{
		printf("esp8266_entry_transparent_transmission fail\r\n");
		return -2;
	}	
	printf("esp8266_entry_transparent_transmission success\r\n");
	delay_ms(2000);
	
	esp8266_send_at(REQUEST);
	
	if(esp8266_find_str_in_rx_packet(SUCCESS_PACKET,10000))
		return -3;
	
	return 0;
}


int32_t esp8266_connect_time_server(void)
{
	int32_t rt;
	
	rt =esp8266_connect_server("TCP",TIME_SERVER_IP,TIME_SERVER_PORT);
	if(rt)
		return -1;
	
	delay_ms(2000);
	return 0;
}


int32_t esp8266_disconnect_time_server(void)
{
	int32_t rt;
	

	rt=esp8266_exit_transparent_transmission();
	if(rt)
	{
		printf("esp8266_exit_transparent_transmission fail\r\n");
		return -1;
	}
	printf("esp8266_exit_transparent_transmission success\r\n");
	
	rt = esp8266_disconnect_server();
	if(rt)
	{
		printf("esp8266 disconnect time server fail\r\n");
		return -2;
	}
	
	printf("esp8266 disconnect time server success\r\n");
	
	return 0;
}


int32_t esp8266_enable_multiple_id(uint32_t b)
{

	char buf[32]={0};
	
	sprintf(buf,"AT+CIPMUX=%d\r\n", b);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}


int32_t esp8266_create_server(uint16_t port)
{
	char buf[32]={0};
	
	sprintf(buf,"AT+CIPSERVER=1,%d\r\n", port);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}


int32_t esp8266_close_server(uint16_t port)
{
	char buf[32]={0};
	
	sprintf(buf,"AT+CIPSERVER=0,%d\r\n", port);
	esp8266_send_at(buf);
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;
	
	return 0;
}


int32_t esp8266_enable_echo(uint32_t b)
{
	if(b)
		esp8266_send_at("ATE1\r\n"); 
	else
		esp8266_send_at("ATE0\r\n"); 
	
	if(esp8266_find_str_in_rx_packet("OK",5000))
		return -1;

	return 0;
}


int32_t esp8266_restart(void)
{
	esp8266_send_at("AT+RST\r\n");
	
	if(esp8266_find_str_in_rx_packet("OK",10000))
		return -1;

	return 0;
}
