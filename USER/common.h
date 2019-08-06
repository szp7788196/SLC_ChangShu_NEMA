#ifndef __COMMON_H
#define __COMMON_H

#include "stm32f10x.h"
#include "string.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <ctype.h>
#include "malloc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"

#include <time.h>

#include "task_sensor.h"
#include "rtc.h"
#include "platform.h"

/*---------------------------------------------------------------------------*/
/* Type Definition Macros                                                    */
/*---------------------------------------------------------------------------*/
#ifndef __WORDSIZE
  /* Assume 32 */
  #define __WORDSIZE 32
#endif

    typedef unsigned char   uint8;
    typedef char            int8;
    typedef unsigned short  uint16;
    typedef short           int16;
    typedef unsigned int    uint32;
    typedef int             int32;

#ifdef WIN32
    typedef int socklen_t;
#endif

    typedef unsigned long long int  uint64;
    typedef long long int           int64;


#define FIRMWARE_FREE					0			//无需固件升级
#define FIRMWARE_DOWNLOADING			1			//固件正在下载中
#define FIRMWARE_DOWNLOAD_WAIT			2			//等待服务器下发固件
#define FIRMWARE_DOWNLOADED				3			//固件下载完成
#define FIRMWARE_DOWNLOAD_FAILED		4			//下载失败
#define FIRMWARE_UPDATING				5			//正在升级
#define FIRMWARE_UPDATE_SUCCESS			6			//升级成功
#define FIRMWARE_UPDATE_FAILED			7			//升级失败
#define FIRMWARE_ERASE_SUCCESS			8			//擦除FLASH成功
#define FIRMWARE_ERASE_FAIL				9			//擦除FLASH成功
#define FIRMWARE_ERASEING				10			//正在擦除FLASH
#define FIRMWARE_BAG_SIZE				258			//128 + 2字节crc
#define FIRMWARE_RUN_FLASH_BASE_ADD		0x08006000	//程序运行地址
#define FIRMWARE_BUCKUP_FLASH_BASE_ADD	0x08043000	//程序备份地址
#define FIRMWARE_MAX_FLASH_ADD			0x08080000	//FLSAH最大地址
#define FIRMWARE_SIZE					FIRMWARE_BUCKUP_FLASH_BASE_ADD - FIRMWARE_RUN_FLASH_BASE_ADD

#define DEVICE_TYPE					'B'			//设备类型

#define RELEASE_VERSION							//正式发布版本

//#define DEBUG_LOG								//是否打印调试信息

#define NEW_BOARD

#define CHRONOLOGY_MODE				1			//年表模式
#define POSOTION_MODE				2			//经纬度模式
#define ILLUMINATION_MODE			3			//光照度模式
#define ALWAYS_ON					4			//常亮模式

#define INTFC_0_10V					0			//0~10V调光
#define INTFC_PWM					1			//PWM调光
#define INTFC_DIGIT					2			//数字调光
#define INTFC_DALI					3			//DALI调光

#define MAX_GROUP_NUM				100			//(255 - 11 - 6 - 36) / 7

#define MAX_UPLOAD_INVL				65535

#define INIT_LIGHT_LEVEL			LightLevelPercent

#define TYPE_WEEKDAY				0x01
#define TYPE_WEEKEND				0x02
#define TYPE_HOLIDAY_START			0x04
#define TYPE_HOLIDAY_END			0x14

#define MODE_AUTO					0
#define MODE_MANUAL					1

#define LAMPS_SWITCH_MAX_DAYS		366

#define IMEI_LEN					15
#define IMSI_LEN					15
#define ICCID_LEN					20



#define EVENT_ERC15					15
#define EVENT_ERC16					16
#define EVENT_ERC17					17
#define EVENT_ERC18					18
#define EVENT_ERC19					19
#define EVENT_ERC20					20
#define EVENT_ERC21					21
#define EVENT_ERC22					22
#define EVENT_ERC23					23
#define EVENT_ERC28					28
#define EVENT_ERC36					36
#define EVENT_ERC37					37
#define EVENT_ERC51					51
#define EVENT_ERC52					52

