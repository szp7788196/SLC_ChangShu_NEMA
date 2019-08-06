#include "net_protocol.h"
#include "rtc.h"
#include "usart.h"
#include "24cxx.h"
#include "common.h"
#include "task_net.h"
#include "error.h"
#include "utils.h"
#include "uart4.h"


//读取/处理网络数据
u16 time_out = 0;
s16 NetDataFrameHandle(u8 *outbuf)
{
	u16 i = 0;
	s16 ret = 0;
	u16 len = 0;
	u8 buf[NET_BUF_MAX_LEN];
	u8 *msg = NULL;
	char tmp[10];

	memset(buf,0,NET_BUF_MAX_LEN);

	ret = nbiot_udp_recv(buf,(size_t *)&len);

	if(ret == NBIOT_ERR_OK)
	{
		if(len != 0)
		{
			time_out = 0;

			msg = (u8 *)strstr((char *)buf,":");

			if(msg == NULL)
				return ret;

			msg = msg + 1;

			memset(tmp,0,10);

			while(*msg != ',')
				tmp[i ++] = *(msg ++);

			tmp[i] = '\0';
			len = nbiot_atoi(tmp,strlen(tmp));

			msg = msg + 1;

			StrToHex(buf,(char *)msg,len);

			ret = (s16)NetDataAnalysis(buf,len,outbuf);
		}
		else
		{
			time_out ++;

			if(time_out >= 3000)		//一分钟内未收到任何数据，强行关闭连接
			{
				time_out = 0;

				ret = -1;
			}
		}
	}

	return ret;
}

