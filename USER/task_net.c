#include "task_net.h"
#include "common.h"
#include "delay.h"
#include "net_protocol.h"
#include "rtc.h"
#include "uart4.h"


TaskHandle_t xHandleTaskNET = NULL;
SensorMsg_S *p_tSensorMsgNet = NULL;			//����װ�ڴ��������ݵĽṹ�����

CONNECT_STATE_E ConnectState = UNKNOW_STATE;
u8 LoginState = 0;								//�豸��¼״̬

u32 LoginStaggeredPeakInterval = 0;				//��¼��Ϣ��������
u32 UploadDataStaggeredPeakInterval = 0;		//�����ϱ���������
u32 HeartBeatStaggeredPeakInterval = 0;			//�����ϱ���������


unsigned portBASE_TYPE NET_Satck;
void vTaskNET(void *pvParameters)
{
	s8 ret = 0;
	u8 err_cnt = 0;
	time_t times_sec = 0;

	p_tSensorMsgNet = (SensorMsg_S *)mymalloc(sizeof(SensorMsg_S));

	if(RandomPeakStaggerTime != 0)		//ʹ��������ʱ��
	{
		LoginStaggeredPeakInterval 		= LOGIN_OUT_TIMEOUT 		+ rand() % RandomPeakStaggerTime;
		UploadDataStaggeredPeakInterval = DataUploadInterval 		+ rand() % RandomPeakStaggerTime;
		HeartBeatStaggeredPeakInterval 	= HeartBeatUploadInterval 	+ rand() % RandomPeakStaggerTime;
	}
	else								//ʹ�ù̶����ʱ��
	{
		LoginStaggeredPeakInterval 		= LOGIN_OUT_TIMEOUT 		+ FixedPeakStaggerTime;
		UploadDataStaggeredPeakInterval = DataUploadInterval 		+ FixedPeakStaggerTime;
		HeartBeatStaggeredPeakInterval 	= HeartBeatUploadInterval 	+ FixedPeakStaggerTime;
	}
	
//	/*******************����Ϊ���Դ���******************/
//	RTC_Set(2019,7,20,17,33,0);	
//	/***************************************************/
	
	bcxx_hard_init();

	RE_INIT:

	ConnectState = UNKNOW_STATE;
	LoginState = 0;
	err_cnt = 0;

	bcxx_soft_init();

	while(1)
	{
		if(GetSysTick1s() - times_sec >= 30)			//ÿ��30���ӻ�ȡһ���ź�ǿ��
		{
			times_sec = GetSysTick1s();
			bcxx_get_AT_CSQ(&NB_ModulePara.csq);
			bcxx_get_AT_NUESTATS(&NB_ModulePara.rsrp,
                                 &NB_ModulePara.rssi,
                                 &NB_ModulePara.snr,
                                 &NB_ModulePara.pci,
                                 &NB_ModulePara.rsrq);

			SyncDataTimeFormM53xxModule(3600);

			if(ConnectState == UNKNOW_STATE)
			{
				if((err_cnt ++) >= 30)
				{
					goto RE_INIT;
				}
			}
			else
			{
				err_cnt = 0;
			}
		}

		ret = OnServerHandle();

		if(ret != 1)
		{
			goto RE_INIT;
		}

		delay_ms(100);
	}
}


//���ߴ������
s8 OnServerHandle(void)
{
	s8 ret = 1;
	s16 len = 0;
	u8 outbuf[NET_BUF_MAX_LEN];
	char *str_buf = NULL;

	len = NetDataFrameHandle(outbuf);				//��ȡ�������������·������ݰ�,�����ȼ�

	if(len <= 0)
	{
		len = SendEventRequestToServer(outbuf);		//������������¼�����,�����ȼ�
	}

	if(len >= 1)									//��������Ҫ����
	{
		str_buf = (char *)mymalloc(sizeof(char) * (len + 1) * 2);

		HexToStr(str_buf,outbuf,len);

		ret = bcxx_set_AT_QLWULDATA(len,str_buf);

		myfree(str_buf);
	}

	return ret;
}