#define MAX_EVENT_NUM				256



#define RE_UPLOAD_PARA_ADD			0		//重发参数
#define RE_UPLOAD_PARA_LEN			4

#define DATA_UPLOAD_INTER_ADD		4		//数据上传间隔
#define DATA_UPLOAD_INTER_LEN		4

#define HEART_BEAT_UPLOAD_INTER_ADD	8		//心跳上传间隔
#define HEART_BEAT_UPLOAD_INTER_LEN	4

#define SWITCH_MODE_ADD				12		//开关灯模式
#define SWITCH_MODE_LEN				3

#define POWER_INTER_ADD				15		//电源接口
#define POWER_INTER_LEN				3

#define DEFAULT_LIGHT_LEVEL_ADD		18		//上电默认亮度
#define DEFAULT_LIGHT_LEVEL_LEN		3

#define CONFIG_INFO_ADD				21		//配置信息
#define CONFIG_INFO_LEN				35

#define SOFT_WARE_VER_ADD			56		//软件版本
#define SOFT_WARE_VER_LEN			4

#define SOFT_WARE_REL_DATE_ADD		60		//软件发布日期
#define SOFT_WARE_REL_DATE_LEN		5

#define HARD_WARE_VER_ADD			65		//硬件版本
#define HARD_WARE_VER_LEN			4

#define HARD_WARE_REL_DATE_ADD		69		//硬件发布日期
#define HARD_WARE_REL_DATE_LEN		5

#define PROTOCOL_VER_ADD			74		//协议版本
#define PROTOCOL_VER_LEN			4

#define ILLUM_THER_ADD				78		//光照度阈值
#define ILLUM_THER_LEN				6

#define SOFT_WARE_INFO_ADD			85		//固件信息
#define SOFT_WARE_INFO_LEN			8

#define UPDATE_STATE_ADD			93		//升级状态
#define UPDATE_STATE_LEN			15

#define SERVER_IP_ADD				108		//服务器IP存储地址
#define SERVER_IP_LEN				18

#define SERVER_PORT_ADD				124		//服务器端口号存储地址
#define SERVER_PORT_LEN				8

#define PEAK_STAGGER_TIME_ADD		134		//错峰时间存储地址
#define PEAK_STAGGER_TIME_LEN		6

#define SWITCH_DATE_DAYS_ADD		256		//开关灯起始日期和天数
#define SWITCH_DATE_DAYS_LEN		6
#define SWITCH_TIME_ADD				262		//具体开关灯时间 共366组
#define SWITCH_TIME_LEN				6

#define TIME_STRATEGY_ADD			2501	//定时策略
#define	TIME_STRATEGY_LEN			9


#define ER_TIME_CONF_ADD			3501	//事件记录时间参数配置
#define ER_TIME_CONF_LEN			7

#define ER_THRE_CONF_ADD			3508	//事件记录时间参数配置
#define ER_THRE_CONF_LEN			18

#define EC1_ADD						3526	//重要事件计数器EC1
#define EC1_LEN						3

#define EC1_LABLE_ADD				3532	//重要事件标签数组
#define EC1_LABLE_LEN				130

#define E_IMPORTANT_FLAG_ADD		3662	//重要事件标志
#define E_IMPORTANT_FLAG_LEN		3

#define E_IMPORTEAT_ADD				3701	//重要事件记录SOE
#define EVENT_LEN					24



#define HolodayRange_S struct HolodayRange
typedef struct HolodayRange *pHolodayRange;
struct HolodayRange
{
	u8 year_s;
	u8 month_s;
	u8 date_s;
	
	u8 year_e;
	u8 month_e;
	u8 date_e;
};

#define RegularTime_S struct RegularTime
typedef struct RegularTime *pRegularTime;
struct RegularTime
{
	u8 number;
	u8 type;			//策略类别Bit0:1 工作日 Bit1:1 周末 Bit2:1节日

	u8 year;
	u8 month;
	u8 date;
	u8 hour;
	u8 minute;

