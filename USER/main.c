#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "rtc.h"
#include "tim3.h"
#include "rgb_led.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "includes.h"

//mqtt msg deal callback
typedef void(*RecvCallBack)(unsigned char *recv_buf);
RecvCallBack RCBS=NULL;


// system init task
OS_TCB TASK_SYSTEM_INIT_TCB;
void task_system_init(void *parg);
CPU_STK task_system_init_stk[512];	

// led task
OS_TCB TASK_RGB_LED_TCB;
void task_rgb_led(void *parg);
CPU_STK task_rgb_led_stk[512];		

// RTC task
OS_TCB TASK_RTC_TCB;
void task_rtc(void *parg);
CPU_STK task_rtc_stk[512];	

// mqtt connect task
OS_TCB TASK_MQTT_CONNECT_TCB;
void task_mqtt_connect(void *parg);
CPU_STK task_mqtt_connect_stk[512];			

//mqtt receive buffer deal task
OS_TCB TASK_RX_BUF_DEAL_TCB;
void task_rx_buf_deal(void *parg);
CPU_STK task_rx_buf_deal_stk[128];

// mqtt receive buffer deal task
OS_TCB TASK_TX_BUF_DEAL_TCB;
void task_tx_buf_deal(void *parg);
CPU_STK task_tx_buf_deal_stk[128];	


// kernel object
OS_MUTEX				g_mutex_printf;		
OS_FLAG_GRP   			g_flag_grp;			


#define DEBUG_PRINTF_EN	1
void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN	
	OS_ERR err;
	
	va_list args;
	va_start(args, format);
	
	OSMutexPend(&g_mutex_printf,0,OS_OPT_PEND_BLOCKING,NULL,&err);	
	vprintf(format, args);
	OSMutexPost(&g_mutex_printf,OS_OPT_POST_NONE,&err);
	
	va_end(args);
#else
	(void)0;
#endif
}

//mqtt receive callback register
void RecvCBRegister(RecvCallBack pCBS)
{
	if(RCBS == NULL)
		RCBS = pCBS;
}

int main(void)
{
	OS_ERR err;

	systick_init();  													
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						
	
	usart_init(115200);  				 								

	OSInit(&err);

	OSTaskCreate(	(OS_TCB *)&TASK_SYSTEM_INIT_TCB,
					(CPU_CHAR *)"task_system_init",	
					(OS_TASK_PTR)task_system_init,	
					(void *)0,						
					(OS_PRIO)6,						
					(CPU_STK *)task_system_init_stk,
					(CPU_STK_SIZE)512/10,			
					(CPU_STK_SIZE)512,				
					(OS_MSG_QTY)0,					
					(OS_TICK)0,						
					(void  *)0,						
					(OS_OPT)OS_OPT_TASK_NONE,		
					&err												
				);
					
	if(err!=OS_ERR_NONE)
	{
		printf("task 1 create fail\r\n");
		
		while(1);
	
	}

					

	//start OS
	OSStart(&err);
							
	printf(".......\r\n");
					
	while(1);
	
}


void task_system_init(void *parg)
{
	int32_t rt;
	OS_ERR err;
	printf("task_system_init is create ok\r\n");
	
	//esp8266 hardware init
	esp8266_init();

	//connect ap
	while(esp8266_connect_ap());
	
	//sync rtc
	rt = sync_local_time();
	if(rt == -1)			
	{
		printf("sync time fail\r\n");
		while(1);
	}
	else if(rt == 1)		
	{
		if(esp8266_exit_transparent_transmission()==0)	
			printf("exit transmission success\r\n");
		
		while(esp8266_disconnect_server())				
			printf("disconnect time server fail\r\n");
		printf("disconnect time server success\r\n");
	}
	printf("sync time success\r\n");
	
	
	
	
	//LED_Init();
         											
	beep_init();
	
	rgb_led_init();
	
	tim3_init();
	
	//callback register
	RecvCBRegister(mqtt_receive_handle);
	
	//create mutex
	OSMutexCreate(&g_mutex_printf,	"g_mutex_printf",&err);
	
	//create flag group
	OSFlagCreate(&g_flag_grp,"g_flag_grp",0,&err);
	
	
	//create task  led
	OSTaskCreate(	(OS_TCB *)&TASK_RGB_LED_TCB,
					(CPU_CHAR *)"task_rgb_led",
					(OS_TASK_PTR)task_rgb_led,
					(void *)0,
					(OS_PRIO)7,
					(CPU_STK *)task_rgb_led_stk,
					(CPU_STK_SIZE)512/10,
					(CPU_STK_SIZE)512,
					(OS_MSG_QTY)0,
					(OS_TICK)0,
					(void  *)0,
					(OS_OPT)OS_OPT_TASK_NONE,
					&err
				);
	//create task rtc		
	OSTaskCreate(	(OS_TCB *)&TASK_RTC_TCB,
					(CPU_CHAR *)"task_rtc",
					(OS_TASK_PTR)task_rtc,
					(void *)0,
					(OS_PRIO)7,
					(CPU_STK *)task_rtc_stk,
					(CPU_STK_SIZE)512/10,
					(CPU_STK_SIZE)512,
					(OS_MSG_QTY)0,
					(OS_TICK)0,
					(void  *)0,
					(OS_OPT)OS_OPT_TASK_NONE,
					&err
				);
	//create task  mqtt connect		
	OSTaskCreate(	(OS_TCB *)&TASK_MQTT_CONNECT_TCB,
					(CPU_CHAR *)"task_mqtt_connect",
					(OS_TASK_PTR)task_mqtt_connect,
					(void *)0,
					(OS_PRIO)6,
					(CPU_STK *)task_mqtt_connect_stk,
					(CPU_STK_SIZE)512/10,
					(CPU_STK_SIZE)512,
					(OS_MSG_QTY)0,
					(OS_TICK)0,
					(void  *)0,
					(OS_OPT)OS_OPT_TASK_NONE,
					&err
				);
	//create task		
	OSTaskCreate(	(OS_TCB *)&TASK_RX_BUF_DEAL_TCB,						
					(CPU_CHAR *)"task_rx_buf_deal",							
					(OS_TASK_PTR)task_rx_buf_deal,							
					(void *)0,												
					(OS_PRIO)5,
					(CPU_STK *)task_rx_buf_deal_stk,
					(CPU_STK_SIZE)128/10,
					(CPU_STK_SIZE)128,
					(OS_MSG_QTY)0,
					(OS_TICK)0,
					(void  *)0,
					(OS_OPT)OS_OPT_TASK_NONE,								
					&err													
				);
	//create task
	OSTaskCreate(	(OS_TCB *)&TASK_TX_BUF_DEAL_TCB,						
					(CPU_CHAR *)"task_tx_buf_deal",							
					(OS_TASK_PTR)task_tx_buf_deal,							
					(void *)0,												
					(OS_PRIO)7,
					(CPU_STK *)task_tx_buf_deal_stk,
					(CPU_STK_SIZE)128/10,
					(CPU_STK_SIZE)128,
					(OS_MSG_QTY)0,
					(OS_TICK)0,
					(void  *)0,
					(OS_OPT)OS_OPT_TASK_NONE,								
					&err													
				);
					
	//	Delete its own task and enter the suspended state
	OSTaskDel(NULL,&err);
}



