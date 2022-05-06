#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "rtc.h"
#include "rgb_led.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "includes.h"

//任务1控制块
OS_TCB TASK_SYSTEM_INIT_TCB;
void task_system_init(void *parg);
CPU_STK task_system_init_stk[512];			//任务堆栈，大小为128字，也就是512字节



//任务LED控制块
OS_TCB TASK_RGB_LED_TCB;
void task_rgb_led(void *parg);
CPU_STK task_rgb_led_stk[512];			//任务2的任务堆栈，大小为128字，也就是512字节

//任务LED控制块
OS_TCB TASK_RTC_TCB;
void task_rtc(void *parg);
CPU_STK task_rtc_stk[512];			//任务2的任务堆栈，大小为128字，也就是512字节



OS_MUTEX				g_mutex_printf;			//互斥锁的对象
OS_FLAG_GRP   			g_flag_grp;				//事件标志组	


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


//主函数
int main(void)
{
	OS_ERR err;

	systick_init();  													//时钟初始化
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//中断分组配置
	
	usart_init(115200);  				 									//串口初始化


	//OS初始化，它是第一个运行的函数,初始化各种的全局变量，例如中断嵌套计数器、优先级、存储器
	OSInit(&err);


	//创建任务1
	OSTaskCreate(	(OS_TCB *)&TASK_SYSTEM_INIT_TCB,									//任务控制块，等同于线程id
					(CPU_CHAR *)"task_system_init",									//任务的名字，名字可以自定义的
					(OS_TASK_PTR)task_system_init,										//任务函数，等同于线程函数
					(void *)0,												//传递参数，等同于线程的传递参数
					(OS_PRIO)6,											 	//任务的优先级6		
					(CPU_STK *)task_system_init_stk,									//任务堆栈基地址
					(CPU_STK_SIZE)512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)512,										//任务堆栈大小			
					(OS_MSG_QTY)0,											//禁止任务消息队列
					(OS_TICK)0,												//默认时间片长度																
					(void  *)0,												//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
					
	if(err!=OS_ERR_NONE)
	{
		printf("task 1 create fail\r\n");
		
		while(1);
	
	}

					

	//启动OS，进行任务调度
	OSStart(&err);
							
	printf(".......\r\n");
					
	while(1);
	
}

//系统初始化任务
void task_system_init(void *parg)
{
	int32_t rt;
	OS_ERR err;
	printf("task_system_init is create ok\r\n");
	
	esp8266_init();
	while(esp8266_connect_ap())
	{
		printf("esp8266 init fail\r\n");
	}
	
//	rt = esp8266_connect_time_server();
//	if(rt)
//		while(1);
//	rt = esp8266_get_network_time();
//	if(rt)
//		while(1);
//	
//	printf("%s\r\n",g_esp8266_rx_buf);
//	
//	time_json_parse(strstr((char *)g_esp8266_rx_buf,"{"));
//	
//	
//	rt = esp8266_disconnect_time_server();
//	if(rt)
//		while(1);

	
	//如果读寄存器不是 0x80  则获取网络时间 设置  rtc  并写寄存器0x80  
	//如果是 0x80   则  从备份寄存器初始化rtc   
	
	//LED_Init();         												//LED初始化	
	rgb_led_init();
	//AliIoT_Parameter_Init();
	
	//年月日星期时分秒
	rtc_init(22,5,6,5,0,9,30);		//手动设置时间
	
	//创建互斥锁
	OSMutexCreate(&g_mutex_printf,	"g_mutex_printf",&err);
	
	//创建事件标志组
	OSFlagCreate(&g_flag_grp,"g_flag_grp",0,&err);
	
	
	//创建任务
	OSTaskCreate(	(OS_TCB *)&TASK_RGB_LED_TCB,									//任务控制块
					(CPU_CHAR *)"task_rgb_led",									//任务的名字
					(OS_TASK_PTR)task_rgb_led,										//任务函数
					(void *)0,												//传递参数
					(OS_PRIO)6,											 	//任务的优先级7		
					(CPU_STK *)task_rgb_led_stk,									//任务堆栈基地址
					(CPU_STK_SIZE)512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)512,										//任务堆栈大小			
					(OS_MSG_QTY)0,											//禁止任务消息队列
					(OS_TICK)0,												//默认时间片长度																
					(void  *)0,												//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
					
		//创建任务
	OSTaskCreate(	(OS_TCB *)&TASK_RTC_TCB,									//任务控制块
					(CPU_CHAR *)"task_rtc",									//任务的名字
					(OS_TASK_PTR)task_rtc,										//任务函数
					(void *)0,												//传递参数
					(OS_PRIO)6,											 	//任务的优先级7		
					(CPU_STK *)task_rtc_stk,									//任务堆栈基地址
					(CPU_STK_SIZE)512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					(CPU_STK_SIZE)512,										//任务堆栈大小			
					(OS_MSG_QTY)0,											//禁止任务消息队列
					(OS_TICK)0,												//默认时间片长度																
					(void  *)0,												//不需要补充用户存储区
					(OS_OPT)OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
					
	//删除自身任务，进入休眠态
	OSTaskDel(NULL,&err);
}

//led任务
void task_rgb_led(void *parg)
{
	int color=1;
	
	printf("task_rgb_led is create ok\r\n");

	while(1)
	{
		rgb_led_set_color(color++);
		
		delay_ms(1000);
		if(color >  7)
			color = 1;
	}
}

//rtc 
void task_rtc(void *parg)
{
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
	printf("task_rtc is create ok\r\n");
	

	while(1)
	{
		if(g_rtc_wakeup_event)
		{
			//获取日期
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			printf("20%02x/%02x/%02x Week:%x ",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);			
					
			//获取时间
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			printf("%02x:%02x:%02x\r\n",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			g_rtc_wakeup_event = 0;
		}
	}
}








