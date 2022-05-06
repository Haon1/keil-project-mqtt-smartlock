#include <stm32f4xx.h>
#include "sys.h"
#include "rgb_led.h"


#define r_pin 2
#define g_pin 4
#define b_pin 3


static GPIO_InitTypeDef  GPIO_InitStructure;

void rgb_led_init(void)
{
	/* GPIOG Peripheral clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	
	PEout(r_pin)=1;		//熄灭
	PEout(g_pin)=1;		
	PEout(b_pin)=1;		
}


void rgb_led_set_color(int color)
{
	if(color == BLACK)		//灭
	{
		PEout(r_pin)=1;		
		PEout(g_pin)=1;
		PEout(b_pin)=1;
	}
	if(color == RED)	//红光
	{
		PEout(r_pin)=0;		
		PEout(g_pin)=1;
		PEout(b_pin)=1;
	}
	else if(color == YELLOW)	//黄色
	{
		PEout(r_pin)=0;		
		PEout(g_pin)=0;
		PEout(b_pin)=1;
	}
	else if(color == BLUE)	//蓝色
	{
		PEout(r_pin)=1;		
		PEout(g_pin)=1;
		PEout(b_pin)=0;
	}
	else if(color == CYAN)	//浅蓝色
	{
		PEout(r_pin)=1;		
		PEout(g_pin)=0;
		PEout(b_pin)=0;
	}
	else if(color == GREEN)	//绿色
	{
		PEout(r_pin)=1;		
		PEout(g_pin)=0;
		PEout(b_pin)=1;
	}
	else if(color == PURPLE)	//紫色
	{
		PEout(r_pin)=0;		
		PEout(g_pin)=1;
		PEout(b_pin)=0;
	}
	else if(color == WHITE)		//白色
	{
		PEout(r_pin)=0;		
		PEout(g_pin)=0;
		PEout(b_pin)=0;
	}
}
