#include "task_hci.h"
#include "delay.h"
#include "usart.h"
#include "inventr.h"
#include "at_protocol.h"


TaskHandle_t xHandleTaskHCI = NULL;
unsigned portBASE_TYPE HCI_Satck;
void vTaskHCI(void *pvParameters)
{
	u16 send_len1 = 0;
	u16 send_len2 = 0;
	u16 msg_len = 0;
//	u8 buff[7] = {'\r','\n',0,0,0,'\r','\n'};

//	AT_CommandInit();

//	printf("READY\r\n");

	while(1)
	{
		if(Usart1RecvEnd == 0xAA)
		{
			Usart1RecvEnd = 0;

//			send_len1 = AT_CommandDataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf,HoldReg);

			memset(Usart1TxBuf,0,Usart1TxLen);

			send_len1 = HCI_DataAnalysis(Usart1RxBuf,Usart1FrameLen,Usart1TxBuf,&msg_len);

			memset(Usart1RxBuf,0,Usart1FrameLen);
		}

		if(Usart2RecvEnd == 0xAA && InventrBusy == 0)
		{
			Usart2RecvEnd = 0;

//			send_len2 = AT_CommandDataAnalysis(Usart2RxBuf,Usart2FrameLen,Usart2TxBuf,HoldReg);

			memset(Usart2RxBuf,0,Usart2FrameLen);
		}

		if(send_len1 != 0)
		{
			printf("%s",Usart1TxBuf);
			
//			buff[2] = msg_len / 100 + 0x30;
//			buff[3] = (msg_len / 10) % 10 + 0x30;
//			buff[4] = msg_len % 10 + 0x30;
//			UsartSendString(USART1,buff, 7);
//			UsartSendString(USART1,Usart1TxBuf, send_len1);

			send_len1 = 0;
		}
		if(send_len2 != 0)
		{
			UsartSendString(USART2,Usart2TxBuf, send_len2);

			memset(Usart2TxBuf,0,send_len2);

			send_len2 = 0;
		}

		delay_ms(100);

		HCI_Satck = uxTaskGetStackHighWaterMark(NULL);
	}
}

//人机交互数据解析
//u16 HCI_DataAnalysis(u8 *inbuf,u16 inbuf_len,u8 *outbuf)
//{
//	u16 ret = 0;
//	char temp_buf[32];
//
//	if(inbuf_len == 9)
//	{
//		if(MyStrstr(inbuf, "AT+IMEI", inbuf_len, 7) != 0xFFFF)
//		{
//			sprintf((char*)outbuf,"{");
//
//			memset(temp_buf,0,32);
//			memcpy(temp_buf,DeviceInfo.iccid,20);
//			strcat((char*)outbuf,"\"ICCID\":\"");
//			strcat((char*)outbuf,temp_buf);
//			strcat((char*)outbuf,"\",");
//
//			memset(temp_buf,0,32);
//			memcpy(temp_buf,DeviceInfo.imei,15);
//			strcat((char*)outbuf,"\"IMEI\":\"");
//			strcat((char*)outbuf,temp_buf);
//			strcat((char*)outbuf,"\",");
//
//			memset(temp_buf,0,32);
//			memcpy(temp_buf,DeviceInfo.imsi,15);
//			strcat((char*)outbuf,"\"IMSI\":\"");
//			strcat((char*)outbuf,temp_buf);
//
//			strcat((char*)outbuf,"}");
//
//			ret = strlen((char *)outbuf);
//		}
//	}
//
//	return ret;
//}

u16 HCI_DataAnalysis(u8 *inbuf,u16 inbuf_len,u8 *outbuf,u16 *msg_len)
{
	u16 ret = 0;
	u8 buf[CONFIG_INFO_LEN];
	char temp_buf[32];
//	u8 temp_buf1[512];

	if(MyStrstr(inbuf, "AT+ADD=", inbuf_len, 7) != 0xFFFF)
	{
		if(inbuf_len == 25)
		{
			memset(buf,'0',CONFIG_INFO_LEN);

			StrToHex(buf, (char*)inbuf + 7, 8);

//			if(buf[0] == 0x00 &&
//			   buf[1] == 0x00 &&
//			   buf[2] == 0x00 &&
//			   buf[3] <= 0x99 &&
//			   buf[4] <= 0x99 &&
//			   buf[5] <= 0x99 &&
//			   buf[6] <= 0x99 &&
//			   buf[7] == 0x01)
			{
				memcpy(DeviceConfigPara.mail_add,buf + 0,8);

				memcpy(buf + 0,DeviceConfigPara.mail_add,8);
				buf[8] = DeviceConfigPara.lamp_type;
				buf[9] = (u8)(DeviceConfigPara.lamp_power >> 8);
				buf[10] = (u8)(DeviceConfigPara.lamp_power & 0x00FF);
				buf[11] = (u8)(DeviceConfigPara.lamp_pf >> 8);
				buf[12] = (u8)(DeviceConfigPara.lamp_pf & 0x00FF);
				memcpy(buf + 13,DeviceConfigPara.longitude,10);
				memcpy(buf + 23,DeviceConfigPara.latitude,10);

				WriteDataFromMemoryToEeprom(buf,
											CONFIG_INFO_ADD,
											CONFIG_INFO_LEN - 2);	//存入EEPROM

				memset(temp_buf,0,32);
				HexToStr((char*)temp_buf,&buf[0],8);
				sprintf((char*)outbuf,"{\"LTUAddr\":\"%s\",",temp_buf);

				strcat((char*)outbuf,"\"No\":\"");
				memset(temp_buf,0,32);
				HexToStr(temp_buf,&buf[2],6);
				strcat((char*)outbuf,temp_buf);
				strcat((char*)outbuf,"\",");

				strcat((char*)outbuf,"\"Type\":\"20\",");
				strcat((char*)outbuf,"\"Provider\":\"LK\",");

				memset(temp_buf,0,32);
				memcpy(temp_buf,DeviceInfo.iccid,20);
				strcat((char*)outbuf,"\"ICCID\":\"");
				strcat((char*)outbuf,temp_buf);
				strcat((char*)outbuf,"\",");

				memset(temp_buf,0,32);
				memcpy(temp_buf,DeviceInfo.imei,15);
				strcat((char*)outbuf,"\"IMEI\":\"");
				strcat((char*)outbuf,temp_buf);
				strcat((char*)outbuf,"\",");

				memset(temp_buf,0,32);
				memcpy(temp_buf,DeviceInfo.imsi,15);
				strcat((char*)outbuf,"\"IMSI\":\"");
				strcat((char*)outbuf,temp_buf);
				strcat((char*)outbuf,"\"}");

				ret = strlen((char *)outbuf);
			}
		}
	}
//	else
//	{
//		if(*(inbuf + 0) == 0xAA)
//		{
//			ret = PackUserData(*(inbuf + 1),inbuf + 2,inbuf_len - 2,temp_buf1);
//		
//			if(ret)
//			{
//				*msg_len = ret;
//				
//				ret = (u16)base64_encode(temp_buf1, (char *)outbuf, ret);
//			}
//		}
//		else if(*(inbuf + 0) == 0x55)
//		{
//			ret = (u16)base64_decode((char *)(inbuf + 1), outbuf);
//		}
//	}

	return ret;
}



































