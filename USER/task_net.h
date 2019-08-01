#ifndef __TASK_NET_H
#define __TASK_NET_H

#include "sys.h"
#include "common.h"
#include "rtos_task.h"
#include "bcxx.h"
#include "task_sensor.h"


#define LOGIN_OUT_TIMEOUT	60

typedef struct 
{
	char ip_adderss[16];
}NTPServerTypeDef;


extern TaskHandle_t xHandleTaskNET;
extern SensorMsg_S *p_tSensorMsgNet;

extern CONNECT_STATE_E ConnectState;
extern u8 LoginState;			//发送UUID成功标识


void vTaskNET(void *pvParameters);
u8 TryToConnectToServer(void);
s8 OnServerHandle(void);
s16 SendEventRequestToServer(u8 *outbuf);
u8 SyncDataTimeFormM53xxModule(time_t sync_cycle);































#endif
