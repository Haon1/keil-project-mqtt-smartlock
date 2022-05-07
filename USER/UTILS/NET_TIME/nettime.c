#include <stdio.h>
#include <stdlib.h>
#include "cjson.h"
#include "nettime.h"

//��ʱ��pai�ظ�����������ȡʱ���
time_t time_json_parse(char *pbuf)
{
	cJSON *json , *json_data, *json_timestamp;
	char buf[11]={0};
	char *p = pbuf;
	time_t T;
		
	//�������ݰ�
	json = cJSON_Parse(p);			
	if (!json)  
	{  
		cJSON_Delete(json);
		json=NULL;		
		return;
	} 		
	
	
	//����data����ȡֵ
	json_data = cJSON_GetObjectItem(json , "data"); 
	if(json_data)
	{
		//����"t"����ȡֵ
		json_timestamp = cJSON_GetObjectItem(json_data , "t"); 
		printf("%s\n",json_timestamp->valuestring);
		strncpy(buf,json_timestamp->valuestring,10);
		printf("buf = %s\n",buf);
		
		T = atoi(buf);		//ͬ�����룬��ubuntu���������Сʱ
		T += 8*60*60;		//���ϰ�Сʱ������
	}
	cJSON_Delete(json);
	json=NULL;	
	
	return T;
}

