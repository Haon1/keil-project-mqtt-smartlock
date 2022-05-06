#ifndef __LED_H__
#define __LED_H__



#define	BLACK	0x00	//黑
#define RED 	0x01	//红
#define YELLOW 	0x02	//黄
#define BLUE 	0x03	//蓝
#define CYAN	0x04	//浅蓝
#define GREEN	0x05	//绿
#define PURPLE 	0x06	//紫
#define WHITE 	0x07	//白



void rgb_led_init(void);
void rgb_led_set_color(int color);


#endif
