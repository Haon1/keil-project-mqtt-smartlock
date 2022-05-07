#include <stm32f4xx.h>
#include "sys.h"
#include "includes.h"

volatile uint8_t g_rtc_alarm_A_event = 0;


static EXTI_InitTypeDef   	EXTI_InitStructure;
static NVIC_InitTypeDef   	NVIC_InitStructure;

static RTC_TimeTypeDef  	RTC_TimeStructure;
static RTC_DateTypeDef  	RTC_DateStructure;

static RTC_InitTypeDef  	RTC_InitStructure;
static RTC_AlarmTypeDef 	RTC_AlarmStructure;

//设置日期 22 5 6 5 
void rtc_init(uint8_t year,uint8_t mon,uint8_t day,uint8_t week, uint8_t hour, uint8_t min, uint8_t sec)
{
	//打开电源管理时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	//允许访问备份寄存器，就是对备份寄存器电路供电
	PWR_BackupAccessCmd(ENABLE);
	
	
#if 0
	/* 使能LSE*/
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* 检查该LSE是否有效*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

	/* 选择LSE作为RTC的硬件时钟源*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	

#else  //若LSE无法工作，可用内部LSI
	/* 使能LSI*/
	RCC_LSICmd(ENABLE);
	
	/* 检查该LSI是否有效*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	/* 选择LSI作为RTC的硬件时钟源*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	

#endif
	
	
	/* Enable the RTC Clock，使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation，等待RTC相关寄存器就绪，因为RTC才供电不久 */
	RTC_WaitForSynchro();
	

	//配置分频值
	/* ck_spre(1Hz) = RTCCLK(LSE) /((uwAsynchPrediv + 1)*(uwSynchPrediv + 1))
					= 32768Hz / ((0x7F+1)*(0xFF+1))
					= 32768Hz /32768
	                =1Hz
	*/
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;//24小时格式
	RTC_Init(&RTC_InitStructure);
	
	
	/* Set the date: Friday May 21th 2021 */
//	RTC_DateStructure.RTC_Year = 0x22;
//	RTC_DateStructure.RTC_Month = RTC_Month_May;
//	RTC_DateStructure.RTC_Date = 0x06;
//	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Friday;
//	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	RTC_DateStructure.RTC_Year = (year+(year/10)*6);  //21+(21/10)*6->21+12->33->0x21
	RTC_DateStructure.RTC_Month = (mon+(mon/10)*6);		//5+(5/10)*6->5+0->5->0x05
	RTC_DateStructure.RTC_Date = (day+(day/10)*6);
	RTC_DateStructure.RTC_WeekDay = (week+(week/10)*6);
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	/* Set the time to 17h 12mn 00s PM */
//	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
//	RTC_TimeStructure.RTC_Hours   = 0x00;
//	RTC_TimeStructure.RTC_Minutes = 0x02;
//	RTC_TimeStructure.RTC_Seconds = 0x00;
//	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
	if(hour >= 12)
	{
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	}
	else
	{
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	}
	
	RTC_TimeStructure.RTC_Hours   = (hour+(hour/10)*6);
	RTC_TimeStructure.RTC_Minutes = (min+(min/10)*6);
	RTC_TimeStructure.RTC_Seconds = (sec+(sec/10)*6);
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
	
	
	//RTC_WriteBackupRegister(RTC_BKP_DR0, 0x80);	//写入备份寄存器
}


//从备份寄存器中恢复时间
void rtc_init_from_bkp_dr0(void)
{
	//打开电源管理时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	//允许访问备份寄存器，就是对备份寄存器电路供电
	PWR_BackupAccessCmd(ENABLE);
	/* 使能LSI*/
	RCC_LSICmd(ENABLE);
	
	/* 检查该LSI是否有效*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	/* 选择LSI作为RTC的硬件时钟源*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);	
	
	
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
	{
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	}
	else
	{
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	}
	
	RTC_TimeStructure.RTC_Hours   = (hour+(hour/10)*6);
	RTC_TimeStructure.RTC_Minutes = (min+(min/10)*6);
	RTC_TimeStructure.RTC_Seconds = (sec+(sec/10)*6);
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
}

//设置闹钟 10-30-20
void rtc_set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
	//关闭闹钟A
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);
	
	if(hour >= 12)
	{
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12  = RTC_H12_PM;
	}
	else
	{
		RTC_AlarmStructure.RTC_AlarmTime.RTC_H12  = RTC_H12_AM;
	}
	
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

