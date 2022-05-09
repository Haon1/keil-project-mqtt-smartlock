#include <stm32f4xx.h>
#include "sys.h"
#include "esp8266.h"
#include "nettime.h"
#include "rtc.h"
#include "delay.h"
#include "includes.h"

volatile uint8_t g_rtc_alarm_A_event = 0;


static EXTI_InitTypeDef   	EXTI_InitStructure;
static NVIC_InitTypeDef   	NVIC_InitStructure;

static RTC_TimeTypeDef  	RTC_TimeStructure;
static RTC_DateTypeDef  	RTC_DateStructure;

static RTC_InitTypeDef  	RTC_InitStructure;
static RTC_AlarmTypeDef 	RTC_AlarmStructure;


//设置日期 
void rtc_init(struct timeinfo t)
{
	u8 i=0;
	//打开电源管理时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	//允许访问备份寄存器，就是对备份寄存器电路供电
	PWR_BackupAccessCmd(ENABLE);
	
	/* 使能LSE*/
	RCC_LSEConfig(RCC_LSE_ON);			//32.768khz
	/*等待外部晶振输出稳定*/
	for(i=0;i<10;i++)
	{
		if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
			break;
		delay_ms(2);		//延时两毫秒
	}
	if(i==10)		//i等于10,说明LSE不起振 ，改用HSE
	{
		//使用外部高速晶振 20分频  
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div20); 
	}
	else
	{
		/*使用外部32.768KHz晶振作为RTC时钟 */                           
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	}
	
	
	/* Enable the RTC Clock，使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation，等待RTC相关寄存器就绪，因为RTC才供电不久 */
	RTC_WaitForSynchro();
	
	
	//配置分频值  要为1Hz
	/* ck_spre(1Hz) = RTCCLK(LSE) /((uwAsynchPrediv + 1)*(uwSynchPrediv + 1))
					= 32768Hz / ((0x7F+1)*(0xFF+1))
					= 32768Hz /32768
	                =1Hz */
	if(i==10)
	{
		//  8000000/20 = 400000   400000/((0x27+1)*(0x270F+1)) = 1Hz
		RTC_InitStructure.RTC_AsynchPrediv = 0x27;				//异步分频系数
		RTC_InitStructure.RTC_SynchPrediv = 0x270F;				//同步分频系数
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
		RTC_Init(&RTC_InitStructure);
	}
	else
	{
		//32768hz
		/* Configure the RTC data register and RTC prescaler，配置RTC数据寄存器与RTC的分频值 */
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//异步分频系数
		RTC_InitStructure.RTC_SynchPrediv = 0xFF;				//同步分频系数
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24小时格式
		RTC_Init(&RTC_InitStructure);
	}
	

	//设置日期
	RTC_DateStructure.RTC_Year = (t.year+(t.year/10)*6);  //21+(21/10)*6->21+12->33->0x21
	RTC_DateStructure.RTC_Month = (t.mon+(t.mon/10)*6);		//5+(5/10)*6->5+0->5->0x05
	RTC_DateStructure.RTC_Date = (t.day+(t.day/10)*6);
	RTC_DateStructure.RTC_WeekDay = (t.week+(t.week/10)*6);
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	//设置时间
	if(t.hour >= 12)
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	else
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	
	RTC_TimeStructure.RTC_Hours   = (t.hour+(t.hour/10)*6);
	RTC_TimeStructure.RTC_Minutes = (t.min+(t.min/10)*6);
	RTC_TimeStructure.RTC_Seconds = (t.sec+(t.sec/10)*6);
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

	
	//关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);
	
	//为唤醒功能选择RTC配置好的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//设置唤醒计数值为自动重载，写入值默认是0
	RTC_SetWakeUpCounter(1-1);
	
	//清除RTC唤醒中断标志
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//使能RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//使能唤醒功能
	RTC_WakeUpCmd(ENABLE);

	/* Configure EXTI Line22，配置外部中断控制线22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//抢占优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//响应优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
	NVIC_Init(&NVIC_InitStructure);	
}


//从备份寄存器中恢复时间
void rtc_init_from_bkp_dr0(void)
{
	u8 i=0;
	//打开电源管理时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	//允许访问备份寄存器，就是对备份寄存器电路供电
	PWR_BackupAccessCmd(ENABLE);
	
	/* 使能LSE*/
	RCC_LSEConfig(RCC_LSE_ON);			//32.768khz
	/*等待外部晶振输出稳定*/
	for(i=0;i<10;i++)
	{
		if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
			break;
		delay_ms(2);		//延时两毫秒
	}
	if(i==10)		//i等于10,说明LSE不起振 ，改用HSE
	{
		//使用外部高速晶振 20分频  
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div20); 
	}
	else
	{
		/*使用外部32.768KHz晶振作为RTC时钟 */                           
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	}
	
	/* Enable the RTC Clock，使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation，等待RTC相关寄存器就绪，因为RTC才供电不久 */
	RTC_WaitForSynchro();
	
	//关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);
	
	//为唤醒功能选择RTC配置好的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//设置唤醒计数值为自动重载，写入值默认是0
	RTC_SetWakeUpCounter(1-1);
	
	//清除RTC唤醒中断标志
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//使能RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//使能唤醒功能
	RTC_WakeUpCmd(ENABLE);

	/* Configure EXTI Line22，配置外部中断控制线22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//抢占优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//响应优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
	NVIC_Init(&NVIC_InitStructure);	
}


//设置日期 21-5-22-6
void rtc_set_date(uint8_t year,uint8_t mon,uint8_t day,uint8_t week)
{	
	RTC_DateStructure.RTC_Year = (year+(year/10)*6);  //21+(21/10)*6->21+12->33->0x21
	RTC_DateStructure.RTC_Month = (mon+(mon/10)*6);		//5+(5/10)*6->5+0->5->0x05
	RTC_DateStructure.RTC_Date = (day+(day/10)*6);
	RTC_DateStructure.RTC_WeekDay = (week+(week/10)*6);
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
}

//设置时间 10-30-20
void rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
	if(hour >= 12)
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;	
	else
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	
	RTC_TimeStructure.RTC_Hours   = (hour+(hour/10)*6);
	RTC_TimeStructure.RTC_Minutes = (min+(min/10)*6);
	RTC_TimeStructure.RTC_Seconds = (sec+(sec/10)*6);
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
}


//时间戳转成时间
void timestamp_to_realtime(time_t *timestamp, struct timeinfo *t)
{
	struct tm *p = localtime(timestamp);
	
	t->year = p->tm_year-100;
	t->mon  = p->tm_mon+1;
	t->day  = p->tm_mday;
	t->week = p->tm_wday;
	if(t->week==0)	t->week = 7;
	t->hour = p->tm_hour;
	t->min  = p->tm_min;
	t->sec  = p->tm_sec;
}

//同步本地时间
int32_t sync_local_time(void)
{
	struct timeinfo t;		//时间结构体
	time_t T;				//时间戳
	
	
	if( RTC_ReadBackupRegister(RTC_BKP_DR0) != BACKUP_FLAG )	//读取备份寄存器
	{
		printf("sync time from network\r\n");
		if(esp8266_get_network_time())
			return -1;
		
		//解析回复得到时间搓
		T = time_json_parse(strstr((char *)g_esp8266_rx_buf,"{"));
		
		//时间戳转时间放到  结构体中
		timestamp_to_realtime(&T,&t);
		
		//用结构体初始化rtc
		rtc_init(t);
		
		RTC_WriteBackupRegister(RTC_BKP_DR0, BACKUP_FLAG);	//写入备份寄存器
		
		return 1;
	}
	else
	{
		printf("sync time from back dr0\r\n");
		//从备份寄存器恢复时间
		rtc_init_from_bkp_dr0();
		return 2;
	}
}

//设置闹钟 10-30-20
void rtc_set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
	//关闭闹钟A
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
	
	if(hour >= 12)
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12  = RTC_H12_PM;
	else
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12  = RTC_H12_AM;
	
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = (hour+(hour/10)*6);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = (min+(min/10)*6);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = (sec+(sec/10)*6);
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;	//每天生效
	
	//设置闹钟
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	//闹钟A的中断使能
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	
	/* 清空标志位 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	
	//打开闹钟A
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}



//闹钟初始化
void rtc_alarm_init(void)
{
	//关闭闹钟A
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
	
	
	//配置闹钟时间 17:02:30
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_PM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 0x17;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 0x12;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0x05;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;	//每天生效
	
	//设置闹钟
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	//闹钟A的中断使能
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	
	
	/*使能外部中断控制线17的中断*/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/*使能闹钟的中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* 清空标志位 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	
	//打开闹钟A
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}


//rtc唤醒中断
void RTC_WKUP_IRQHandler(void)
{
	OS_ERR err;

	//进入中断
	OSIntEnter();
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
	{
		OSFlagPost(&g_flag_grp,FLAG_GRP_RTC_WAKEUP,OS_OPT_POST_FLAG_SET,&err);
		
		//清空RTC的唤醒中断标志位
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		//清空外部中断22的标志位
		EXTI_ClearITPendingBit(EXTI_Line22);
	} 

	//退出中断
	OSIntExit();

}

void RTC_Alarm_IRQHandler(void)
{
	OS_ERR err;

	//进入中断
	OSIntEnter();
	
	
	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
	{
		//PFout(9) = 0;

		g_rtc_alarm_A_event = 1;
		
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);

	}

	//退出中断
	OSIntExit();
}

