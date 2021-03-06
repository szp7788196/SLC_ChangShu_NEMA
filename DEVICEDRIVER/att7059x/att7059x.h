#ifndef __ATT7059X_H
#define __ATT7059X_H

#include "sys.h"


#define ACK_OK		0x54
#define ACK_FAIL	0x63



#define POWER_RATIO						0.0187939698f		//功率转换系数
#define ELECTRIC_ENERGY_METER_CONSTANT	6400.0f				//电能表常数
#define CURRENT_RATIO					0.011531979f		//电流转换系数
#define VOLTAGE_RATIO					0.0002001529511f	//电压转换系数




u8 CalAtt7059CheckSum(u8 *buf, u8 len);
u8 Att7059xWriteOperate(u8 add, u16 data);
u8 Att7059xReadOperate(u8 add, u32 *data);

s32 Att7059xGetCurrent1ADCValue(void);
s32 Att7059xGetVoltageADCValue(void);
float Att7059xGetCurrent1(void);
float Att7059xGetVoltage(void);
float Att7059xGetVoltageFreq(void);
float Att7059xGetChannel1PowerP(void);
float Att7059xGetChannel1PowerQ(void);
float Att7059xGetChannel1PowerS(void);
double Att7059xGetEnergyP(void);
double Att7059xGetEnergyQ(void);
double Att7059xGetEnergyS(void);
s32 Att7059xGetMaxVoltageADCValue(void);


































#endif
