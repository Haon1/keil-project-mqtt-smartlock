#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 


void usart_init(uint32_t baud);
//´®¿Ú2³õÊ¼»¯
void usart2_init(uint32_t baud);

void usart2_send_str(char *str);

void usart2_send_bytes(uint8_t *buf,uint32_t len);

#endif


