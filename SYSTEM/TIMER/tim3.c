#include <stm32f4xx.h>
#include "sys.h"
#include "esp8266.h"


static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static NVIC_InitTypeDef 		NVIC_InitStructure;

void tim3_init(void)
{
	//时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = 299;		//计数值 300次计数是30ms, 300-1
	TIM_TimeBaseStructure.TIM_Prescaler = 8399;		//分频值 84000000/(8399+1)=10000Hz,10000次计数是1S
	//TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//再分频,F407不支持
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能tim3	时间更新中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);		//使能TIM3工作	
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update)==SET)
	{
		g_esp8266_rx_end = 1;		//接收完成
		
		TIM_Cmd(TIM3, DISABLE);                        				  //关闭TIM4定时器
		TIM_SetCounter(TIM3, 0);    
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}

