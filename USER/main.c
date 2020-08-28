#include "common.h"
#include "rtos_task.h"
#include "24cxx.h"
#include "led.h"
#include "rtc.h"
#include "usart.h"
#include "uart4.h"
#include "pwm.h"
#include "dac.h"
#include "att7059x.h"
#include "inventr.h"
#include "dali.h"
#include "cd4051b.h"
#include "ntc.h"
#include "bcxx.h"

//u16 i = 0;
//u8 eepbuf[256];
//u16 cnt = 0;
//u8 led_s = 0;
RCC_ClocksTypeDef RCC_Clocks;

int main(void)
{
	SCB->VTOR = FLASH_BASE | 0x06000; 	/* Vector Table Relocation in Internal FLASH. */
	IWDG_Init(IWDG_Prescaler_128,1600);	//128分频 312.5HZ 625为2秒

	RCC_GetClocksFreq(&RCC_Clocks);		//查看各个总线的时钟频率
	__set_PRIMASK(1);	//关闭全局中断

	NVIC_Configuration();
	delay_init(72);
	RTC_Init();
	AT24CXX_Init();
	DAC1_Init();
	ADC1_DMA1_Init();
	PVD_Init();
	LED_Init();
	RELAY_Init();
	CD4051B_Init();
	TIM2_Init(99,7199);
	TIM5_Init(2000,36 - 1);
	USART1_Init(115200);
	USART3_Init(4800);
	USART2_Init(9600);
	UART4_Init(9600);

	__set_PRIMASK(0);	//开启全局中断

//	FrameWareState.state 			= FIRMWARE_FREE;
//	WriteFrameWareStateToEeprom();	//将固件升级状态写入EEPROM

//	for(i = 0; i < 256; i ++)
//	{
//		AT24CXX_WriteOneByte(i,i);
//	}
//	for(i = 0; i < 256; i ++)
//	{
//		eepbuf[i] = AT24CXX_ReadOneByte(i);
//	}
//	AT24CXX_WriteOneByte(UU_ID_ADD,255);

	mem_init();

	IWDG_Feed();				//喂看门狗

	ReadParametersFromEEPROM();	//读取所有的运行参数

	AppObjCreate();				//创建消息队列、互斥量
	AppTaskCreate();			//创建任务

	vTaskStartScheduler();		//启动调度，开始执行任务

	while(1)
	{
		delay_ms(100);
	}
}

