//���������ʱ���ʹ��������ݺ�������
s16 SendEventRequestToServer(u8 *outbuf)
{
	static time_t times_sec1 = 0;
	static time_t times_sec2 = 0;
	static u16 download_time_out = 0;
	static u8  download_failed_times = 0;
	u16 send_len = 0;

	if(LoginState == 0 && ConnectState == ON_SERVER)
	{
		if(GetSysTick1s() - times_sec2 >= LoginStaggeredPeakInterval)
		{
			times_sec2 = GetSysTick1s();

			if(RandomPeakStaggerTime != 0)	//ʹ��������ʱ��
			{
				LoginStaggeredPeakInterval 	= LOGIN_OUT_TIMEOUT + rand() % RandomPeakStaggerTime;
			}

			send_len = CombineLogin_outFrame(1,outbuf);
		}
	}
	else if(GetSysTick1s() - times_sec1 >= UploadDataStaggeredPeakInterval)
	{
		times_sec1 = GetSysTick1s();
		times_sec2 = GetSysTick1s();

		if(RandomPeakStaggerTime != 0)		//ʹ��������ʱ��
		{
			UploadDataStaggeredPeakInterval = DataUploadInterval + rand() % RandomPeakStaggerTime;
		}

		send_len = CombineSensorDataFrame(outbuf);
	}
	else if(GetSysTick1s() - times_sec2 >= HeartBeatStaggeredPeakInterval)
	{
		times_sec2 = GetSysTick1s();

		if(RandomPeakStaggerTime != 0)		//ʹ��������ʱ��
		{
			HeartBeatStaggeredPeakInterval = HeartBeatUploadInterval + rand() % RandomPeakStaggerTime;
		}

		send_len = CombineHeartBeatFrame(outbuf);
	}
	else if(FrameWareState.state == FIRMWARE_DOWNLOADING)
	{
		download_time_out = 0;											//�̼����ȴ���ʱ
		download_failed_times = 0;										//�̼�������ʧ�ܴ���
		FrameWareState.state = FIRMWARE_DOWNLOAD_WAIT;					//�ȴ���ǰ�̼���

		send_len = CombineRequestFrameWareFrame(outbuf);
	}
	else if(FrameWareState.state == FIRMWARE_DOWNLOAD_WAIT)
	{
		if((download_time_out ++) >= 300)
		{
			if((download_failed_times ++) >= 15)
			{
				FrameWareState.state = FIRMWARE_DOWNLOAD_FAILED;	//�ж�Ϊ�̼�����ʧ��
			}
			else
			{
				FrameWareState.state = FIRMWARE_DOWNLOADING;		//�������ص�ǰ�̼���
			}
		}
	}
//	else if(EventRecordList.important_event_flag == 1)
//	{
//		EventRecordList.important_event_flag = 0;
//		
//		send_len = CombineFaultEventFrame(outbuf);
//	}

	return (s16)send_len;
}

//��ָ����NTP��������ȡʱ��
u8 SyncDataTimeFormM53xxModule(time_t sync_cycle)
{
	u8 ret = 0;
	struct tm tm_time;
	static time_t time_s = 0;
	static time_t time_c = 0;
	char buf[32];

	if((GetSysTick1s() - time_c >= sync_cycle) || GetTimeOK != 1)
	{
		time_c = GetSysTick1s();

		memset(buf,0,32);

		if(bcxx_get_AT_CCLK(buf))
		{
			tm_time.tm_year = 2000 + (buf[0] - 0x30) * 10 + buf[1] - 0x30 - 1900;
			tm_time.tm_mon = (buf[3] - 0x30) * 10 + buf[4] - 0x30 - 1;
			tm_time.tm_mday = (buf[6] - 0x30) * 10 + buf[7] - 0x30;

			tm_time.tm_hour = (buf[9] - 0x30) * 10 + buf[10] - 0x30;
			tm_time.tm_min = (buf[12] - 0x30) * 10 + buf[13] - 0x30;
			tm_time.tm_sec = (buf[15] - 0x30) * 10 + buf[16] - 0x30;

			time_s = mktime(&tm_time);

			time_s += 28800;

			SyncTimeFromNet(time_s);

			GetTimeOK = 1;

			ret = 1;
		}
	}

	return ret;
}






