	HolodayRange_S range;
	
	u8 percent;

	pRegularTime prev;
	pRegularTime next;
};


typedef struct DeviceConfigPara				//设备基本信息
{
	u8 mail_add[8];							//终端地址(通信地址)
	u8 lamp_type;							//灯具类型
	u16 lamp_power;							//灯具功率
	u16 lamp_pf;								//灯具功率因数
	u8 longitude[10];						//经度
	u8 latitude[10];						//纬度
}DeviceConfigPara_S;

typedef struct IlluminanceThreshold			//光照度开关阈值
{
	u16 on;									//开灯阈值
	u16 off;								//关灯阈值
}IlluminanceThreshold_S;

typedef struct SwitchTime					//开关灯时间
{
	u8 on_time[2];							//开灯时间
	u8 off_time[2];							//关灯时间
}SwitchTime_S;

typedef struct LampsSwitchProject						//每天开关灯
{
	u8 start_month;										//开始月
	u8 start_date;										//开始日
	u16 total_days;										//有效总天数
	
	SwitchTime_S switch_time[LAMPS_SWITCH_MAX_DAYS];	//每天的开关灯时间
}LampsSwitchProject_S;

typedef struct NB_ModulePara				//NB模块参数
{
	u8 csq;									//通信状态质量
	u8 band;								//频段
	s16 pci;								//基站
	s16 rsrp;								//信号强度
	s16 rsrq;								//参考信号接收质量
	s16 rssi;								//接收信号强度等级
	s16 snr;								//信噪比
}NB_ModulePara_S;

typedef struct DeviceInfo					//设备信息
{
	u16 software_ver;						//终端软件版本号
	u8 software_release_date[3];			//终端软件发布日期
	u16 hardware_ver;						//终端硬件版本号
	u8 hardware_release_date[3];			//终端硬件发布日期
	u16 protocol_ver;						//终端通讯协议版本号
	u8 *iccid;								//iccid
	u8 *imsi;								//imsi
	u8 *imei;								//imei
}DeviceInfo_S;

typedef struct FrameWareInfo				//FTP升级固件信息
{
	u16 version;							//固件版本
	u32 length;								//固件大小
}FrameWareInfo_S;

typedef struct FrameWareState				//固件升级状态信息
{
	u8 state;								//当前状态
	u16 total_bags;							//总包数
	u16 current_bag_cnt;					//当前下载的包数
	u16 bag_size;							//每包大小
	u16 last_bag_size;						//最末包大小
	u32 total_size;							//固件大小
	
}FrameWareState_S;

typedef struct EventDetectConf				//事件记录配置设置数据
{
	u8 comm_falut_detect_interval;			//终端通信故障检测时间间隔
	u8 router_fault_detect_interval;		//集中器路由板故障检测时间间隔
	u8 turn_on_collect_delay;				//单灯正常开灯采集延时
	u8 turn_off_collect_delay;				//单灯正常关灯采集延时
	u8 current_detect_delay;				//单灯电流检测延时时间
	
	u8 over_current_ratio;					//单灯电流过大事件电流限值比值
	u8 over_current_recovery_ratio;			//单灯电流过大事件恢复电流限值比值
	u8 low_current_ratio;					//单灯电流过小事件电流限值比值
	u8 low_current_recovery_ratio;			//单灯电流过小事件恢复电流限值比值
	u8 capacitor_fault_pf_ratio[2];			//单灯电容故障事件故障功率因数限值
	u8 capacitor_fault_recovery_pf_ratio[2];//单灯电容故障事件故障恢复功率因数限值
	u8 lamps_over_current_ratio;			//单灯灯具故障事件电流限值
	u8 lamps_over_current_recovery_ratio;	//单灯灯具故障事件恢复电流限值
	u8 fuse_over_current_ratio;				//单灯熔丝故障事件电流限值
	u8 fuse_over_current_recovery_ratio;	//单灯熔丝故障事件恢复电流限值
	u8 leakage_over_current_ratio;			//单灯漏电故障事件电流限值
	u8 leakage_over_current_recovery_ratio;	//单灯漏电故障事件恢复电流限值
	u8 leakage_over_voltage_ratio;			//单灯漏电故障事件电压限值
	u8 leakage_over_voltage_recovery_ratio;	//单灯漏电故障事件恢复电压限值
}EventDetectConf_S;