//mqtt connect
void task_mqtt_connect(void *parg)
{
	OS_ERR 		err;
	
	printf("task_mqtt_connect is create ok\r\n");
	
	AliIoT_Parameter_Init();		

	while(1)
	{
		OSFlagPend(&g_flag_grp,FLAG_GRP_MQTT_CONNECT,0,\
										OS_OPT_PEND_FLAG_CLR_ALL+\
										OS_OPT_PEND_BLOCKING, NULL, &err);		
		mqtt_buffer_init();

		//connect ali broker
		while(esp8266_connect_ali_broker());
		
		//set mqtt connect broker flag  1
		OSFlagPost(&g_flag_grp,FLAG_GRP_MQTT_CONNECT,OS_OPT_POST_FLAG_SET,&err);
		mqtt_connect_broker_flag = 1;
		
		//send mqtt connect packet
		mqtt_connect_packet();

		delay_ms(2000);		
		
		//send subcribe packet 
		mqtt_subscribe_topic(MQTT_SUBSCRIBE_TOPIC,0,1);
		
	}
}


//Process the data in the receive buffer
void task_rx_buf_deal(void *parg)
{
	OS_ERR 		err;
	
	printf("task_rx_buf_deal is create ok\r\n");
	

	while(1)
	{
		OSFlagPend(&g_flag_grp,FLAG_GRP_ESP8266_RX_END,0,\
										OS_OPT_PEND_FLAG_SET_ALL+\
										OS_OPT_PEND_BLOCKING, NULL, &err);	
		
		while(mqtt_rx_inptr != mqtt_rx_outptr)
		{			
			RCBS(mqtt_rx_outptr);
			
			mqtt_rx_outptr += BUFF_UNIT;
			if(mqtt_rx_outptr == mqtt_rx_endptr)
				mqtt_rx_outptr = mqtt_rx_buf[0];
			delay_ms(1000);
		}
		
		//After processing, clear the flag bit
		OSFlagPost(&g_flag_grp,FLAG_GRP_ESP8266_RX_END,OS_OPT_POST_FLAG_CLR,&err);
	}
}


//Process the data in the send buffer
void task_tx_buf_deal(void *parg)
{
	OS_ERR 		err;
	
	printf("task_tx_buf_deal is create ok\r\n");
	

	while(1)
	{
		OSFlagPend(&g_flag_grp,FLAG_GRP_ESP8266_RX_END,0,\
										OS_OPT_PEND_FLAG_SET_ALL+\
										OS_OPT_PEND_BLOCKING, NULL, &err);	
		
		while(mqtt_tx_inptr != mqtt_tx_outptr)
		{

			mqtt_tx_outptr += BUFF_UNIT;
			if(mqtt_tx_outptr == mqtt_tx_endptr)
				mqtt_tx_outptr = mqtt_tx_buf[0];
			
			delay_ms(100);		//delay 100ms
		}
		
	}
}


//task block led
void task_rgb_led(void *parg)
{
	int color=1;
	
	printf("task_rgb_led is create ok\r\n");

	while(1)
	{
//		rgb_led_set_color(color++);
//		
//		delay_ms(1000);
//		if(color >  7)
//			color = 1;
	}
}

//task block rtc 
void task_rtc(void *parg)
{
	OS_ERR err;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
	printf("task_rtc is create ok\r\n");
	

	while(1)
	{
		//Block waiting for RTC to wake up
		OSFlagPend(&g_flag_grp,FLAG_GRP_RTC_WAKEUP,0,OS_OPT_PEND_FLAG_SET_ALL +\
													OS_OPT_PEND_FLAG_CONSUME + \
													OS_OPT_PEND_BLOCKING, NULL, &err);
		RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
		printf("20%02x/%02x/%02x Week:%x ",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);			
				
		RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
		printf("%02x:%02x:%02x\r\n",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
	}
}








