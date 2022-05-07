#include "sys.h"
#include "usart.h"	
#include "esp8266.h"
#include "esp8266_mqtt.h"
 
#include "includes.h"					//ucos ʹ��	  


//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 

_ttywrch(int ch)
{
	ch = ch;
}
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}

 
//��ʼ��IO ����1 
//baud:������
void usart_init(u32 baud)
{
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	
}

//����2��ʼ��
void usart2_init(uint32_t baud)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3����ΪUSART2
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

}

void usart2_send_str(char *str)
{
	char *p = str;
	
	while(*p!='\0')
	{
		USART_SendData(USART2,*p);
		
		p++;
	
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USART2,USART_FLAG_TXE);
	}
}


void usart2_send_bytes(uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;
	
	while(len--)
	{
		USART_SendData(USART2,*p);
		
		p++;
		
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USART2,USART_FLAG_TXE);
	}
}


void USART1_IRQHandler(void)                	//����1�жϷ������
{
	//uint8_t d=0;

	//�����ж�
	OSIntEnter();    

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		//���մ�������
		//d=USART_ReceiveData(USART1);	
		
		//��մ��ڽ����жϱ�־λ
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 
	
	//�˳��ж�
	OSIntExit();    	
} 


//esp8266ʹ�ô���2
void USART2_IRQHandler(void)                	//����2�жϷ������
{
	uint8_t 	d=0;
	OS_FLAGS  	flags=0;
	OS_ERR 		err;

	//�����ж�
	OSIntEnter();    

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		//���մ�������
		d=USART_ReceiveData(USART2);	
		flags = OSFlagPend(&g_flag_grp,FLAG_GRP_MQTT_CONNECT,0,\
										OS_OPT_PEND_FLAG_SET_ALL +\
										OS_OPT_PEND_NON_BLOCKING, NULL, &err);		//OS_OPT_PEND_NON_BLOCKING �������ȴ�
		if(!(flags & 0x02))												//������� ��־λ����0����ǰ��û�����ӷ�����������ָ������״̬
		{                                
			if(d)                                				//����ָ������״̬ʱ������ֵ�ű��浽������	
				g_esp8266_rx_buf[g_esp8266_rx_cnt++] = d; 		//���浽������	
				
		}
		else
		{		                                        //��֮Connect_flag����1�������Ϸ�������	
			g_esp8266_rx_buf[g_esp8266_rx_cnt++] = d;   //�ѽ��յ������ݱ��浽g_esp8266_rx_buf��			
			
			if(g_esp8266_rx_cnt == 1)   					//���g_esp8266_rx_cnt����1����ʾ�ǽ��յĵ�1�����ݣ�����if��֧				
				TIM_Cmd(TIM3,ENABLE); 
			else                       					//else��֧����ʾ��g_esp8266_rx_cnt������1�����ǽ��յĵ�һ������
				TIM_SetCounter(TIM3,0);  
				
		}
		
		
#if EN_DEBUG_ESP8266		
		//�����յ������ݷ�������1
		USART_SendData(USART1,d);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
#endif	
		
		//��մ��ڽ����жϱ�־λ
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	} 
	
	//�˳��ж�
	OSIntExit();    	

} 

