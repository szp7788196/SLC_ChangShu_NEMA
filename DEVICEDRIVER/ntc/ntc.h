#ifndef __NTC_H
#define __NTC_H

#include "sys.h"

#define ADC1_DR_ADDRESS ((u32)0x4001244C)

#define POINT_NUM	64




void ADC1_DMA1_Init(void);
float GetNTC_Temperature(void);



































#endif
