#ifndef __TASK_SENSOR_H
#define __TASK_SENSOR_H

#include "sys.h"
#include "rtos_task.h"


typedef struct SensorMsg
{
	u16 temperature;
	u16 humidity;
	u16 illumination;
	u16 in_put_current;
	u16 in_put_voltage;
	u16 in_put_freq;
	s16 in_put_power_p;
	s16 in_put_power_q;
	u16 in_put_power_s;
	u16 in_put_pf;
	u32 in_put_energy_p_day;
	u32 in_put_energy_q_day;
	u32 in_put_energy_s_day;
	u32 in_put_energy_p_total;
	u32 in_put_energy_q_total;
	u32 in_put_energy_s_total;
	u16 out_put_current;
	u16 out_put_voltage;
	u8 light_level;
	u8 csq;
	u8 year;
	u8 month;
	u8 date;
	u8 hour;
	u8 minute;
	u8 second;
}SensorMsg_S;
typedef struct EnergyRecord
{
	double energy_p_day;
	double energy_q_day;
	double energy_s_day;
	double energy_p_total;
	double energy_q_total;
	double energy_s_total;
	u8 year;
	u8 month;
	u8 date;
	u8 hour;
	u8 minute;
	u8 second;
}EnergyRecord_S;


extern float InputCurrent;
extern float InputVoltage;
extern float InputFreq;
extern float InputPowerP;
extern float InputPowerQ;
extern float InputPowerS;
extern float InputPf;
extern double InputEnergyP;
extern double InputEnergyQ;
extern double InputEnergyS;
extern float Temperature;
extern float Humidity;
extern float Illumination;


extern TaskHandle_t xHandleTaskSENSOR;

extern SensorMsg_S *p_tSensorMsg;	//用于装在传感器数据的结构体变量

void vTaskSENSOR(void *pvParameters);






































#endif
