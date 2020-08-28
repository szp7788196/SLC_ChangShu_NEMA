#include "task_sensor.h"
#include "delay.h"
#include "sht2x.h"
#include "bh1750.h"
#include "task_net.h"
#include "common.h"
#include "inventr.h"
#include "rtc.h"
#include "usart.h"
#include "att7059x.h"
#include "ntc.h"

float InputCurrent = 0;
float InputVoltage = 0;
float InputFreq = 0.0f;
float InputPowerP = 0.0f;
float InputPowerQ = 0.0f;
float InputPowerS = 0.0f;
float InputPf = 0.0f;
double InputEnergyP = 0.0f;
double InputEnergyQ = 0.0f;
double InputEnergyS = 0.0f;
double InputEnergyP_Prev = 0.0f;
double InputEnergyQ_Prev  = 0.0f;
double InputEnergyS_Prev  = 0.0f;
float Temperature = 0.0f;
float Humidity = 0.0f;
float Illumination = 0.0f;

TaskHandle_t xHandleTaskSENSOR = NULL;

SensorMsg_S *p_tSensorMsg = NULL;	//用于装在传感器数据的结构体变量
unsigned portBASE_TYPE SENSOR_Satck;
void vTaskSENSOR(void *pvParameters)
{
	u8 first_flag = 0;
	
	p_tSensorMsg = (SensorMsg_S *)mymalloc(sizeof(SensorMsg_S));

	while(1)
	{
		if(PowerInterface == INTFC_DIGIT)
		{
			InventrOutPutCurrent = InventrGetOutPutCurrent();	//读取电源输出电流
			delay_ms(500);
			InventrOutPutVoltage = InventrGetOutPutVoltage();	//读取电源输出电压
		}

		delay_ms(100);
		InputCurrent 	= Att7059xGetCurrent1();
		delay_ms(100);
		InputVoltage 	= Att7059xGetVoltage();
		delay_ms(100);
		InputFreq 		= Att7059xGetVoltageFreq();
		delay_ms(100);
		InputPowerP 	= Att7059xGetChannel1PowerP();
		delay_ms(100);
		InputPowerQ 	= Att7059xGetChannel1PowerQ();
		delay_ms(100);
		InputPowerS 	= Att7059xGetChannel1PowerS();
		delay_ms(100);
		Temperature 	= GetNTC_Temperature();
		InputEnergyP 	= Att7059xGetEnergyP();
		delay_ms(100);
		InputEnergyQ 	= Att7059xGetEnergyQ();
		delay_ms(100);
		InputEnergyS 	= Att7059xGetEnergyS();
		
		if(first_flag == 0)
		{
			first_flag = 1;
			
			InputEnergyP_Prev = InputEnergyP;
			InputEnergyQ_Prev = InputEnergyQ;
			InputEnergyS_Prev = InputEnergyS;
		}

		EnergyRecord.energy_p_day += (InputEnergyP - InputEnergyP_Prev);
		EnergyRecord.energy_q_day += (InputEnergyQ - InputEnergyQ_Prev);
		EnergyRecord.energy_s_day += (InputEnergyS - InputEnergyS_Prev);

		EnergyRecord.energy_p_total += (InputEnergyP - InputEnergyP_Prev);
		EnergyRecord.energy_q_total += (InputEnergyQ - InputEnergyQ_Prev);
		EnergyRecord.energy_s_total += (InputEnergyS - InputEnergyS_Prev);

		InputEnergyP_Prev = InputEnergyP;
		InputEnergyQ_Prev = InputEnergyQ;
		InputEnergyS_Prev = InputEnergyS;
		p_tSensorMsg->in_put_current 	= (u16)(InputCurrent + 0.5f);
		p_tSensorMsg->in_put_voltage 	= (u16)(InputVoltage + 0.5f);
		p_tSensorMsg->in_put_freq 		= (u16)(InputFreq * 100.0f + 0.5f);
		p_tSensorMsg->in_put_power_p 	= InputPowerP >= 0 ? (s16)(InputPowerP + 0.5f) : (s16)(InputPowerP - 0.5f);
		p_tSensorMsg->in_put_power_q 	= InputPowerQ >= 0 ? (s16)(InputPowerQ + 0.5f) : (s16)(InputPowerQ - 0.5f);
		p_tSensorMsg->in_put_power_s 	= (u16)(InputPowerS + 0.5f);
		p_tSensorMsg->in_put_pf			= (u16)((float)(p_tSensorMsg->in_put_power_p / (float)p_tSensorMsg->in_put_power_s) * 100.0f + 0.5f);
		p_tSensorMsg->in_put_energy_p_day 	= (u32)(EnergyRecord.energy_p_day * 1000.0f + 0.5f);
		p_tSensorMsg->in_put_energy_q_day 	= (u32)(EnergyRecord.energy_q_day * 1000.0f + 0.5f);
		p_tSensorMsg->in_put_energy_s_day 	= (u32)(EnergyRecord.energy_s_day * 1000.0f + 0.5f);
		p_tSensorMsg->in_put_energy_p_total = (u32)(EnergyRecord.energy_p_total * 1000.0f + 0.5f);
		p_tSensorMsg->in_put_energy_q_total = (u32)(EnergyRecord.energy_q_total * 1000.0f + 0.5f);
		p_tSensorMsg->in_put_energy_s_total = (u32)(EnergyRecord.energy_s_total * 1000.0f + 0.5f);
		p_tSensorMsg->out_put_current 	= (u16)(InventrOutPutCurrent + 0.5f);
		p_tSensorMsg->out_put_voltage 	= (u16)(InventrOutPutVoltage + 0.5f);
		p_tSensorMsg->csq 				= 113 - (NB_ModulePara.csq * 2);
		p_tSensorMsg->temperature		= Temperature;
		p_tSensorMsg->humidity			= Humidity;
		p_tSensorMsg->illumination		= Illumination;
		p_tSensorMsg->light_level		= LightLevelPercent;

		p_tSensorMsg->year 				= calendar.w_year - 2000;
		p_tSensorMsg->month 			= calendar.w_month;
		p_tSensorMsg->date 				= calendar.w_date;
		p_tSensorMsg->hour 				= calendar.hour;
		p_tSensorMsg->minute 			= calendar.min;
		p_tSensorMsg->second 			= calendar.sec;

		delay_ms(100);

//		SENSOR_Satck = uxTaskGetStackHighWaterMark(NULL);
	}
}






