//网络数据帧协议解析
u16 NetDataAnalysis(u8 *inbuf,u16 len,u8 *outbuf)
{
	u16 ret = 0;

	u8 message_id = 0;
	u16 cmd_id = 0;
	u8 *msg = NULL;
	u8 check_sum_read = 0;
	u8 check_sum_cal = 0;
	u8 mail_add[8];
//	u8 protocol_ver = 0;
	u8 ctrl_code = 0;
	u16 data_len = 0;

	message_id = *(inbuf + 0);

	if(message_id != 0x81 && message_id != 0x84)	//判断消息ID
	{
		return 0;
	}

	switch(message_id)
	{
		case 0x81:				//命令下发
			cmd_id = (((u16)(*(inbuf + 1))) << 8) + (u16)(*(inbuf + 2));

			msg = inbuf + 5;	//指到用户数据
		break;

		case 0x84:				//事件响应

			msg = inbuf + 3;	//指到用户数据
		break;

		default:
		break;
	}

	if(*(msg + 0) != 0x68 || *(msg + 9) != 0x68 || *(inbuf + len - 1) != 0x16)	//判断帧起始符和帧结束符
	{
		return 0;
	}

	memcpy(mail_add,msg + 1,8);						//获取通讯地址

//	if(MyStrstr(mail_add, DeviceConfigPara.mail_add, 8, 8) == 0xFFFF)	//判断地址域(通讯地址)
//	{
//		return 0;
//	}

//	protocol_ver = (((u16)(*(msg + 8))) << 8) + (u16)(*(msg + 9));

//	if(protocol_ver >= DeviceInfo.protocol_ver)		//判断协议版本
//	{
//		return 0;
//	}

	ctrl_code = *(msg + 12);

	data_len = (((u16)(*(msg + 13))) << 8) + (u16)(*(msg + 14));

	check_sum_read = *(msg + 15 + data_len);

	check_sum_cal = CalCheckSum(msg, 15 + data_len);

	if(check_sum_cal != check_sum_read)				//校验和错误
	{
		return 0;
	}

	msg += 15;			//指到数据域

	switch(ctrl_code)
	{
		case 0x54:		//接收固件包数据
			ret = RecvFrameWareBag(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x60:		//复位命令
			ret = SetDeviveReset(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x61:		//设置超时时间、重发次数命令
			ret = SetReUpLoadPara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x62:		//设置心跳周期命令
			ret = SetHeartBeatUploadInterval(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x63:		//设置采集数据上报周期命令
			ret = SetDataUploadInterval(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x64:		//设置终端工作模式命令
			ret = SetDeviceWorkMode(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x65:		//设置开关灯模式命令
			ret = SetSwitchMode(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x66:		//设置光照度阈值命令
			ret = SetIlluminanceThreshold(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x67:		//设置单灯配置信息命令
			ret = SetDeviceConfigPara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x68:		//设置电源控制接口类型命令
			ret = SetPowerInterface(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x69:		//对时命令
			ret = SyncDateTime(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6A:		//即时调光命令
			ret = SetLightLevelPercent(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6B:		//设置开关灯年表命令
			ret = SetLampsSwitchProject(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6C:		//设置调光策略命令
			ret = SetTimeStrategy(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6D:		//读取终端信息命令
			ret = GetDeviceInfo(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6E:		//固件升级命令
			ret = SetFrameWareInfo(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x6F:		//设置上电默认亮度
			ret = SetDefaultLightLevelPercent(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x70:		//设置错峰时间
			ret = SetPeakStaggerTime(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x71:		//设置故障检测延时参数
			ret = SetFaultDetectDelayPara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x72:		//设置故障检测阈值参数
			ret = SetFaultDetectThrePara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x90:		//读取超时时间、重发次数命令
			ret = GetReUpLoadPara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x91:		//读取心跳周期
			ret = GetHeartBeatUploadInterval(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x92:		//读取采集数据上报周期命令
			ret = GetDataUploadInterval(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x93:		//读取终端工作模式命令
			ret = GetDeviceWorkMode(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x94:		//读取开关灯模式命令
			ret = GetSwitchMode(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x95:		//读取光照度阈值命令
			ret = GetIlluminanceThreshold(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x96:		//读取单灯配置信息命令
			ret = GetDeviceConfigPara(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x97:		//读取电源控制接口类型命令
			ret = GetPowerInterface(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x98:		//读取灯具当前亮度命令
			ret = GetLightLevelPercent(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x99:		//读取开关灯年表命令
			ret = GetLampsSwitchProject(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x9A:		//读取调光策略命令
			ret = GetTimeStrategy(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x9B:		//读取上电默认亮度
			ret = GetDefaultLightLevelPercent(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x9C:		//读取采集数据命令
			ret = GetSensorData(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x9D:		//读取采集数据命令
			ret = GetPeakStaggerTime(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x9E:		//读取故障事件命令
			ret = GetPeakStaggerTime(cmd_id,ctrl_code,msg,data_len,outbuf);
		break;

		case 0x80:		//事件响应
			ret = UnPackAckPacket(msg,data_len);
		break;

		default:
		break;
	}

	return ret;
}

//合并登录登出报文
//mode 0:登出；1:登录
u16 CombineLogin_outFrame(u8 mode,u8 *outbuf)
{
	u16 len = 0;

	u8 buf[23];

	buf[0] = mode;
	memcpy(buf + 1,DeviceInfo.iccid,20);
	buf[21] = (u8)(DeviceInfo.protocol_ver >> 8);
	buf[22] = (u8)(DeviceInfo.protocol_ver & 0x00FF);

	len = PackUserData(0x50,buf,23,outbuf + 3);

	len = PackEventUploadData(outbuf + 3,len,outbuf);

	WaittingRsp = 1;
	WaitRspCtrlCode = 0x50;

	return len;
}

//合并心跳报文
u16 CombineHeartBeatFrame(u8 *outbuf)
{
	u16 len = 0;

	len = PackUserData(0x51,NULL,0,outbuf + 3);

	len = PackEventUploadData(outbuf + 3,len,outbuf);

	WaittingRsp = 1;
	WaitRspCtrlCode = 0x51;

	return len;
}

//合并传感器数据报文
u16 CombineSensorDataFrame(u8 *outbuf)
{
	u16 len = 0;

	u8 buf[50];

	len = UnPackSensorData(p_tSensorMsg,buf);

	len = PackUserData(0x52,buf,len,outbuf + 3);

	len = PackEventUploadData(outbuf + 3,len,outbuf);

	WaittingRsp = 1;
	WaitRspCtrlCode = 0x52;

	return len;
}

//合并请求固件报文
u16 CombineRequestFrameWareFrame(u8 *outbuf)
{
	u16 len = 0;

	u8 buf[4];

	buf[0] = (u8)(FrameWareState.total_bags >> 8);
	buf[1] = (u8)(FrameWareState.total_bags & 0x00FF);
	buf[2] = (u8)(FrameWareState.current_bag_cnt >> 8);
	buf[3] = (u8)(FrameWareState.current_bag_cnt & 0x00FF);

	len = PackUserData(0x54,buf,4,outbuf + 3);

	len = PackEventUploadData(outbuf + 3,len,outbuf);

	WaittingRsp = 1;
	WaitRspCtrlCode = 0x54;

	return len;
}

//合并故障事件报文
u16 CombineFaultEventFrame(u8 *outbuf)
{
	u16 len = 0;

	u8 ret = 0;
	u8 temp0 = 0;
	u8 buf[32];
	u8 temp_buf[32];

	switch(EventRecordList.lable1[EventRecordList.ec1 - 1])
	{
		case 15:
			temp0 = 14;	//当前事件所占内存长度
		break;

		case 16:
			temp0 = 14;
		break;

		case 17:
			temp0 = 12;
		break;

		case 18:
			temp0 = 12;
		break;

		case 19:
			temp0 = 14;
		break;

		case 20:
			temp0 = 14;
		break;

		case 21:
			temp0 = 16;
		break;

		case 22:
			temp0 = 16;
		break;

		case 23:
			temp0 = 16;
		break;

		case 28:
			temp0 = 21;
		break;

		case 36:
			temp0 = 15;
		break;

		case 37:
			temp0 = 11;
		break;

		case 51:
			temp0 = 11;
		break;

		case 52:
			temp0 = 20;
		break;

		default:
		break;
	}

	buf[0] = EventRecordList.ec1;

	ret = ReadDataFromEepromToMemory(temp_buf,E_IMPORTEAT_ADD + (EventRecordList.ec1 - 1) * EVENT_LEN,EVENT_LEN);	//从EEPROM中读取时间内容

	if(ret == 1)
	{
		memcpy(&buf[1],temp_buf,temp0);			//读取成功 将时间内容放入数据单元
	}
	else
	{
		memset(&buf[1],0,temp0);					//读取失败 将数据单元清空
	}

	len = PackUserData(0x53,buf,temp0 + 1,outbuf + 3);

	len = PackEventUploadData(outbuf + 3,len,outbuf);

	WaittingRsp = 1;
	WaitRspCtrlCode = 0x53;

	return len;
}

//解析ACK包
u8 UnPackAckPacket(u8 *inbuf,u8 len)
{
	u8 ret = 0;

	if(len == 2)
	{
		if(WaittingRsp == 1)
		{
			if(*(inbuf + 0) == WaitRspCtrlCode)
			{
				WaittingRsp = 0;

				if(WaitRspCtrlCode == 0x50)
				{
					LoginState = 1;
				}
			}
		}
	}

	return ret;
}

//ACK打包
u16 PackAckPacket(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf)
{
	u16 len = 0;

	len = PackUserData(ctrl_code,inbuf,inbuf_len,outbuf + 7);

	len = PackCommandRspData(cmd_id,0,outbuf + 7,len,outbuf);

	return len;
}

//下发复位命令
u16 SetDeviveReset(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) < 3)
		{
			ResetFlag = *(inbuf + 0);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//接收固件包
u16 RecvFrameWareBag(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u16 i = 0;
	u16 total_bags = 0;
	u16 current_bags = 0;
	u16 bag_size = 0;
	u8 *msg = NULL;
	u16 crc_read = 0;
	u16 crc_cal = 0;
	u32 crc32_cal = 0xFFFFFFFF;
	u32 crc32_read = 0;
	u8 crc32_cal_buf[1024];
	u32 file_len = 0;
	u16 k_num = 0;
	u16 last_k_byte_num = 0;
	u16 temp = 0;

	if(WaittingRsp == 1)
	{
		if(*(inbuf + 0) == WaitRspCtrlCode)
		{
			WaittingRsp = 0;
		}
	}

	total_bags = (((u16)(*(inbuf + 0))) << 8) + (u16)(*(inbuf + 1));
	current_bags = (((u16)(*(inbuf + 2))) << 8) + (u16)(*(inbuf + 3));
	bag_size = (((u16)(*(inbuf + 4))) << 8) + (u16)(*(inbuf + 5));

	if(data_len == 6 + bag_size)
	{
		if(total_bags != FrameWareState.total_bags ||
		   current_bags > FrameWareState.total_bags)		//总包数匹配错误
		{
			return 0;
		}

		msg = inbuf + 6;

		crc_read = (((u16)(*(msg + bag_size - 2))) << 8) + (u16)(*(msg + bag_size - 1));

		crc_cal = GetCRC16(msg,bag_size - 2);

		if(crc_cal == crc_read)
		{
			if(current_bags == FrameWareState.current_bag_cnt)
			{
				if(current_bags < FrameWareState.total_bags)
				{
					FLASH_Unlock();						//解锁FLASH

					if(bag_size == FIRMWARE_BAG_SIZE)
					{
						for(i = 0; i < (FIRMWARE_BAG_SIZE - 2) / 2; i ++)
						{
							temp = ((u16)(*(msg + i * 2 + 1)) << 8) + (u16)(*(msg + i * 2));

							FLASH_ProgramHalfWord(FIRMWARE_BUCKUP_FLASH_BASE_ADD + (current_bags - 1) * (FIRMWARE_BAG_SIZE - 2) + i * 2,temp);
						}
					}

					FLASH_Lock();						//上锁

					FrameWareState.current_bag_cnt ++;

					FrameWareState.state = FIRMWARE_DOWNLOADING;	//当前包下载完成
				}
				else if(current_bags == FrameWareState.total_bags)
				{
					crc32_read = (((u32)(*(msg + 0))) << 24) +
					             (((u32)(*(msg + 1))) << 16) +
					             (((u32)(*(msg + 2))) << 8) +
					             (((u32)(*(msg + 3))));
					
					file_len = 256 * (FrameWareState.total_bags - 1);
					
					k_num = file_len / 1024;
					last_k_byte_num = file_len % 1024;
					if(last_k_byte_num > 0)
					{
						k_num += 1;
					}

					for(i = 0; i < k_num; i ++)
					{
						memset(crc32_cal_buf,0,1024);
						if(i < k_num - 1)
						{
							STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,1024);
							crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,0);
						}
						if(i == k_num - 1)
						{
							if(last_k_byte_num == 0)
							{
								STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,1024);
								crc32_cal = CRC32(crc32_cal_buf,1024,crc32_cal,1);
							}
							else if(last_k_byte_num > 0)
							{
								STMFLASH_ReadBytes(FIRMWARE_BUCKUP_FLASH_BASE_ADD + 1024 * i,crc32_cal_buf,last_k_byte_num);
								crc32_cal = CRC32(crc32_cal_buf,last_k_byte_num,crc32_cal,1);
							}
						}
					}

					if(crc32_read == crc32_cal)
					{
						FrameWareState.state = FIRMWARE_DOWNLOADED;		//全部下载完成
					}
					else
					{
						FrameWareState.state = FIRMWARE_DOWNLOAD_FAILED;		//全部下载完成
					}

					WriteFrameWareStateToEeprom();
				}
			}
		}
	}

	return ret;
}

//设置重发参数
u16 SetReUpLoadPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 2)
	{
		if(*(inbuf + 1) <= 3)
		{
			RspTimeOut = *(inbuf + 0);
			ReUpLoadTimes = *(inbuf + 1);

			WriteDataFromMemoryToEeprom(inbuf,
			                            RE_UPLOAD_PARA_ADD,
			                            RE_UPLOAD_PARA_LEN - 2);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置心跳包上送间隔
u16 SetHeartBeatUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 2)
	{
		HeartBeatUploadInterval = (((u16)(*(inbuf + 0))) << 8) + (u16)(*(inbuf + 1));

		WriteDataFromMemoryToEeprom(inbuf,
		                            HEART_BEAT_UPLOAD_INTER_ADD,
		                            HEART_BEAT_UPLOAD_INTER_LEN - 2);

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置重发参数
u16 SetDataUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 2)
	{
		DataUploadInterval = (((u16)(*(inbuf + 0))) << 8) + (u16)(*(inbuf + 1));

		WriteDataFromMemoryToEeprom(inbuf,
		                            DATA_UPLOAD_INTER_ADD,
		                            DATA_UPLOAD_INTER_LEN - 2);

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置工作模式
u16 SetDeviceWorkMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) <= MODE_MANUAL)
		{
			DeviceWorkMode = *(inbuf + 0);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置工作模式
u16 SetSwitchMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) <= ALWAYS_ON)
		{
			SwitchMode = *(inbuf + 0);

			WriteDataFromMemoryToEeprom(inbuf,
			                            SWITCH_MODE_ADD,
			                            SWITCH_MODE_LEN - 2);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置光照度阈值
u16 SetIlluminanceThreshold(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 4)
	{
		IlluminanceThreshold.on = (((u16)(*(inbuf + 0))) << 8) + (u16)(*(inbuf + 1));
		IlluminanceThreshold.off = (((u16)(*(inbuf + 2))) << 8) + (u16)(*(inbuf + 3));

		WriteDataFromMemoryToEeprom(inbuf,
		                            ILLUM_THER_ADD,
		                            ILLUM_THER_LEN - 2);

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置单灯配置信息
u16 SetDeviceConfigPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 33)
	{
		memcpy(DeviceConfigPara.mail_add,inbuf + 0,8);

		DeviceConfigPara.lamp_type = inbuf[8];
		DeviceConfigPara.lamp_power = (((u16)inbuf[9]) << 8) + (u16)inbuf[10];
		DeviceConfigPara.lamp_pf = (((u16)inbuf[11]) << 8) + (u16)inbuf[12];

		memset(DeviceConfigPara.longitude,'0',10);
		memcpy(DeviceConfigPara.longitude,inbuf + 13,10);

		memset(DeviceConfigPara.latitude,'0',10);
		memcpy(DeviceConfigPara.latitude,inbuf + 23,10);

		WriteDataFromMemoryToEeprom(inbuf,
		                            CONFIG_INFO_ADD,
		                            CONFIG_INFO_LEN - 2);

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置电源接口
u16 SetPowerInterface(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) <= INTFC_DALI)
		{
			PowerInterface = *(inbuf + 0);

			WriteDataFromMemoryToEeprom(inbuf,
			                            POWER_INTER_ADD,
			                            POWER_INTER_LEN - 2);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//对时
u16 SyncDateTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 6)
	{
		RTC_Set(*(inbuf + 0) + 2000,
		        *(inbuf + 1),
		        *(inbuf + 2),
		        *(inbuf + 3),
		        *(inbuf + 4),
		        *(inbuf + 5));

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//即时调光控制
u16 SetLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) <= 100)
		{
			LightLevelPercent = *(inbuf + 0);

//			WriteDataFromMemoryToEeprom(inbuf,
//			                            LIGHT_LEVEL_ADD,
//			                            LIGHT_LEVEL_LEN - 2);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置开关灯年表
u16 SetLampsSwitchProject(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u16 k = 0;
	u8 buf[2];
	u16 days = 0;
	u8 *msg = NULL;

	buf[0] = ctrl_code;
	buf[1] = 1;

	days = (((u16)(*(inbuf + 2))) << 8) + (u16)(*(inbuf + 3));

	if(data_len == (2 + 2 + days * 4))
	{
		if(days <= 366)
		{
			if(*(inbuf + 0) >= 1 && *(inbuf + 0) <= 12 &&
			   *(inbuf + 1) >= 1 && *(inbuf + 1) <= 31)
			{
				LampsSwitchProject.start_month = *(inbuf + 0);
				LampsSwitchProject.start_date  = *(inbuf + 1);

				LampsSwitchProject.total_days = days;

				WriteDataFromMemoryToEeprom(inbuf + 0,
											SWITCH_DATE_DAYS_ADD,
											SWITCH_DATE_DAYS_LEN - 2);	//将数据写入EEPROM

				msg = inbuf + 4;

				for(k = 0; k < LampsSwitchProject.total_days; k ++)
				{
					LampsSwitchProject.switch_time[k].on_time[0] = *(msg + k * 4 + 0);
					LampsSwitchProject.switch_time[k].on_time[1] = *(msg + k * 4 + 1);
					LampsSwitchProject.switch_time[k].off_time[0] = *(msg + k * 4 + 2);
					LampsSwitchProject.switch_time[k].off_time[1] = *(msg + k * 4 + 3);

					WriteDataFromMemoryToEeprom(msg + k * (SWITCH_TIME_LEN - 2) + 0,
												SWITCH_TIME_ADD + k * SWITCH_TIME_LEN,
												SWITCH_TIME_LEN - 2);	//将数据写入EEPROM
				}

				buf[1] = 0;
			}
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置调光策略
u16 SetTimeStrategy(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];
	u8 group_num = 0;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u8 time_group[TIME_STRATEGY_LEN * MAX_GROUP_NUM];
	u16 crc16 = 0;

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len % (TIME_STRATEGY_LEN - 2) == 0)			//数据长度必须是10的倍数
	{
		group_num = data_len / (TIME_STRATEGY_LEN - 2);	//计算下发了几组数据

		if(group_num <= MAX_GROUP_NUM)					//组数要小于MAX_GROUP_NUM
		{
			RemoveAllStrategy();						//删除所有本地存储策略

			memset(time_group,0,TIME_STRATEGY_LEN * MAX_GROUP_NUM);

			for(i = 0; i < group_num; i ++)
			{
				for(j = i * TIME_STRATEGY_LEN;
				    j < i * TIME_STRATEGY_LEN + (TIME_STRATEGY_LEN - 2);
				    j ++, k ++)
				{
					time_group[j] = *(inbuf + k);
				}

				crc16 = GetCRC16(&time_group[j - (TIME_STRATEGY_LEN - 2)],
				                 TIME_STRATEGY_LEN - 2);

				time_group[j + 0] = (u8)(crc16 >> 8);
				time_group[j + 1] = (u8)(crc16 & 0x00FF);
			}

			for(i = 0; i < group_num; i ++)
			{
				pRegularTime tmp_time = NULL;

				tmp_time = (pRegularTime)mymalloc(sizeof(RegularTime_S));

				tmp_time->prev = NULL;
				tmp_time->next = NULL;

				tmp_time->number 		= i;
				tmp_time->type 			= time_group[i * TIME_STRATEGY_LEN + 0];
				tmp_time->year 			= time_group[i * TIME_STRATEGY_LEN + 1];
				tmp_time->month 		= time_group[i * TIME_STRATEGY_LEN + 2];
				tmp_time->date 			= time_group[i * TIME_STRATEGY_LEN + 3];
				tmp_time->hour 			= time_group[i * TIME_STRATEGY_LEN + 4];
				tmp_time->minute 		= time_group[i * TIME_STRATEGY_LEN + 5];
				tmp_time->percent		= time_group[i * TIME_STRATEGY_LEN + 6];

				switch(tmp_time->type)
				{
					case TYPE_WEEKDAY:
						RegularTimeGroupAdd(TYPE_WEEKDAY,tmp_time);
					break;

					case TYPE_HOLIDAY_START:
						RegularTimeGroupAdd(TYPE_HOLIDAY_START,tmp_time);
					break;

					case TYPE_HOLIDAY_END:
						RegularTimeGroupAdd(TYPE_HOLIDAY_END,tmp_time);
					break;

					default:

					break;
				}
			}

			for(i = 0; i < group_num * TIME_STRATEGY_LEN + group_num; i ++)				//每组7个字节+2个字节(CRC16)
			{
				AT24CXX_WriteOneByte(TIME_STRATEGY_ADD + i,time_group[i]);
			}

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//获取终端信息
u16 GetDeviceInfo(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[91];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = 113 - (NB_ModulePara.csq * 2);
		buf[1] = NB_ModulePara.band;
		buf[2] = (u8)(NB_ModulePara.pci >> 8);
		buf[3] = (u8)(NB_ModulePara.pci & 0x00FF);
		buf[4] = (u8)(NB_ModulePara.rsrp >> 8);
		buf[5] = (u8)(NB_ModulePara.rsrp & 0x00FF);
		buf[6] = (u8)(NB_ModulePara.snr >> 8);
		buf[7] = (u8)(NB_ModulePara.snr & 0x00FF);
		memcpy(buf + 8,DeviceInfo.iccid,20);
		memcpy(buf + 28,DeviceInfo.imei,15);
		memcpy(buf + 43,DeviceInfo.imsi,15);
		memcpy(buf + 58,DeviceConfigPara.longitude,10);
		memcpy(buf + 68,DeviceConfigPara.latitude,10);
		buf[78] = (u8)(DeviceInfo.protocol_ver >> 8);
		buf[79] = (u8)(DeviceInfo.protocol_ver & 0x00FF);
		buf[80] = (u8)(DeviceInfo.hardware_ver >> 8);
		buf[81] = (u8)(DeviceInfo.hardware_ver & 0x00FF);
		memcpy(buf + 82,DeviceInfo.hardware_release_date,3);
		buf[85] = (u8)(DeviceInfo.software_ver >> 8);
		buf[86] = (u8)(DeviceInfo.software_ver & 0x00FF);
		memcpy(buf + 87,DeviceInfo.software_release_date,3);

		len = 90;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//下发固件升级指令
u16 SetFrameWareInfo(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 i = 0;
	u8 buf[2];

	buf[0] = 1;
	buf[1] = 0;

	if(data_len == 6)
	{
		if(*(inbuf + 0) <= 100)
		{
			FrameWareInfo.version = (((u16)(*(inbuf + 0))) << 8) + (u16)(*(inbuf + 1));

			FrameWareInfo.length = (((u32)(*(inbuf + 2))) << 24) +
								   (((u32)(*(inbuf + 3))) << 16) +
								   (((u32)(*(inbuf + 4))) << 8) +
								   (((u32)(*(inbuf + 5))) << 0);

			WriteDataFromMemoryToEeprom(inbuf + 0,
										SOFT_WARE_INFO_ADD,
										SOFT_WARE_INFO_LEN - 2);	//将数据写入EEPROM

			if(FrameWareInfo.version == DeviceInfo.software_ver)
			{
				buf[0] = 0;
				buf[1] = 1;
			}
			else if(FrameWareInfo.length > FIRMWARE_SIZE)
			{
				buf[0] = 0;
				buf[1] = 2;
			}
			else
			{
				u16 page_num = 0;

				FrameWareState.state 			= FIRMWARE_DOWNLOADING;
				FrameWareState.total_bags 		= FrameWareInfo.length % FIRMWARE_BAG_SIZE != 0 ?
												  FrameWareInfo.length / FIRMWARE_BAG_SIZE + 1 : FrameWareInfo.length / FIRMWARE_BAG_SIZE;
				FrameWareState.current_bag_cnt 	= 1;
				FrameWareState.bag_size 		= FIRMWARE_BAG_SIZE;
				FrameWareState.last_bag_size 	= FrameWareInfo.length % FIRMWARE_BAG_SIZE != 0 ?
												  FrameWareInfo.length % FIRMWARE_BAG_SIZE : FIRMWARE_BAG_SIZE;
				FrameWareState.total_size 		= FrameWareInfo.length;

				WriteFrameWareStateToEeprom();	//将固件升级状态写入EEPROM

				page_num = (FIRMWARE_MAX_FLASH_ADD - FIRMWARE_BUCKUP_FLASH_BASE_ADD) / 2048;	//得到备份区的扇区总数

				FLASH_Unlock();						//解锁FLASH

				for(i = 0; i < page_num; i ++)
				{
					FLASH_ErasePage(i * 2048 + FIRMWARE_BUCKUP_FLASH_BASE_ADD);	//擦除当前FLASH扇区
				}

				FLASH_Lock();						//上锁

				buf[0] = 1;
				buf[1] = 0;
			}
		}
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,2,outbuf);

	return ret;
}

//设置上电默认亮度
u16 SetDefaultLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 1)
	{
		if(*(inbuf + 0) <= 100)
		{
			DefaultLightLevelPercent = *(inbuf + 0);

			WriteDataFromMemoryToEeprom(inbuf,
			                            HARD_WARE_REL_DATE_ADD,
			                            HARD_WARE_REL_DATE_ADD - 2);

			buf[1] = 0;
		}
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置错峰时间
u16 SetPeakStaggerTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 4)
	{

		RandomPeakStaggerTime = (((u16)inbuf[0]) << 8) + (u16)inbuf[1];
		FixedPeakStaggerTime = (((u16)inbuf[2]) << 8) + (u16)inbuf[3];

		WriteDataFromMemoryToEeprom(inbuf,
									PEAK_STAGGER_TIME_ADD,
									PEAK_STAGGER_TIME_ADD - 2);

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置故障检测延时参数
u16 SetFaultDetectDelayPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 5)
	{
		EventDetectConf.comm_falut_detect_interval 		= *(inbuf + 0);
		EventDetectConf.router_fault_detect_interval	= *(inbuf + 1);
		EventDetectConf.turn_on_collect_delay 			= *(inbuf + 2);
		EventDetectConf.turn_off_collect_delay 			= *(inbuf + 3);
		EventDetectConf.current_detect_delay 			= *(inbuf + 4);

		WriteDataFromMemoryToEeprom(inbuf + 0,
									ER_TIME_CONF_ADD,
									ER_TIME_CONF_LEN - 2);	//将数据写入EEPROM

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//设置故障检测延时参数
u16 SetFaultDetectThrePara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];

	buf[0] = ctrl_code;
	buf[1] = 1;

	if(data_len == 16)
	{
		EventDetectConf.over_current_ratio 			= *(inbuf + 0);
		EventDetectConf.over_current_recovery_ratio = *(inbuf + 1);
		EventDetectConf.low_current_ratio 			= *(inbuf + 2);
		EventDetectConf.low_current_recovery_ratio 	= *(inbuf + 3);

		memcpy(EventDetectConf.capacitor_fault_pf_ratio,inbuf + 4,2);
		memcpy(EventDetectConf.capacitor_fault_recovery_pf_ratio,inbuf + 6,2);

		EventDetectConf.lamps_over_current_ratio 			= *(inbuf + 8);
		EventDetectConf.lamps_over_current_recovery_ratio 	= *(inbuf + 9);
		EventDetectConf.fuse_over_current_ratio 			= *(inbuf + 10);
		EventDetectConf.fuse_over_current_recovery_ratio 	= *(inbuf + 11);
		EventDetectConf.leakage_over_current_ratio 			= *(inbuf + 12);
		EventDetectConf.leakage_over_current_recovery_ratio = *(inbuf + 13);
		EventDetectConf.leakage_over_voltage_ratio 			= *(inbuf + 14);
		EventDetectConf.leakage_over_voltage_recovery_ratio = *(inbuf + 15);

		WriteDataFromMemoryToEeprom(inbuf + 0,
									ER_THRE_CONF_ADD,
									ER_THRE_CONF_LEN - 2);	//将数据写入EEPROM

		buf[1] = 0;
	}

	ret = PackAckPacket(cmd_id,0x80,buf,2,outbuf);

	return ret;
}

//获取重发配置参数
u16 GetReUpLoadPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = RspTimeOut;
		buf[1] = ReUpLoadTimes;

		len = 2;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取心跳周期
u16 GetHeartBeatUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = (u8)(HeartBeatUploadInterval >> 8);
		buf[1] = (u8)(HeartBeatUploadInterval & 0x00FF);

		len = 2;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取心跳周期
u16 GetDataUploadInterval(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[2];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = (u8)(DataUploadInterval >> 8);
		buf[1] = (u8)(DataUploadInterval & 0x00FF);

		len = 2;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取工作模式
u16 GetDeviceWorkMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[1];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = DeviceWorkMode;

		len = 1;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取开关灯模式
u16 GetSwitchMode(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[1];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = SwitchMode;

		len = 1;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取光照度阈值
u16 GetIlluminanceThreshold(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[4];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = (u8)(IlluminanceThreshold.on >> 8);
		buf[1] = (u8)(IlluminanceThreshold.on & 0x00FF);
		buf[2] = (u8)(IlluminanceThreshold.off >> 8);
		buf[3] = (u8)(IlluminanceThreshold.off & 0x00FF);

		len = 4;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取单灯配置信息
u16 GetDeviceConfigPara(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[33];
	u16 len = 0;

	if(data_len == 0)
	{
		memcpy(buf + 0,DeviceConfigPara.mail_add,8);
		buf[8] = DeviceConfigPara.lamp_type;
		buf[9] = (u8)(DeviceConfigPara.lamp_power >> 8);
		buf[10] = (u8)(DeviceConfigPara.lamp_power & 0x00FF);
		buf[11] = (u8)(DeviceConfigPara.lamp_pf >> 8);
		buf[12] = (u8)(DeviceConfigPara.lamp_pf & 0x00FF);
		memcpy(buf + 13,DeviceConfigPara.longitude,10);
		memcpy(buf + 23,DeviceConfigPara.latitude,10);

		len = 33;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取电源控制接口
u16 GetPowerInterface(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[1];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = PowerInterface;

		len = 1;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取当前亮度值
u16 GetLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[1];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = LightLevelPercent;

		len = 1;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取开关灯年表
u16 GetLampsSwitchProject(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u16 k = 0;
	u8 buf[1];
	u16 days = 0;
	s16 temp = 0;
	u16 len = 0;

	if(data_len == 4)
	{
		days = ((((u16)(*(inbuf + 2))) << 8) + (u16)(*(inbuf + 3)));	//要查询的时间天数

		temp = get_dates_diff(LampsSwitchProject.start_month,LampsSwitchProject.start_date,*(inbuf + 0),*(inbuf + 1));

		if(temp >= 0)													//要查询的日期 是第几个开关
		{
			if(temp + days <= LampsSwitchProject.total_days)			//总天数要小于等于之前设置的总天数
			{
				memcpy(buf + 0,inbuf,4);

				for(k = 0; k < days; k ++)
				{
					buf[4 + k * 4 + 0] = LampsSwitchProject.switch_time[temp + k].on_time[0];
					buf[4 + k * 4 + 1] = LampsSwitchProject.switch_time[temp + k].on_time[1];
					buf[4 + k * 4 + 2] = LampsSwitchProject.switch_time[temp + k].off_time[0];
					buf[4 + k * 4 + 3] = LampsSwitchProject.switch_time[temp + k].off_time[1];
				}
			}
		}

		len = 1 + 1 + 2 + days * 4;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取调光策略
u16 GetTimeStrategy(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u16 i = 0;
	u16 j = 0;
	u16 read_crc = 0;
	u16 cal_crc = 0;
	u8 buf[TIME_STRATEGY_LEN * MAX_GROUP_NUM];
	u8 read_success_buf_flag[MAX_GROUP_NUM];
	u16 len = 0;

	if(data_len == 0)
	{
		for(i = 0; i < MAX_GROUP_NUM; i ++)
		{
			for(j = i * TIME_STRATEGY_LEN; j < i * TIME_STRATEGY_LEN + TIME_STRATEGY_LEN; j ++)
			{
				buf[j] = AT24CXX_ReadOneByte(TIME_STRATEGY_ADD + j);
			}

			cal_crc = GetCRC16(&buf[j - TIME_STRATEGY_LEN],TIME_STRATEGY_LEN - 2);
			read_crc = (((u16)buf[j - 2]) << 8) + (u16)buf[j - 1];

			if(cal_crc == read_crc)
			{
				read_success_buf_flag[i] = 1;
			}
		}

		for(i = 0; i < MAX_GROUP_NUM; i ++)
		{
			if(read_success_buf_flag[i] == 1)
			{
				memcpy(buf + i * (TIME_STRATEGY_LEN - 2),buf + i * TIME_STRATEGY_LEN,TIME_STRATEGY_LEN - 2);
			}
			else
			{
				break;
			}
		}

		len = i * (TIME_STRATEGY_LEN - 2);
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取当前亮度值
u16 GetDefaultLightLevelPercent(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[1];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = DefaultLightLevelPercent;

		len = 1;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取采集数据
u16 GetSensorData(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[50];
	u16 len = 0;

	if(data_len == 0)
	{
		len = UnPackSensorData(p_tSensorMsg,buf);
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取错峰时间
u16 GetPeakStaggerTime(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[4];
	u16 len = 0;

	if(data_len == 0)
	{
		buf[0] = (u8)(RandomPeakStaggerTime >> 8);
		buf[1] = (u8)(RandomPeakStaggerTime & 0x00FF);
		buf[2] = (u8)(FixedPeakStaggerTime >> 8);
		buf[3] = (u8)(FixedPeakStaggerTime & 0x00FF);

		len = 4;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}

//获取故障事件命令
u16 GetFaultEvents(u16 cmd_id,u8 ctrl_code,u8 *inbuf,u16 data_len,u8 *outbuf)
{
	u16 ret = 0;
	u8 buf[512];
	u16 len = 0;
	u8 temp0 = 0;
	u16 temp6 = 0;
	u8 temp_buf[64];
	u16 i = 0;

	if(data_len == 2)
	{
		buf[0] = EventRecordList.ec1;
		buf[1] = *(inbuf + 0);
		buf[2] = *(inbuf + 1);

		temp6 = 0;	//每一个事件之间的偏移地址

		if(*(inbuf + 0) < *(inbuf + 1))
		{
			for(i = *(inbuf + 0); i < *(inbuf + 1); i ++)
			{
				memset(temp_buf,0,32);

				switch(EventRecordList.lable1[*(inbuf + i) - 1])
				{
					case 15:
						temp0 = 14;	//当前事件所占内存长度
					break;

					case 16:
						temp0 = 14;
					break;

					case 17:
						temp0 = 12;
					break;

					case 18:
						temp0 = 12;
					break;

					case 19:
						temp0 = 14;
					break;

					case 20:
						temp0 = 14;
					break;

					case 21:
						temp0 = 16;
					break;

					case 22:
						temp0 = 16;
					break;

					case 23:
						temp0 = 16;
					break;

					case 28:
						temp0 = 21;
					break;

					case 36:
						temp0 = 15;
					break;

					case 37:
						temp0 = 11;
					break;

					case 51:
						temp0 = 11;
					break;

					case 52:
						temp0 = 20;
					break;

					default:
					break;
				}

				if(temp0 != 0)
				{
					ret = ReadDataFromEepromToMemory(temp_buf,E_IMPORTEAT_ADD + i * EVENT_LEN,EVENT_LEN);		//从EEPROM中读取时间内容

					if(ret == 1)
					{
						memcpy(&buf[3 + temp6],temp_buf,temp0);			//读取成功 将时间内容放入数据单元
					}
					else
					{
						memset(&buf[3 + temp6],0,temp0);					//读取失败 将数据单元清空
					}

					temp6 += temp0;

					temp0 = 0;
				}
			}
		}
		else
		{
			for(i = 0; i < *(inbuf + 1); i ++)
			{
				memset(temp_buf,0,32);

				switch(EventRecordList.lable1[*(inbuf + i) - 1])
				{
					case 15:
						temp0 = 14;
					break;

					case 16:
						temp0 = 14;
					break;

					case 17:
						temp0 = 12;
					break;

					case 18:
						temp0 = 12;
					break;

					case 19:
						temp0 = 15;
					break;

					case 20:
						temp0 = 15;
					break;

					case 21:
						temp0 = 17;
					break;

					case 22:
						temp0 = 17;
					break;

					case 23:
						temp0 = 17;
					break;

					case 28:
						temp0 = 21;
					break;

					case 36:
						temp0 = 16;
					break;

					case 37:
						temp0 = 11;
					break;

					case 51:
						temp0 = 17;
					break;

					case 52:
						temp0 = 22;
					break;

					default:
					break;
				}

				if(temp0 != 0)
				{
					ret = ReadDataFromEepromToMemory(temp_buf,E_IMPORTEAT_ADD + i * EVENT_LEN,EVENT_LEN);		//从EEPROM中读取时间内容

					if(ret == 1)
					{
						memcpy(&buf[3 + temp6],temp_buf,temp0);			//读取成功 将时间内容放入数据单元
					}
					else
					{
						memset(&buf[3 + temp6],0,temp0);					//读取失败 将数据单元清空
					}

					temp6 += temp0;

					temp0 = 0;
				}
			}

			for(i = *(inbuf + 0); i <= 255; i ++)
			{
				memset(temp_buf,0,32);

				switch(EventRecordList.lable1[*(inbuf + i) - 1])
				{
					case 15:
						temp0 = 14;
					break;

					case 16:
						temp0 = 14;
					break;

					case 17:
						temp0 = 12;
					break;

					case 18:
						temp0 = 12;
					break;

					case 19:
						temp0 = 15;
					break;

					case 20:
						temp0 = 15;
					break;

					case 21:
						temp0 = 17;
					break;

					case 22:
						temp0 = 17;
					break;

					case 23:
						temp0 = 17;
					break;

					case 28:
						temp0 = 21;
					break;

					case 36:
						temp0 = 16;
					break;

					case 37:
						temp0 = 11;
					break;

					case 51:
						temp0 = 17;
					break;

					case 52:
						temp0 = 22;
					break;

					default:
					break;
				}

				if(temp0 != 0)
				{
					ret = ReadDataFromEepromToMemory(temp_buf,E_IMPORTEAT_ADD + i * EVENT_LEN,EVENT_LEN);		//从EEPROM中读取时间内容

					if(ret == 1)
					{
						memcpy(&buf[3 + temp6],temp_buf,temp0);			//读取成功 将时间内容放入数据单元
					}
					else
					{
						memset(&buf[3 + temp6],0,temp0);					//读取失败 将数据单元清空
					}

					temp6 += temp0;

					temp0 = 0;
				}
			}
		}

		len = temp6 + 3;
	}

	ret = PackAckPacket(cmd_id,ctrl_code,buf,len,outbuf);

	return ret;
}





