typedef struct EventRecordList				//事件记录表
{
	u8 ec1;									//重要事件计数器
	u8 lable1[128];							//重要事件标签
	
	u8 important_event_flag;				//系统中有重要事件标志
}EventRecordList_S;



extern SemaphoreHandle_t  xMutex_IIC1;			//IIC1的互斥量
extern SemaphoreHandle_t  xMutex_INVENTR;		//英飞特电源的互斥量
extern SemaphoreHandle_t  xMutex_AT_COMMAND;	//AT指令的互斥量
extern SemaphoreHandle_t  xMutex_STRATEGY;		//AT指令的互斥量
extern SemaphoreHandle_t  xMutex_EVENT_RECORD;	//事件记录的互斥量
extern SemaphoreHandle_t  xMutex_SYSYICK_1S;	//事件记录的互斥量

extern QueueHandle_t xQueue_sensor;				//用于存储传感器的数据


extern pRegularTime RegularTimeWeekDay;			//工作日策略
extern pRegularTime RegularTimeWeekEnd;			//周末策略
extern pRegularTime RegularTimeHoliday;			//节假日策略
extern HolodayRange_S HolodayRange;				//节假日起始日期

/***************************系统心跳相关*****************************/
extern u32 SysTick1ms;					//1ms滴答时钟
extern u32 SysTick10ms;					//10ms滴答时钟
extern u32 SysTick100ms;				//10ms滴答时钟
extern time_t SysTick1s;				//1s滴答时钟

/***********************MCU厂商唯一序列号*****************************/
extern u8 *UniqueChipID;

/***************************网络相关*********************************/
extern u8 *ServerIP;					//服务器IP地址
extern u8 *ServerPort;					//服务器端口号
extern u8 *LocalIp;						//本地IP地址

/***************************运行参数相关*****************************/
extern u8 WaitRspCtrlCode;				//等待平台响应的控制码
extern u8 WaittingRsp;					//等待平台响应标志
extern u8 ResetFlag;					//系统复位标志
extern u16 DataUploadInterval;			//数据上传时间间隔0~65535秒
extern u16 HeartBeatUploadInterval;		//心跳上传时间间隔0~65535秒
extern u16 RandomPeakStaggerTime;		//随机错峰时间0~65535秒
extern u16 FixedPeakStaggerTime;		//固定错峰时间0~65535秒
extern u8 PowerInterface;				//电源控制接口编号 0:0~10V 1:PWM 2:UART 3:DALI
extern u8 DefaultLightLevelPercent;		//灯的上电默认亮度级别
extern u16 RspTimeOut;					//响应超时时间
extern u8 ReUpLoadTimes;				//超时重发次数
extern u8 DeviceWorkMode;				//运行模式，0：自动，1：手动
extern u8 SwitchMode;					//开关灯模式
extern u8 SwitchState;					//开关状态
extern u8 LightLevelPercent;			//灯的亮度级别

extern float FaultInputCurrent;			//发生故障时的电流
extern float FaultInputVoltage;			//发生故障时的电压

extern u8 CalendarClock[6];

extern EventDetectConf_S EventDetectConf;
extern EventRecordList_S EventRecordList;

/***************************其他*****************************/
extern u8 GetTimeOK;								//成功获取时间标志

extern DeviceConfigPara_S DeviceConfigPara;			//设备基本信息
extern LampsSwitchProject_S LampsSwitchProject;		//年表
extern NB_ModulePara_S NB_ModulePara;				//NB模块参数
extern DeviceInfo_S DeviceInfo;						//设备信息
extern FrameWareInfo_S FrameWareInfo;				//固件信息
extern FrameWareState_S FrameWareState;				//固件升级状态
extern IlluminanceThreshold_S IlluminanceThreshold;	//光照度阈值


