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

			if(GetTimeOK != 0)							//系统时间状态
			{
				if(DeviceWorkMode == MODE_AUTO)
				{
					CheckSwitchStatus(&SwitchState);		//查询当前开关应该处于的状态

					if(SwitchState == 1)			//只有在开关为开的状态时才轮询策略
					{
						AutoLoopRegularTimeGroups(&LightLevelPercent);	//轮训策略列表
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

			time_cnt = GetSysTick1s();		//复位获取当前状态电参数计时器
			get_e_para_ok = 0;				//复位获取当前电参数标志 0:未获取 1:以获取
		}
		else
		{
			if(GetSysTick1s() - time_cnt >= EventDetectConf.current_detect_delay * 60 / 2)			//等到电流检测延时的1/2时采集电参数
			{
				if(get_e_para_ok == 0)		//未获取当前电参数
				{
					get_e_para_ok = 1;		//以获取当前电参数

					FaultInputCurrent = InputCurrent;		//获取当前电流值
					FaultInputVoltage = InputVoltage;		//获取当前电压值
				}
			}
		}

//		CheckEventsEC15(LightLevelPercent);	//单灯正常开灯记录
//		CheckEventsEC16(LightLevelPercent);	//单灯正常关灯记录
//		CheckEventsEC17(LightLevelPercent);	//单灯异常开灯记录
//		CheckEventsEC18(LightLevelPercent);	//单灯异常关灯记录
//		CheckEventsEC19(LightLevelPercent);	//单灯电流过大记录
//		CheckEventsEC20(LightLevelPercent);	//单灯电流过小记录

		if(ResetFlag == 1)								//接收到重启的命令
		{
			ResetFlag = 0;
			delay_ms(5000);

			__disable_fault_irq();							//重启指令
			NVIC_SystemReset();
		}

		if(FrameWareState.state == FIRMWARE_DOWNLOADED)		//固件下载完成,即将引导新程序
		{
			delay_ms(1000);									//延时1秒,等待固件状态存入EEPROM

			__disable_fault_irq();							//关闭全局中断
			NVIC_SystemReset();								//重启指令
		}
		else if(FrameWareState.state == FIRMWARE_DOWNLOAD_FAILED)
		{
			FrameWareState.state = FIRMWARE_FREE;			//暂时不进行固件下载,等到下次上电的时候再下载
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

	if(SwitchMode != 1)							//非年表控制
	{
		*switch_mode = 1;						//默认开灯
	}
	else										//年表控制
	{
		if(LampsSwitchProject.total_days >= 1 &&
		   LampsSwitchProject.total_days <= 366)	//年表不为空
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

						if(month_c >= 1 && month_c <= 12 && date_c >= 1 && date_c <= 31)		//得到合法日期
						{
							if(month_c == calendar.w_month && date_c == calendar.w_date)
							{
								on_hour = LampsSwitchProject.switch_time[i].on_time[0];
								on_minute = LampsSwitchProject.switch_time[i].on_time[1];
								off_hour = LampsSwitchProject.switch_time[i].off_time[0];
								off_minute = LampsSwitchProject.switch_time[i].off_time[1];

								if(on_hour <= 23 && on_minute <= 59 && off_hour <= 23 && off_minute <= 59)		//得到合法时间
								{
									on_gate = on_hour * 60 + on_minute;
									off_gate = off_hour * 60 + off_minute;
									now_gate = calendar.hour * 60 + calendar.min;

									if(on_gate != off_gate)
									{
										if(on_gate < off_gate)		//先开灯后关灯
										{
											if(on_gate <= now_gate && now_gate < off_gate)	//当前时间大于等于开灯时间,小于关灯时间,则开灯
											{
												*switch_mode = 1;		//开灯
											}
											else	//当前时间小于开灯时间,或者大于等于关灯时间,则关灯
											{
												*switch_mode = 0;		//关灯
											}
										}
										else						//先关灯后开灯
										{
											if(off_gate <= now_gate && now_gate < on_gate)	//当前时间大于等于关灯时间,小于开灯时间,则关灯
											{
												*switch_mode = 0;		//关灯
											}
											else	//当前时间小于关灯时间,或者大于等于开灯时间,则开灯
											{
												*switch_mode = 1;		//开灯
											}
										}
									}
									else					//开关灯时间相同
									{
										*switch_mode = 1;		//默认亮灯
									}
								}
								else	//得到非法时间
								{
									*switch_mode = 1;		//默认亮灯
								}

								i = 0xFF00;		//大于366即可
							}
						}
						else	//得到非法日期
						{
							*switch_mode = 1;	//默认亮灯
						}
					}
					else		//日期超出范围
					{
						*switch_mode = 1;		//默认亮灯
					}
				}
			}
			else								//年表表头错误
			{
				*switch_mode = 1;				//默认亮灯
			}
		}
		else									//年表为空
		{
			*switch_mode = 1;					//默认亮灯
		}
	}
}

//轮询时间策略
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
//			   calendar.w_year - 2000 <= tmp_time->range.year_e)		//当前年处于起始和结束年之间
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
							   tmp_time->minute == calendar.min)		//判断当前时间是否同该条策略时间相同
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

	if(calendar.week <= 6)				//判断是否是工作日
	{
		if(RegularTimeWeekDay->next != NULL)		//判断策略列表是否不为空
		{
			ret = 0;

			for(tmp_time = RegularTimeWeekDay->next; tmp_time != NULL; tmp_time = tmp_time->next)	//轮训策略列表
			{
				if(tmp_time->hour 	== calendar.hour &&
				   tmp_time->minute == calendar.min)		//判断当前时间是否同该条策略时间相同
				{
					ret = 1;
				}
				else if(tmp_time->next != NULL)				//该条策略是不是最后一条
				{
					if(tmp_time->next->hour   == calendar.hour &&
					   tmp_time->next->minute == calendar.min)		//判断该条策略的next的时间是否与当前时间相同
					{
						tmp_time = tmp_time->next;

						ret = 1;
					}
					else
					{
						gate1 = tmp_time->hour * 60 + tmp_time->minute;					//该条策略的分钟数
						gate2 = tmp_time->next->hour * 60 + tmp_time->next->minute;		//该条策略的next的分钟数
						gate_n = calendar.hour * 60 + calendar.min;						//当前时间的分钟数

						if(gate1 < gate2)												//该条策略时间早于next的时间
						{
							if(gate1 <= gate_n && gate_n <= gate2)						//判断当前时间是否在两条策略时间段中间
							{
								ret = 1;
							}
						}
						else if(gate1 > gate2)											//该条策略时间晚于next的时间
						{
							if(gate1 <= gate_n && gate_n <= gate24)						//判断当前时间是否在该条策略时间和24点时间段中间
							{
								ret = 1;
							}
							else if(gate0 <= gate_n && gate_n <= gate2)					//判断当前时间是否在0点和next的时间段中间
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



































