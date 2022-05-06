#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "rtc.h"
#include "rgb_led.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "includes.h"

//����1���ƿ�
OS_TCB TASK_SYSTEM_INIT_TCB;
void task_system_init(void *parg);
CPU_STK task_system_init_stk[512];			//�����ջ����СΪ128�֣�Ҳ����512�ֽ�



//����LED���ƿ�
OS_TCB TASK_RGB_LED_TCB;
void task_rgb_led(void *parg);
CPU_STK task_rgb_led_stk[512];			//����2�������ջ����СΪ128�֣�Ҳ����512�ֽ�

//����LED���ƿ�
OS_TCB TASK_RTC_TCB;
void task_rtc(void *parg);
CPU_STK task_rtc_stk[512];			//����2�������ջ����СΪ128�֣�Ҳ����512�ֽ�



OS_MUTEX				g_mutex_printf;			//�������Ķ���
OS_FLAG_GRP   			g_flag_grp;				//�¼���־��	


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


//������
int main(void)
{
	OS_ERR err;

	systick_init();  													//ʱ�ӳ�ʼ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//�жϷ�������
	
	usart_init(115200);  				 									//���ڳ�ʼ��


	//OS��ʼ�������ǵ�һ�����еĺ���,��ʼ�����ֵ�ȫ�ֱ����������ж�Ƕ�׼����������ȼ����洢��
	OSInit(&err);


	//��������1
	OSTaskCreate(	(OS_TCB *)&TASK_SYSTEM_INIT_TCB,									//������ƿ飬��ͬ���߳�id
					(CPU_CHAR *)"task_system_init",									//��������֣����ֿ����Զ����
					(OS_TASK_PTR)task_system_init,										//����������ͬ���̺߳���
					(void *)0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					(OS_PRIO)6,											 	//��������ȼ�6		
					(CPU_STK *)task_system_init_stk,									//�����ջ����ַ
					(CPU_STK_SIZE)512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)512,										//�����ջ��С			
					(OS_MSG_QTY)0,											//��ֹ������Ϣ����
					(OS_TICK)0,												//Ĭ��ʱ��Ƭ����																
					(void  *)0,												//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
					
	if(err!=OS_ERR_NONE)
	{
		printf("task 1 create fail\r\n");
		
		while(1);
	
	}

					

	//����OS�������������
	OSStart(&err);
							
	printf(".......\r\n");
					
	while(1);
	
}

//ϵͳ��ʼ������
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

	
	//������Ĵ������� 0x80  ���ȡ����ʱ�� ����  rtc  ��д�Ĵ���0x80  
	//����� 0x80   ��  �ӱ��ݼĴ�����ʼ��rtc   
	
	//LED_Init();         												//LED��ʼ��	
	rgb_led_init();
	//AliIoT_Parameter_Init();
	
	//����������ʱ����
	rtc_init(22,5,6,5,0,9,30);		//�ֶ�����ʱ��
	
	//����������
	OSMutexCreate(&g_mutex_printf,	"g_mutex_printf",&err);
	
	//�����¼���־��
	OSFlagCreate(&g_flag_grp,"g_flag_grp",0,&err);
	
	
	//��������
	OSTaskCreate(	(OS_TCB *)&TASK_RGB_LED_TCB,									//������ƿ�
					(CPU_CHAR *)"task_rgb_led",									//���������
					(OS_TASK_PTR)task_rgb_led,										//������
					(void *)0,												//���ݲ���
					(OS_PRIO)6,											 	//��������ȼ�7		
					(CPU_STK *)task_rgb_led_stk,									//�����ջ����ַ
					(CPU_STK_SIZE)512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)512,										//�����ջ��С			
					(OS_MSG_QTY)0,											//��ֹ������Ϣ����
					(OS_TICK)0,												//Ĭ��ʱ��Ƭ����																
					(void  *)0,												//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
					
		//��������
	OSTaskCreate(	(OS_TCB *)&TASK_RTC_TCB,									//������ƿ�
					(CPU_CHAR *)"task_rtc",									//���������
					(OS_TASK_PTR)task_rtc,										//������
					(void *)0,												//���ݲ���
					(OS_PRIO)6,											 	//��������ȼ�7		
					(CPU_STK *)task_rtc_stk,									//�����ջ����ַ
					(CPU_STK_SIZE)512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					(CPU_STK_SIZE)512,										//�����ջ��С			
					(OS_MSG_QTY)0,											//��ֹ������Ϣ����
					(OS_TICK)0,												//Ĭ��ʱ��Ƭ����																
					(void  *)0,												//����Ҫ�����û��洢��
					(OS_OPT)OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
					
	//ɾ���������񣬽�������̬
	OSTaskDel(NULL,&err);
}

//led����
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
			//��ȡ����
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			printf("20%02x/%02x/%02x Week:%x ",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);			
					
			//��ȡʱ��
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			printf("%02x:%02x:%02x\r\n",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			g_rtc_wakeup_event = 0;
		}
	}
}








