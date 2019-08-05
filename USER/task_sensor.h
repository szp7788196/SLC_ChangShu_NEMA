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
	u32 in_put_energy_p;
	u32 in_put_energy_q;
	u32 in_put_energy_s;
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


extern float InputCurrent;
extern float InputVoltage;
extern float InputFreq;
extern float InputPowerP;
extern float InputPowerQ;
extern float InputPowerS;
extern float InputPf;
extern float InputEnergyP;
extern float InputEnergyQ;
extern float InputEnergyS;
extern float Temperature;
extern float Humidity;
extern float Illumination;


extern TaskHandle_t xHandleTaskSENSOR;

extern SensorMsg_S *p_tSensorMsg;	//用于装在传感器数据的结构体变量

void vTaskSENSOR(void *pvParameters);






































#endif
