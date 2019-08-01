#include "ntc.h"
#include "math.h"


u16 AdcValBuf[POINT_NUM];

void ADC1_DMA1_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

	DMA_Cmd(DMA1_Channel1, DISABLE);
	ADC_DMACmd(ADC1, DISABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AdcValBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = POINT_NUM;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_239Cycles5);

    ADC_DMACmd(ADC1, ENABLE);
    ADC_TempSensorVrefintCmd(ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


//获取ADC采样均值
u16 ADCGetAverageVal(void)
{
	u8 i = 0;
	u32 result = 0;

	for(i = 0; i < POINT_NUM; i ++)
	{
		result += AdcValBuf[i];
	}
	
	result = result / (u32)POINT_NUM;
	return (u16)result;
}

//获取NTC温度
float GetNTC_Temperature(void)
{
	float vol = 0.0f;
	float vol_cha = 0.0f;
	float current = 0.0f;
	float resistor = 0.0f;
	float temp = 0.0f;
	float exp = 0.0f;
	float x = 0.0f;
	float z = 0.0f;
	float y = 0.0f;
	float w = 0.0f;

	vol = (float)(ADCGetAverageVal()) * 3.3f / 4096.0f;
	vol_cha = 3.3f - vol;
	current = vol_cha / 10000.0f;
	resistor = vol / current;
	
	exp = resistor / 10000.0f;
	x = log(exp);
	z = x / 3950.0f;
	y = 1 / 298.15f;	//(273.15f + 25.0f)
	w = z + y;
	temp = 1 / w;
	temp = temp - 273.15f;

	return temp;
}
































