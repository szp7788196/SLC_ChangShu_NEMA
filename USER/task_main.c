#include "task_main.h"
#include "common.h"
#include "delay.h"
#include "usart.h"
#include "inventr.h"
#include "pwm.h"
#include "event.h"


TaskHandle_t xHandleTaskMAIN = NULL;

u8 MirrorLightLevelPercent = 0;
u8 MirrorPowerInterface = 0xFF;
unsigned portBASE_TYPE MAIN_Satck;

void vTaskMAIN(void *pvParameters)
{
	time_t times_sec = 0;
	time_t time_cnt = 0;
	u8 get_e_para_ok = 0;

	GetTimeOK = GetSysTimeState();

	SetLightLevel(PowerInterface, DefaultLightLevelPercent);

	while(1)
	{
		if(GetSysTick1s() - times_sec >= 1)
		{
			times_sec = GetSysTick1s();

			if(GetTimeOK != 0)							//ϵͳʱ��״̬
			{
				if(DeviceWorkMode == MODE_AUTO)
				{
					CheckSwitchStatus(&SwitchState);		//��ѯ��ǰ����Ӧ�ô��ڵ�״̬

					if(SwitchState == 1)			//ֻ���ڿ���Ϊ����״̬ʱ����ѯ����
					{
						AutoLoopRegularTimeGroups(&LightLevelPercent);	//��ѵ�����б�
					}
				}
			}
		}

		if(MirrorLightLevelPercent != LightLevelPercent || \
			MirrorPowerInterface != PowerInterface)
		{
			MirrorLightLevelPercent = LightLevelPercent;
			MirrorPowerInterface = PowerInterface;

			SetLightLevel(PowerInterface, LightLevelPercent);

			time_cnt = GetSysTick1s();		//��λ��ȡ��ǰ״̬�������ʱ��
			get_e_para_ok = 0;				//��λ��ȡ��ǰ�������־ 0:δ��ȡ 1:�Ի�ȡ
		}
		else
		{
			if(GetSysTick1s() - time_cnt >= EventDetectConf.current_detect_delay * 60 / 2)			//�ȵ����������ʱ��1/2ʱ�ɼ������
			{
				if(get_e_para_ok == 0)		//δ��ȡ��ǰ�����
				{
					get_e_para_ok = 1;		//�Ի�ȡ��ǰ�����

					FaultInputCurrent = InputCurrent;		//��ȡ��ǰ����ֵ
					FaultInputVoltage = InputVoltage;		//��ȡ��ǰ��ѹֵ
				}
			}
		}

//		CheckEventsEC15(LightLevelPercent);	//�����������Ƽ�¼
//		CheckEventsEC16(LightLevelPercent);	//���������صƼ�¼
//		CheckEventsEC17(LightLevelPercent);	//�����쳣���Ƽ�¼
//		CheckEventsEC18(LightLevelPercent);	//�����쳣�صƼ�¼
//		CheckEventsEC19(LightLevelPercent);	//���Ƶ��������¼
//		CheckEventsEC20(LightLevelPercent);	//���Ƶ�����С��¼

		if(ResetFlag == 1)								//���յ�����������
		{
			ResetFlag = 0;
			delay_ms(5000);

			__disable_fault_irq();							//����ָ��
			NVIC_SystemReset();
		}

		if(FrameWareState.state == FIRMWARE_DOWNLOADED)		//�̼��������,���������³���
		{
			delay_ms(1000);									//��ʱ1��,�ȴ��̼�״̬����EEPROM

			__disable_fault_irq();							//�ر�ȫ���ж�
			NVIC_SystemReset();								//����ָ��
		}
		else if(FrameWareState.state == FIRMWARE_DOWNLOAD_FAILED)
		{
			FrameWareState.state = FIRMWARE_FREE;			//��ʱ�����й̼�����,�ȵ��´��ϵ��ʱ��������
		}

		delay_ms(100);
//		MAIN_Satck = uxTaskGetStackHighWaterMark(NULL);
	}
}

