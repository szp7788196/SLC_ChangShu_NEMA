#include "event.h"
#include "24cxx.h"
#include "task_sensor.h"

//��¼���洢�������¼�
void RecordEventsECx(u8 ecx,u8 len,u8 *msg)
{
	u16 crc_cal = 0;
	u16 add_pos = 0;
	u8 buf[24];
	
	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_EVENT_RECORD, portMAX_DELAY);
	}

	add_pos = E_IMPORTEAT_ADD;

	memset(buf,0,24);

	buf[0] = ecx;						//�¼�����
	buf[1] = len + 6;					//�¼�����

	memcpy(buf + 2,CalendarClock,6);	//����ʱ��
	memcpy(buf + 8,msg,len);			//�����¼�����

	crc_cal = GetCRC16(buf,22);			//����У��

	buf[22] = (u8)(crc_cal >> 8);
	buf[23] = (u8)(crc_cal & 0x00FF);

	EventRecordList.lable1[EventRecordList.ec1] = ecx;	//�����¼��б�

	crc_cal = GetCRC16(EventRecordList.lable1,128);

	AT24CXX_WriteOneByte(EC1_LABLE_ADD + EventRecordList.ec1,EventRecordList.lable1[EventRecordList.ec1]);
	AT24CXX_WriteOneByte(EC1_LABLE_ADD + 128,(u8)(crc_cal >> 8));
	AT24CXX_WriteOneByte(EC1_LABLE_ADD + 129,(u8)(crc_cal & 0x00FF));

	WriteDataFromMemoryToEeprom(buf,add_pos + EventRecordList.ec1 * EVENT_LEN, EVENT_LEN);

	EventRecordList.ec1 ++;				//�����¼�������

	WriteDataFromMemoryToEeprom(&EventRecordList.ec1,EC1_ADD, 1);
	
	EventRecordList.important_event_flag = 1;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_EVENT_RECORD);
	}
}

//������������¼�
void CheckEventsEC15(u8 light_level)
{
	static u8 mirror_level = 0;
	static time_t cnt = 0;
	static u8 state_change = 0;
	static u8 first = 1;
	u8 buf[6];
	
	if(first == 1)
	{
		first = 0;
		
		mirror_level = light_level;
	}

	if(mirror_level != light_level)		//����״̬�б仯
	{
		if(state_change == 0)
		{
			cnt = GetSysTick1s();

			state_change = 1;
		}

		if(GetSysTick1s() - cnt >= EventDetectConf.turn_on_collect_delay * 60)	//�ȴ�һ��ʱ�䣬��������͵�ѹ
		{
			cnt = 0;
			state_change = 0;

			if(mirror_level == 0)	//�ϸ�״̬Ϊ�ص�
			{
				if(light_level >= 1)		//����״̬Ϊ����
				{
					memset(buf,0,6);

					if(InputCurrent <= SWITCH_ON_MIN_CURRENT)			//�����쳣
					{
						buf[0] = 0x00;		//���Ƴɹ���
						buf[1] = 0x00;
						buf[2] = 0x01;		//����ʧ����
						buf[3] = 0x00;
						buf[4] = 0x00;		//ʧ�ܵƺ�
						buf[5] = 0x01;
					}
					else												//��������
					{
						buf[0] = 0x01;		//���Ƴɹ���
						buf[1] = 0x00;
						buf[2] = 0x00;		//����ʧ����
						buf[3] = 0x00;
						buf[4] = 0x00;		//ʧ�ܵƺ�
						buf[5] = 0x00;
					}

					RecordEventsECx(EVENT_ERC15,6,buf);
					
				}
			}
			
			mirror_level = light_level;
		}
	}
	else
	{
		cnt = 0;
		state_change = 0;
	}
}

