#include "event.h"
#include "24cxx.h"
#include "task_sensor.h"

//记录并存储发生的事件
void RecordEventsECx(u8 ecx,u8 len,u8 *msg)
{
	u16 crc_cal = 0;
	u16 add_pos = 0;
	u8 buf[24];

	if((EventEffective & ((long long)1 << ((long long)ecx - 1))) == 0x00)
	{
		return;
	}
	
	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_EVENT_RECORD, portMAX_DELAY);
	}

	add_pos = E_IMPORTEAT_ADD;

	memset(buf,0,24);

	buf[0] = ecx;						//事件代号
	buf[1] = len + 6;					//事件长度

	memcpy(buf + 2,CalendarClock,6);	//发生时间
	memcpy(buf + 8,msg,len);			//具体事件内容

	crc_cal = GetCRC16(buf,22);			//计算校验

	buf[22] = (u8)(crc_cal >> 8);
	buf[23] = (u8)(crc_cal & 0x00FF);

	EventRecordList.lable1[EventRecordList.ec1] = ecx;	//更新事件列表

	crc_cal = GetCRC16(EventRecordList.lable1,128);

	AT24CXX_WriteOneByte(EC1_LABLE_ADD + EventRecordList.ec1,EventRecordList.lable1[EventRecordList.ec1]);
	AT24CXX_WriteOneByte(EC1_LABLE_ADD + 128,(u8)(crc_cal >> 8));
	AT24CXX_WriteOneByte(EC1_LABLE_ADD + 129,(u8)(crc_cal & 0x00FF));

	WriteDataFromMemoryToEeprom(buf,add_pos + EventRecordList.ec1 * EVENT_LEN, EVENT_LEN);

	EventRecordList.ec1 ++;				//更新事件计数器

	WriteDataFromMemoryToEeprom(&EventRecordList.ec1,EC1_ADD, 1);

	EventRecordList.important_event_flag ++;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_EVENT_RECORD);
	}
}