void CheckSwitchStatus(u8 *switch_mode)
{
	u16 i = 0;
	s16 sum = 0;
	u8 month = 0;
	u8 date = 0;
	u8 month_c = 0;
	u8 date_c = 0;
	u8 on_hour = 0;
	u8 on_minute = 0;
	u8 off_hour = 0;
	u8 off_minute = 0;

	u16 on_gate = 0;
	u16 off_gate = 0;
	u16 now_gate = 0;

	if(SwitchMode != 1)							//��������
	{
		*switch_mode = 1;						//Ĭ�Ͽ���
	}
	else										//������
	{
		if(LampsSwitchProject.total_days >= 1 &&
		   LampsSwitchProject.total_days <= 366)	//���Ϊ��
		{
			month = LampsSwitchProject.start_month;
			date = LampsSwitchProject.start_date;

			if(month >= 1 && month <= 12 && date >= 1 && date <= 31)
			{
				for(i = 0; i < LampsSwitchProject.total_days; i ++)
				{
					sum = get_day_num(month,date) + i;

					if(sum <= 366)
					{
						get_date_from_days(sum, &month_c, &date_c);

						if(month_c >= 1 && month_c <= 12 && date_c >= 1 && date_c <= 31)		//�õ��Ϸ�����
						{
							if(month_c == calendar.w_month && date_c == calendar.w_date)
							{
								on_hour = LampsSwitchProject.switch_time[i].on_time[0];
								on_minute = LampsSwitchProject.switch_time[i].on_time[1];
								off_hour = LampsSwitchProject.switch_time[i].off_time[0];
								off_minute = LampsSwitchProject.switch_time[i].off_time[1];

								if(on_hour <= 23 && on_minute <= 59 && off_hour <= 23 && off_minute <= 59)		//�õ��Ϸ�ʱ��
								{
									on_gate = on_hour * 60 + on_minute;
									off_gate = off_hour * 60 + off_minute;
									now_gate = calendar.hour * 60 + calendar.min;

									if(on_gate != off_gate)
									{
										if(on_gate < off_gate)		//�ȿ��ƺ�ص�
										{
											if(on_gate <= now_gate && now_gate < off_gate)	//��ǰʱ����ڵ��ڿ���ʱ��,С�ڹص�ʱ��,�򿪵�
											{
												*switch_mode = 1;		//����
											}
											else	//��ǰʱ��С�ڿ���ʱ��,���ߴ��ڵ��ڹص�ʱ��,��ص�
											{
												*switch_mode = 0;		//�ص�
											}
										}
										else						//�ȹصƺ󿪵�
										{
											if(off_gate <= now_gate && now_gate < on_gate)	//��ǰʱ����ڵ��ڹص�ʱ��,С�ڿ���ʱ��,��ص�
											{
												*switch_mode = 0;		//�ص�
											}
											else	//��ǰʱ��С�ڹص�ʱ��,���ߴ��ڵ��ڿ���ʱ��,�򿪵�
											{
												*switch_mode = 1;		//����
											}
										}
									}
									else					//���ص�ʱ����ͬ
									{
										*switch_mode = 1;		//Ĭ������
									}
								}
								else	//�õ��Ƿ�ʱ��
								{
									*switch_mode = 1;		//Ĭ������
								}

								i = 0xFF00;		//����366����
							}
						}
						else	//�õ��Ƿ�����
						{
							*switch_mode = 1;	//Ĭ������
						}
					}
					else		//���ڳ�����Χ
					{
						*switch_mode = 1;		//Ĭ������
					}
				}
			}
			else								//����ͷ����
			{
				*switch_mode = 1;				//Ĭ������
			}
		}
		else									//���Ϊ��
		{
			*switch_mode = 1;					//Ĭ������
		}
	}
}