//����쳣�����¼�
void CheckEventsEC16(u8 light_level)
{
	static u8 mirror_level = 0;
	static time_t cnt = 0;
	static u8 state_change = 0;
	static u8 first = 1;
	u8 buf[6];
	
	if(first == 1)
	{
		first = 0;
		
		mirror_level = light_level;
	}

	if(mirror_level != light_level)		//����״̬�б仯
	{
		if(state_change == 0)
		{
			cnt = GetSysTick1s();

			state_change = 1;
		}

		if(GetSysTick1s() - cnt >= EventDetectConf.turn_off_collect_delay * 60)	//�ȴ�һ��ʱ�䣬��������͵�ѹ
		{
			cnt = 0;
			state_change = 0;

			if(mirror_level >= 1)	//�ϸ�״̬Ϊ����
			{
				if(light_level == 0)		//����״̬Ϊ�ص�
				{
					memset(buf,0,6);

					if(InputCurrent > SWITCH_OFF_MAX_CURRENT)			//�ص��쳣
					{
						buf[0] = 0x00;		//���Ƴɹ���
						buf[1] = 0x00;
						buf[2] = 0x01;		//����ʧ����
						buf[3] = 0x00;
						buf[4] = 0x00;		//ʧ�ܵƺ�
						buf[5] = 0x01;
					}
					else												//�ص�����
					{
						buf[0] = 0x01;		//���Ƴɹ���
						buf[1] = 0x00;
						buf[2] = 0x00;		//����ʧ����
						buf[3] = 0x00;
						buf[4] = 0x00;		//ʧ�ܵƺ�
						buf[5] = 0x00;
					}

					RecordEventsECx(EVENT_ERC16,6,buf);
				}
			}

			mirror_level = light_level;
		}
	}
	else
	{
		cnt = 0;
		state_change = 0;
	}
}

//�����쳣����
void CheckEventsEC17(u8 light_level)
{
	static s8 cnt = 0;
	static u8 occur = 0;
	u8 buf[4];

	if(light_level == 0)			//�ϸ�״̬Ϊ�ص�
	{
		if(InputCurrent > SWITCH_OFF_MAX_CURRENT)				//�쳣����
		{
			if(occur == 0)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
		}
		else
		{
			if(occur == 1)
			{
				if(cnt > -100)
				{
					cnt --;
				}
			}
		}
	}
	else
	{
		cnt = 0;
		occur = 0;
	}

	if(cnt >= 100)
	{
		cnt = 0;
		occur = 1;

		memset(buf,0,4);

		buf[0] = 0x01;														//��¼����
		buf[1] = 0x00;														//�ƾ����
		buf[2] = 0x01;
		buf[3] = 0x00;														//�¼���ʼ

		RecordEventsECx(EVENT_ERC17,4,buf);
	}
	else if(cnt <= -100)
	{
		cnt = 0;
		occur = 0;

		memset(buf,0,4);

		buf[0] = 0x00;														//��¼����
		buf[1] = 0x00;														//�ƾ����
		buf[2] = 0x01;
		buf[3] = 0x00;														//�¼�����

		RecordEventsECx(EVENT_ERC17,4,buf);
	}
}

//�����쳣�ص�
void CheckEventsEC18(u8 light_level)
{
	static s8 cnt = 0;
	static u8 occur = 0;
	u8 buf[4];

	if(light_level >= 1)			//����״̬Ϊ����
	{
		if(InputCurrent <= SWITCH_ON_MIN_CURRENT)		//�쳣����
		{
			if(occur == 0)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
		}
		else
		{
			if(occur == 1)
			{
				if(cnt > -100)
				{
					cnt --;
				}
			}

		}
	}
	else
	{
		cnt = 0;
		occur = 0;
	}

	if(cnt >= 100)
	{
		cnt = 0;
		occur = 1;

		memset(buf,0,4);

		buf[0] = 0x01;
		buf[1] = 0x00;					//�ƾ����
		buf[2] = 0x01;
		buf[3] = 0x00;

		RecordEventsECx(EVENT_ERC18,4,buf);
	}
	else if(cnt <= -100)
	{
		cnt = 0;
		occur = 0;

		memset(buf,0,4);

		buf[0] = 0x00;
		buf[1] = 0x00;					//�ƾ����
		buf[2] = 0x01;
		buf[3] = 0x00;

		RecordEventsECx(EVENT_ERC18,4,buf);
	}
}

