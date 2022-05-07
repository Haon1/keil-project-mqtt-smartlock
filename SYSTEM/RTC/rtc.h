#ifndef _RTC_
#define _RTC_

extern volatile uint8_t g_rtc_alarm_A_event;

// 22 5 6 5 12 33 30   �� �� �� ���� ʱ �� ��
void rtc_init(uint8_t year,uint8_t mon,uint8_t day,uint8_t week, uint8_t hour, uint8_t min, uint8_t sec);
//�ӱ��ݼĴ����лָ�ʱ��
void rtc_init_from_bkp_dr0(void);
void rtc_alarm_init(void);

//��������
void rtc_set_date(uint8_t year,uint8_t mon,uint8_t day,uint8_t week);
//����ʱ�� 10-30-20
void rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec);
//�������� 10-30-20
void rtc_set_alarm(uint8_t hour, uint8_t min, uint8_t sec);


#endif
