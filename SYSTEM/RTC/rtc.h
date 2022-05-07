#ifndef _RTC_
#define _RTC_

#include <time.h>

struct timeinfo		//时间结构体
{
	int year;
	int mon;
	int day;
	int week;
	int hour;
	int min;
	int sec;
};

extern volatile uint8_t g_rtc_alarm_A_event;

// 22 5 6 5 12 33 30   年 月 日 星期 时 分 秒
void rtc_init(struct timeinfo t);
//从备份寄存器中恢复时间
void rtc_init_from_bkp_dr0(void);

//时间同步
int32_t sync_local_time(void);

//时间戳转成时间
void timestamp_to_realtime(time_t *timestamp, struct timeinfo *t);

void rtc_alarm_init(void);

//设置日期
void rtc_set_date(uint8_t year,uint8_t mon,uint8_t day,uint8_t week);
//设置时间 10-30-20
void rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec);
//设置闹钟 10-30-20
void rtc_set_alarm(uint8_t hour, uint8_t min, uint8_t sec);


#endif