//检测正常开灯事件
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

		cnt = GetSysTick1s();
		mirror_level = light_level;
	}

	if(mirror_level != light_level)		//单灯状态有变化
	{
		if(state_change == 0)
		{
			cnt = GetSysTick1s();

			state_change = 1;
		}

		if(GetSysTick1s() - cnt >= EventDetectConf.turn_on_collect_delay * 60)	//等待一段时间，计算电流和电压
		{
			cnt = 0;
			state_change = 0;

			if(mirror_level == 0)	//上个状态为关灯
			{
				if(light_level >= 1)		//现在状态为开灯
				{
					memset(buf,0,6);

					if(InputCurrent <= SWITCH_ON_MIN_CURRENT)			//开灯异常
					{
						buf[0] = 0x00;		//开灯成功数
						buf[1] = 0x00;
						buf[2] = 0x00;		//开灯失败数
						buf[3] = 0x01;
						buf[4] = 0x00;		//失败灯号
						buf[5] = 0x01;
					}
					else												//开灯正常
					{
						buf[0] = 0x00;		//开灯成功数
						buf[1] = 0x01;
						buf[2] = 0x00;		//开灯失败数
						buf[3] = 0x00;
						buf[4] = 0x00;		//失败灯号
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

//检测正常关灯
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

		cnt = GetSysTick1s();
		mirror_level = light_level;
	}

	if(mirror_level != light_level)		//单灯状态有变化
	{
		if(state_change == 0)
		{
			cnt = GetSysTick1s();

			state_change = 1;
		}

		if(GetSysTick1s() - cnt >= EventDetectConf.turn_off_collect_delay * 60)	//等待一段时间，计算电流和电压
		{
			cnt = 0;
			state_change = 0;

			if(mirror_level >= 1)	//上个状态为开灯
			{
				if(light_level == 0)		//现在状态为关灯
				{
					memset(buf,0,6);

					if(InputCurrent > SWITCH_OFF_MAX_CURRENT)			//关灯异常
					{
						buf[0] = 0x00;		//开灯成功数
						buf[1] = 0x00;
						buf[2] = 0x00;		//开灯失败数
						buf[3] = 0x01;
						buf[4] = 0x00;		//失败灯号
						buf[5] = 0x01;
					}
					else												//关灯正常
					{
						buf[0] = 0x00;		//开灯成功数
						buf[1] = 0x01;
						buf[2] = 0x00;		//开灯失败数
						buf[3] = 0x00;
						buf[4] = 0x00;		//失败灯号
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

//单灯异常开灯
void CheckEventsEC17(u8 light_level)
{
	static s16 cnt = 0;
	static u8 occur = 0;
	u8 buf[4];

	if(light_level == 0)			//现在状态为关灯
	{
		if(InputCurrent > SWITCH_OFF_MAX_CURRENT)				//异常开灯
		{
			if(occur == 0)
			{
				if(cnt < 1200)
				{
					cnt ++;
				}
			}
			else if(cnt < 0)
			{
				cnt = 0;
			}
		}
		else
		{
			if(occur == 1)
			{
				if(cnt > -1200)
				{
					cnt --;
				}
			}
			else if(cnt > 0)
			{
				cnt = 0;
			}
		}
	}
	else
	{
		cnt = 0;
		occur = 0;
	}

	if(cnt >= 1200)
	{
		cnt = 0;
		occur = 1;

		memset(buf,0,4);

		buf[0] = 0x01;														//记录类型
		buf[1] = 0x00;														//灯具序号
		buf[2] = 0x01;
		buf[3] = 0x00;														//事件开始

		RecordEventsECx(EVENT_ERC17,4,buf);
	}
	else if(cnt <= -1200)
	{
		cnt = 0;
		occur = 0;

		memset(buf,0,4);

		buf[0] = 0x00;														//记录类型
		buf[1] = 0x00;														//灯具序号
		buf[2] = 0x01;
		buf[3] = 0x00;														//事件结束

		RecordEventsECx(EVENT_ERC17,4,buf);
	}
}

//单灯异常关灯
void CheckEventsEC18(u8 light_level)
{
	static s16 cnt = 0;
	static u8 occur = 0;
	u8 buf[4];

	if(light_level == 100)			//现在状态为开灯
	{
		if(InputCurrent <= SWITCH_ON_MIN_CURRENT)		//异常关灯
		{
			if(occur == 0)
			{
				if(cnt < 1200)
				{
					cnt ++;
				}
			}
			else if(cnt < 0)
			{
				cnt = 0;
			}
		}
		else
		{
			if(occur == 1)
			{
				if(cnt > -1200)
				{
					cnt --;
				}
			}
			else if(cnt > 0)
			{
				cnt = 0;
			}
		}
	}
	else
	{
		cnt = 0;
		occur = 0;
	}

	if(cnt >= 1200)
	{
		cnt = 0;
		occur = 1;

		memset(buf,0,4);

		buf[0] = 0x01;
		buf[1] = 0x00;					//灯具序号
		buf[2] = 0x01;
		buf[3] = 0x00;

		RecordEventsECx(EVENT_ERC18,4,buf);
	}
	else if(cnt <= -1200)
	{
		cnt = 0;
		occur = 0;

		memset(buf,0,4);

		buf[0] = 0x00;
		buf[1] = 0x00;					//灯具序号
		buf[2] = 0x01;
		buf[3] = 0x00;

		RecordEventsECx(EVENT_ERC18,4,buf);
	}
}

//检测单灯电流过大记录
void CheckEventsEC19(u8 light_level,u8 get_e_para_ok)
{
	static u8 mirror_level = 0;
	static u8 occur = 0;
	static s16 cnt = 0;
	static u8 first = 1;
	u8 buf[7];

	if(first == 1)
	{
		first = 0;

		mirror_level = light_level;
	}

	if(light_level < 20 || get_e_para_ok == 0)
	{
		return;
	}

	if(mirror_level != light_level)		//单灯状态有变化
	{
		occur = 0;
		cnt = 0;

		mirror_level = light_level;
	}

	if(occur == 0)
	{
		if((InputCurrent - FaultInputCurrent) / FaultInputCurrent >=
		  (float)EventDetectConf.over_current_ratio / 100.0f)
		{
			if(cnt < 1200)
			{
				cnt ++;
			}
		}
		else
		{
			cnt = 0;
		}

		if(cnt >= 1200)
		{
			cnt = 0;
			occur = 1;

			buf[0] = 0x01;														//记录类型
			buf[1] = 0x00;														//灯具序号
			buf[2] = 0x01;
			buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//发生时电流
			buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
			buf[5] = light_level;												//发生时控制状态

			RecordEventsECx(EVENT_ERC19,6,buf);
		}
	}
	else if(occur == 1)
	{
		if((InputCurrent - FaultInputCurrent) / FaultInputCurrent <=
		  (float)EventDetectConf.over_current_recovery_ratio / 100.0f)
		{
			if(cnt < 1200)
			{
				cnt ++;
			}
		}
		else
		{
			cnt = 0;
		}

		if(cnt >= 1200)
		{
			cnt = 0;
			occur = 0;

			buf[0] = 0x00;
			buf[1] = 0x00;		//灯具序号
			buf[2] = 0x01;
			buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//发生时电流
			buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
			buf[5] = light_level;

			RecordEventsECx(EVENT_ERC19,6,buf);
		}
	}
}

//检测单灯电流过小记录
void CheckEventsEC20(u8 light_level,u8 get_e_para_ok)
{
	static u8 mirror_level = 0;
	static u8 occur = 0;
	static s16 cnt = 0;
	static u8 first = 1;
	u8 buf[7];

	if(first == 1)
	{
		first = 0;

		mirror_level = light_level;
	}

	if(light_level < 20 || get_e_para_ok == 0)
	{
		return;
	}

	if(mirror_level != light_level)
	{
		occur = 0;
		cnt = 0;

		mirror_level = light_level;
	}

	if(occur == 0)
	{
		if((FaultInputCurrent - InputCurrent) / FaultInputCurrent >=
		  (float)EventDetectConf.low_current_ratio / 100.0f)
		{
			if(cnt < 1200)
			{
				cnt ++;
			}
		}
		else
		{
			cnt = 0;
		}

		if(cnt >= 1200)
		{
			cnt = 0;
			occur = 1;

			buf[0] = 0x01;														//记录类型
			buf[1] = 0x00;														//灯具序号
			buf[2] = 0x01;
			buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//发生时电流
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
			if(cnt < 1200)
			{
				cnt ++;
			}
		}
		else
		{
			cnt = 0;
		}

		if(cnt >= 1200)
		{
			cnt = 0;
			occur = 0;

			buf[0] = 0x00;														//记录类型
			buf[1] = 0x00;														//灯具序号
			buf[2] = 0x01;
			buf[3] = (u8)((((u16)InputCurrent) >> 8) & 0x00FF);					//发生时电流
			buf[4] = (u8)(((u16)InputCurrent) & 0x00FF);
			buf[5] = light_level;

			RecordEventsECx(EVENT_ERC20,6,buf);
		}
	}
}

//校时结果事件记录
void CheckEventsEC28(u8 *cal1,u8 *cal2)
{
	u8 buf[13];

	memset(buf,0,13);

	buf[0] = 0x00;

	memcpy(&buf[1],cal1,6);		//校时前时间
	memcpy(&buf[7],cal2,6);		//校时后时间

	RecordEventsECx(EVENT_ERC28,13,buf);
}

//升级结果事件记录
void CheckEventsEC51(u8 result,u16 version)
{
	u8 buf[3];

	memset(buf,0,3);

	buf[0] = result;			//记录类型 成功或失败
	buf[1] = (u8)((version >> 8) & 0x00FF);
	buf[2] = (u8)(version & 0x00FF);

	RecordEventsECx(EVENT_ERC51,3,buf);
}

//单灯状态变化记录
void CheckEventsEC52(u8 light_level)
{
	static u8 mirror_level = 255;
	u8 buf[12];
	static u8 cnt = 0;
//	static u8 first = 1;

//	if(first == 1)
//	{
//		first = 0;

//		mirror_level = light_level;
//	}

	if(mirror_level != light_level)
	{
		cnt ++;

		if(cnt >= 200)	//等待一段时间，计算电流和电压
		{
			cnt = 0;

			memset(buf,0,14);

			buf[0] = DeviceWorkMode;											//记录类型
			buf[1] = 0x00;														//灯具序号
			buf[2] = 0x01;
			buf[3] = light_level;												//亮度

			buf[4] = (u8)((p_tSensorMsg->in_put_voltage >> 8) & 0x00FF);		//发生时电压
			buf[5] = (u8)(p_tSensorMsg->in_put_voltage & 0x00FF);

			buf[6] = (u8)((p_tSensorMsg->in_put_current >> 8) & 0x00FF);		//发生时电流
			buf[7] = (u8)(p_tSensorMsg->in_put_current & 0x00FF);

			buf[8] = (u8)((((u16)p_tSensorMsg->in_put_power_p) >> 8) & 0x00FF);	//发生时电流
			buf[9] = (u8)(((u16)p_tSensorMsg->in_put_power_p) & 0x00FF);

			buf[10] = (u8)((p_tSensorMsg->in_put_pf >> 8) & 0x00FF);			//发生时电流
			buf[11] = (u8)(p_tSensorMsg->in_put_pf & 0x00FF);

			RecordEventsECx(EVENT_ERC52,12,buf);

			mirror_level = light_level;
		}
	}
	else
	{
		cnt = 0;
	}
}














