u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len);
u8 GetDatBit(u32 dat);
u32 GetADV(u8 len);
void IntToString(u8 *DString,u32 Dint,u8 zero_num);
u32 StringToInt(u8 *String);
void HexToStr(char *pbDest, u8 *pbSrc, u16 len);
void StrToHex(u8 *pbDest, char *pbSrc, u16 len);
int base64_encode(const unsigned char * bindata, char * base64, int binlength);
int base64_decode(const char * base64, unsigned char * bindata);
unsigned short find_str(unsigned char *s_str, unsigned char *p_str, unsigned short count, unsigned short *seek);
int search_str(unsigned char *source, unsigned char *target);
unsigned short get_str1(unsigned char *source, unsigned char *begin, unsigned short count1, unsigned char *end, unsigned short count2, unsigned char *out);
unsigned short get_str2(unsigned char *source, unsigned char *begin, unsigned short count, unsigned short length, unsigned char *out);
unsigned short get_str3(unsigned char *source, unsigned char *out, unsigned short length);
u16 GetCRC16(u8 *data,u16 len);
u32 CRC32(const u8 *buf, u32 size, u32 temp,u8 flag);
u8 CalCheckSum(u8 *buf, u16 len);
u8 GetSysTimeState(void);
u16 get_day_num(u8 m,u8 d);
void get_date_from_days(u16 days, u8 *m, u8 *d);
s16 get_dates_diff(u8 m1,u8 d1,u8 m2,u8 d2);
u8 leap_year_judge(u16 year);
u32 get_days_form_calendar(u16 year,u8 month,u8 date);

void SysTick1msAdder(void);
u32 GetSysTick1ms(void);
void SysTick10msAdder(void);
u32 GetSysTick10ms(void);
void SysTick100msAdder(void);
u32 GetSysTick100ms(void);
void SetSysTick1s(time_t sec);
time_t GetSysTick1s(void);

u8 STMFLASH_ReadByte(u32 faddr);
void STMFLASH_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 NumToRead);

u8 ReadDataFromEepromToMemory(u8 *buf,u16 s_add, u16 len);
void WriteDataFromMemoryToEeprom(u8 *inbuf,u16 s_add, u16 len);
u8 GetMemoryForSpecifyPointer(u8 **str,u16 size, u8 *memory);

u8 ReadEventDetectTimeConf(void);
u8 ReadEventDetectThreConf(void);
u8 ReadEventRecordList(void);
u8 ReadUpCommPortPara(void);
u8 ReadDataUploadInterval(void);
u8 ReadHeartBeatUploadInterval(void);
u8 ReadPeakStaggerTime(void);
u8 ReadSwitchMode(void);
u8 ReadIlluminanceThreshold(void);
u8 ReadPowerInterface(void);
u8 ReadLightLevelPercent(void);
u8 ReadDeviceConfigPara(void);
u8 ReadSoftWareVersion(void);
u8 ReadSoftWareReleaseDate(void);
u8 ReadHardWareVersion(void);
u8 ReadHardWareReleaseDate(void);
u8 ReadProtocolVer(void);
u8 ReadFrameWareInfo(void);
void WriteFrameWareStateToEeprom(void);
u8 UpdateSoftWareVer(void);
u8 UpdateSoftWareReleaseDate(void);
u8 ReadFrameWareState(void);
u8 ReadServerIP(void);
u8 ReadServerPort(void);
u8 ReadLampsSwitchProject(void);
u8 ReadRegularTimeGroups(void);

void ReadParametersFromEEPROM(void);


u16 PackUserData(u8 ctrl_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf);
u16 PackCommandRspData(u16 cmd_id,u16 err_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf);
u16 PackEventUploadData(u8 *inbuf,u16 inbuf_len,u8 *outbuf);
u16 UnPackSensorData(SensorMsg_S *msg,u8 *buf);

u8 RegularTimeGroupAdd(u8 type,pRegularTime group_time);
u8 RegularTimeGroupSub(u8 number);
void RemoveAllStrategy(void);































#endif
