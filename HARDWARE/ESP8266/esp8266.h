#ifndef __ESP8266_H__
#define __ESP8266_H__


#define EN_DEBUG_ESP8266	0
//添加WIFI热点宏定义，此处根据自己的wifi作调整

#define WIFI_SSID 			"HiXyan"
#define WIFI_PASSWORD		"19990321"

//获取网络时间戳api
//请求 URL: http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp
//返回{"api":"mtop.common.getTimestamp","v":"*","ret":["SUCCESS::接口调用成功"],"data":{"t":"1651751177482"}}
#define TIME_SERVER_IP		"106.11.52.98"
#define TIME_SERVER_PORT	80
#define	REQUEST				"GET /rest/api3.do?api=mtop.common.getTimestamp HTTP/1.1\r\nHost:api.m.taobao.com\r\n\r\n"
#define SUCCESS_PACKET		"SUCCESS"



extern uint8_t  g_esp8266_tx_buf[512];
extern volatile uint8_t  g_esp8266_rx_buf[1024];
extern volatile uint32_t g_esp8266_rx_cnt;

extern volatile uint32_t g_esp8266_transparent_transmission_sta;


void 		esp8266_reset_io_init(void);	//复位引脚初始化
int 		esp8266_reset(int timeout);		//物理复位
int32_t 	esp8266_init(void);
int32_t  	esp8266_self_test(void);
int32_t 	esp8266_exit_transparent_transmission (void);
int32_t 	esp8266_entry_transparent_transmission(void);
int32_t 	__esp8266_connect_ap(char* ssid,char* pswd);
int32_t 	esp8266_connect_ap(void);
int32_t 	esp8266_connect_server(char* mode,char* ip,uint16_t port);
int32_t 	esp8266_disconnect_server(void);
void 		esp8266_send_bytes(uint8_t *buf,uint32_t len);
void 		esp8266_send_str(char *buf);
void 		esp8266_send_at(char *str);
int32_t  	esp8266_enable_multiple_id(uint32_t b);
int32_t 	esp8266_create_server(uint16_t port);
int32_t 	esp8266_close_server(uint16_t port);
int32_t 	esp8266_enable_echo(uint32_t b);
int32_t 	esp8266_restart(void);

int32_t esp8266_find_str_in_rx_packet(char *str,uint32_t timeout);

/*获取网络时间*/
int32_t 	esp8266_connect_time_server(void);
int32_t 	esp8266_get_network_time(void);
int32_t 	esp8266_disconnect_time_server(void);


;
#endif