//��ⵥ�Ƶ��������¼
void CheckEventsEC19(u8 light_level)
{
	static u8 mirror_level = 0;
	static time_t time_cnt = 0;
	static u8 occur = 0;
	static s8 cnt = 0;
	static u8 first = 1;
	u8 buf[7];
	
	if(first == 1)
	{
		first = 0;
		
		mirror_level = light_level;
	}

	if(mirror_level != light_level)		//����״̬�б仯
	{
		time_cnt = GetSysTick1s();
		occur = 0;
		cnt = 0;

		mirror_level = light_level;
	}

	if(GetSysTick1s() - time_cnt >= EventDetectConf.current_detect_delay * 60)	//�ȴ�һ��ʱ�䣬��������͵�ѹ
	{
		if(occur == 0)
		{
			if((InputCurrent - FaultInputCurrent) / FaultInputCurrent >=
			  (float)EventDetectConf.over_current_ratio / 100.0f)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
			else
			{
				if(cnt > 0)
				{
					cnt --;
				}
			}

			if(cnt >= 100)
			{
				cnt = 0;
				occur = 1;

				buf[0] = 0x01;														//��¼����
				buf[1] = 0x00;														//�ƾ����
				buf[2] = 0x01;
				buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//����ʱ����
				buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
				buf[5] = light_level;												//����ʱ����״̬

				RecordEventsECx(EVENT_ERC19,6,buf);
			}
		}
		else if(occur == 1)
		{
			if((InputCurrent - FaultInputCurrent) / FaultInputCurrent <=
			  (float)EventDetectConf.over_current_recovery_ratio / 100.0f)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
			else
			{
				if(cnt > 0)
				{
					cnt --;
				}
			}

			if(cnt >= 100)
			{
				cnt = 0;
				occur = 0;

				buf[0] = 0x00;
				buf[1] = 0x00;		//�ƾ����
				buf[2] = 0x01;
				buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//����ʱ����
				buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
				buf[5] = light_level;

				RecordEventsECx(EVENT_ERC19,6,buf);
			}
		}
	}
}

//��ⵥ�Ƶ�����С��¼
void CheckEventsEC20(u8 light_level)
{
	static u8 mirror_level = 0;
	static time_t time_cnt = 0;
	static u8 occur = 0;
	static s8 cnt = 0;
	static u8 first = 1;
	u8 buf[7];
	
	if(first == 1)
	{
		first = 0;
		
		mirror_level = light_level;
	}

	if(mirror_level != light_level)
	{
		time_cnt = GetSysTick1s();
		occur = 0;
		cnt = 0;

		mirror_level = light_level;
	}

	if(GetSysTick1s() - time_cnt >= EventDetectConf.current_detect_delay * 60)	//�ȴ�һ��ʱ�䣬��������͵�ѹ
	{
		if(occur == 0)
		{
			if((FaultInputCurrent - InputCurrent) / FaultInputCurrent >=
			  (float)EventDetectConf.low_current_ratio / 100.0f)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
			else
			{
				if(cnt > 0)
				{
					cnt --;
				}
			}

			if(cnt >= 100)
			{
				cnt = 0;
				occur = 1;

				buf[0] = 0x01;														//��¼����
				buf[1] = 0x00;														//�ƾ����
				buf[2] = 0x01;
				buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//����ʱ����
				buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
				buf[5] = light_level;	

				RecordEventsECx(EVENT_ERC20,6,buf);
			}
		}
		else if(occur == 1)
		{
			if((FaultInputCurrent - InputCurrent) / FaultInputCurrent <=
			  (float)EventDetectConf.low_current_recovery_ratio / 100.0f)
			{
				if(cnt < 100)
				{
					cnt ++;
				}
			}
			else
			{
				if(cnt > 0)
				{
					cnt --;
				}
			}

			if(cnt >= 100)
			{
				cnt = 0;
				occur = 0;

				buf[0] = 0x00;														//��¼����
				buf[1] = 0x00;														//�ƾ����
				buf[2] = 0x01;
				buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//����ʱ����
				buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
				buf[5] = light_level;	

				RecordEventsECx(EVENT_ERC20,6,buf);
			}
		}
	}
}

//Уʱ����¼���¼
void CheckEventsEC28(u8 *cal1,u8 *cal2)
{
	u8 buf[13];

	memset(buf,0,13);

	buf[0] = 0x00;

	memcpy(&buf[1],cal1,6);		//Уʱǰʱ��
	memcpy(&buf[7],cal2,6);		//Уʱ��ʱ��

	RecordEventsECx(EVENT_ERC28,13,buf);
}

//��������¼���¼
void CheckEventsEC51(u8 result,u16 version)
{
	u8 buf[3];

	memset(buf,0,9);

	buf[0] = result;			//��¼���� �ɹ���ʧ��
	buf[1] = (u8)((version >> 8) & 0x00FF);
	buf[2] = (u8)(version & 0x00FF);

	RecordEventsECx(EVENT_ERC51,3,buf);
}
















