//��ѯʱ�����
void AutoLoopRegularTimeGroups(u8 *percent)
{
	u8 ret = 0;
	u16 gate0 = 0;
	u16 gate1 = 0;
	u16 gate2 = 0;
	u16 gate24 = 1440;	//24*60;
	u16 gate_n = 0;
	u32 gate_day_s = 0;
	u32 gate_day_e = 0;
	u32 gate_day_n = 0;

	static u8 last_percent = 100;
	static u8 current_percent = 100;

	pRegularTime tmp_time = NULL;

	xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);

	if(RegularTimeHoliday->next != NULL)
	{
		ret = 0;

		for(tmp_time = RegularTimeHoliday->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
//			if(tmp_time->range.year_s <= calendar.w_year - 2000 &&
//			   calendar.w_year - 2000 <= tmp_time->range.year_e)		//��ǰ�괦����ʼ�ͽ�����֮��
//			{
//				if(tmp_time->range.month_s <= calendar.w_month &&
//				   calendar.w_month <= tmp_time->range.month_e)
//				{
//					if(tmp_time->range.date_s <= calendar.w_date &&
//					   calendar.w_date <= tmp_time->range.date_e)
//					{
						gate_day_s = get_days_form_calendar(tmp_time->range.year_s + 2000,tmp_time->range.month_s,tmp_time->range.date_s);

						gate_day_e = get_days_form_calendar(tmp_time->range.year_e + 2000,tmp_time->range.month_e,tmp_time->range.date_e);

						gate_day_n = get_days_form_calendar(calendar.w_year,calendar.w_month,calendar.w_date);

						if(gate_day_s <= gate_day_n && gate_day_n <= gate_day_e)
						{
							if(tmp_time->hour 	== calendar.hour &&
							   tmp_time->minute == calendar.min)		//�жϵ�ǰʱ���Ƿ�ͬ��������ʱ����ͬ
							{
								ret = 1;
							}
							else if(tmp_time->next != NULL)
							{
								if(tmp_time->next->hour   == calendar.hour &&
								   tmp_time->next->minute == calendar.min)
								{
									tmp_time = tmp_time->next;

									ret = 1;
								}
								else
								{
									gate1 = tmp_time->hour * 60 + tmp_time->minute;
									gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;
									gate_n = calendar.hour * 60 + calendar.min;

									if(gate1 < gate2)
									{
										if(gate1 <= gate_n && gate_n <= gate2)
										{
											ret = 1;
										}
									}
									else if(gate1 > gate2)
									{
										if(gate1 <= gate_n && gate_n <= gate24)
										{
											ret = 1;
										}
										else if(gate0 <= gate_n && gate_n <= gate2)
										{
											ret = 1;
										}
									}
								}
							}
							else
							{
								ret = 1;
							}
						}
//					}
//				}
//			}

			if(ret == 1)
			{
				current_percent = tmp_time->percent;

				goto UPDATE_PERCENT;
			}
		}
	}

	if(calendar.week <= 6)				//�ж��Ƿ��ǹ�����
	{
		if(RegularTimeWeekDay->next != NULL)		//�жϲ����б��Ƿ�Ϊ��
		{
			ret = 0;

			for(tmp_time = RegularTimeWeekDay->next; tmp_time != NULL; tmp_time = tmp_time->next)	//��ѵ�����б�
			{
				if(tmp_time->hour 	== calendar.hour &&
				   tmp_time->minute == calendar.min)		//�жϵ�ǰʱ���Ƿ�ͬ��������ʱ����ͬ
				{
					ret = 1;
				}
				else if(tmp_time->next != NULL)				//���������ǲ������һ��
				{
					if(tmp_time->next->hour   == calendar.hour &&
					   tmp_time->next->minute == calendar.min)		//�жϸ������Ե�next��ʱ���Ƿ��뵱ǰʱ����ͬ
					{
						tmp_time = tmp_time->next;

						ret = 1;
					}
					else
					{
						gate1 = tmp_time->hour * 60 + tmp_time->minute;					//�������Եķ�����
						gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;		//�������Ե�next�ķ�����
						gate_n = calendar.hour * 60 + calendar.min;						//��ǰʱ��ķ�����

						if(gate1 < gate2)												//��������ʱ������next��ʱ��
						{
							if(gate1 <= gate_n && gate_n <= gate2)						//�жϵ�ǰʱ���Ƿ�����������ʱ����м�
							{
								ret = 1;
							}
						}
						else if(gate1 > gate2)											//��������ʱ������next��ʱ��
						{
							if(gate1 <= gate_n && gate_n <= gate24)						//�жϵ�ǰʱ���Ƿ��ڸ�������ʱ���24��ʱ����м�
							{
								ret = 1;
							}
							else if(gate0 <= gate_n && gate_n <= gate2)					//�жϵ�ǰʱ���Ƿ���0���next��ʱ����м�
							{
								ret = 1;
							}
						}
					}
				}
				else
				{
					ret = 1;
				}

				if(ret == 1)
				{
					current_percent = tmp_time->percent;

					goto UPDATE_PERCENT;
				}
			}
		}
	}

	UPDATE_PERCENT:
	if(last_percent != current_percent)
	{
		last_percent = current_percent;

		*percent = current_percent;
	}

	xSemaphoreGive(xMutex_STRATEGY);
}



































