#ifndef __NET_PROTOCOL_H
#define __NET_PROTOCOL_H

#include "sys.h"
#include "bcxx.h"
#include "common.h"

/******************************************************************************************
/                                    通讯错误码
/	
/	
/	
/	
/	
/	
/	
/
******************************************************************************************/
s16 NetDataFrameHandle(u8 *outbuf);
u16 NetDataAnalysis(u8 *buf,u16 len,u8 *outbuf);

u8 UnPackAckPacket(u8 *buf,u8 len);
u16 PackAckPacket(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf);

u16 CombineLogin_outFrame(u8 mode,u8 *outbuf);
u16 CombineHeartBeatFrame(u8 *outbuf);
u16 CombineSensorDataFrame(u8 *outbuf);
u16 CombineRequestFrameWareFrame(u8 *outbuf);
u16 CombineFaultEventFrame(u8 *outbuf);

u16 RecvFrameWareBag(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetDeviveReset(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetReUpLoadPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetHeartBeatUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetDataUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetDeviceWorkMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetSwitchMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetIlluminanceThreshold(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetDeviceConfigPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetPowerInterface(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SyncDateTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetLampsSwitchProject(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetTimeStrategy(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetDeviceInfo(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetFrameWareInfo(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetDefaultLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetPeakStaggerTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetFaultDetectDelayPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetFaultDetectThrePara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 SetEventRecordReportConf(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetReUpLoadPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetHeartBeatUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetDataUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetDeviceWorkMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetSwitchMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetIlluminanceThreshold(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetDeviceConfigPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetPowerInterface(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetLampsSwitchProject(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetTimeStrategy(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetDefaultLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetSensorData(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetPeakStaggerTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetFaultEvents(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetFaultDetectDelayPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetFaultDetectThrePara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);
u16 GetEventRecordReportConf(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf);




































#endif
