#include "common.h"
#include "24cxx.h"
#include "bcxx.h"

pRegularTime RegularTimeWeekDay = NULL;			//工作日策略
pRegularTime RegularTimeWeekEnd = NULL;			//周末策略
pRegularTime RegularTimeHoliday = NULL;			//节假日策略
HolodayRange_S HolodayRange;					//节假日起始日期

/****************************互斥量相关******************************/
SemaphoreHandle_t  xMutex_IIC1 			= NULL;	//IIC总线1的互斥量
SemaphoreHandle_t  xMutex_INVENTR 		= NULL;	//英飞特电源的互斥量
SemaphoreHandle_t  xMutex_AT_COMMAND 	= NULL;	//AT指令的互斥量
SemaphoreHandle_t  xMutex_STRATEGY 		= NULL;	//AT指令的互斥量
SemaphoreHandle_t  xMutex_EVENT_RECORD 	= NULL;	//事件记录的互斥量
SemaphoreHandle_t  xMutex_SYSYICK_1S 	= NULL;	//事件记录的互斥量

/***************************消息队列相关*****************************/
QueueHandle_t xQueue_sensor 		= NULL;	//用于存储传感器的数据


/***************************系统心跳相关*****************************/
u32 SysTick1ms = 0;					//1ms滴答时钟
u32 SysTick10ms = 0;				//10ms滴答时钟
u32 SysTick100ms = 0;				//10ms滴答时钟
time_t SysTick1s = 0;				//1s滴答时钟


/***************************网络相关*********************************/
u8 *ServerIP = NULL;				//服务器IP地址
u8 *ServerPort = NULL;				//服务器端口号
u8 *LocalIp = NULL;					//本地IP地址

/***************************运行参数相关*****************************/
u8 WaitRspCtrlCode = 0;				//等待平台响应的控制码
u8 WaittingRsp = 0;					//等待平台响应标志
u8 ResetFlag = 0;					//系统复位标志
u16 RspTimeOut = 0;					//响应超时时间
u8 ReUpLoadTimes = 0;				//超时重发次数
u16 DataUploadInterval = 3600;		//数据上传时间间隔0~65535秒
u16 HeartBeatUploadInterval = 1800;	//心跳上传时间间隔0~65535秒
u16 RandomPeakStaggerTime = 60;		//随机错峰时间0~65535秒
u16 FixedPeakStaggerTime = 0;		//固定错峰时间0~65535秒
u8 DeviceWorkMode = 0;				//运行模式，0：自动，1：手动
u8 SwitchMode = ALWAYS_ON;			//开关灯模式
u8 SwitchState = 1;					//开关状态
u8 LightLevelPercent = 100;			//灯的亮度级别
u8 PowerInterface = 0;				//电源控制接口编号 0:0~10V 1:PWM 2:UART 3:DALI
u8 DefaultLightLevelPercent = 100;	//灯的上电默认亮度级别

float FaultInputCurrent = 0.0f;		//发生故障时的电流
float FaultInputVoltage = 0.0f;		//发生故障时的电压

u8 CalendarClock[6];

EventDetectConf_S EventDetectConf;
EventRecordList_S EventRecordList;



/***************************其他*****************************/
u8 GetTimeOK = 0;							//成功获取时间标志

DeviceConfigPara_S DeviceConfigPara;		//设备基本信息
LampsSwitchProject_S LampsSwitchProject;	//年表
NB_ModulePara_S NB_ModulePara;				//NB模块参数
DeviceInfo_S DeviceInfo;					//设备信息
FrameWareInfo_S FrameWareInfo;				//固件信息
FrameWareState_S FrameWareState;			//固件升级状态
IlluminanceThreshold_S IlluminanceThreshold;//光照度阈值



static const uint32_t crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL 
};


//在str1中查找str2，失败返回0xFF,成功返回str2首个元素在str1中的位置
u16 MyStrstr(u8 *str1, u8 *str2, u16 str1_len, u16 str2_len)
{
	u16 len = str1_len;
	if(str1_len == 0 || str2_len == 0)
	{
		return 0xFFFF;
	}
	else
	{
		while(str1_len >= str2_len)
		{
			str1_len --;
			if (!memcmp(str1, str2, str2_len))
			{
				return len - str1_len - 1;
			}
			str1 ++;
		}
		return 0xFFFF;
	}
}

//获得整数的位数
u8 GetDatBit(u32 dat)
{
	u8 j = 1;
	u32 i;
	i = dat;
	while(i >= 10)
	{
		j ++;
		i /= 10;
	}
	return j;
}

//用个位数换算出一个整数 1 10 100 1000......
u32 GetADV(u8 len)
{
	u32 count = 1;
	if(len == 1)
	{
		return 1;
	}
	else
	{
		len --;
		while(len --)
		{
			count *= 10;
		}
	}
	return count;
}

//整数转换为字符串
void IntToString(u8 *DString,u32 Dint,u8 zero_num)
{
	u16 i = 0;
	u8 j = GetDatBit(Dint);
	for(i = 0; i < GetDatBit(Dint) + zero_num; i ++)
	{
		DString[i] = Dint / GetADV(j) % 10 + 0x30;
		j --;
	}
}

u32 StringToInt(u8 *String)
{
	u8 len;
	u8 i;
	u32 count=0;
	u32 dev;

	len = strlen((char *)String);
	dev = 1;
	for(i = 0; i < len; i ++)//len-1
	{
		if(String[i] != '.')
		{
			count += ((String[i] - 0x30) * GetADV(len) / dev);
			dev *= 10;
		}
		else
		{
			len --;
			count /= 10;
		}
	}
	if(String[i]!=0x00)
	{
		count += (String[i] - 0x30);
	}
	return count;
}

/*
// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 存放目标字符串
// [IN] pbSrc - 输入16进制数的起始地址
// [IN] nLen - 16进制数的字节数
// return value:
// remarks : 将16进制数转化为字符串
*/
void HexToStr(char *pbDest, u8 *pbSrc, u16 len)
{
	char ddl,ddh;
	int i;

	for (i = 0; i < len; i ++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i * 2] = ddh;
		pbDest[i * 2 + 1] = ddl;
	}

	pbDest[len * 2] = '\0';
}

/*
// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - 输出缓冲区
// [IN] pbSrc - 字符串
// [IN] nLen - 16进制数的字节数(字符串的长度/2)
// return value:
// remarks : 将字符串转化为16进制数
*/
void StrToHex(u8 *pbDest, char *pbSrc, u16 len)
{
	char h1,h2;
	u8 s1,s2;
	int i;

	for (i = 0; i < len; i ++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
		s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
		s2 -= 7;

		pbDest[i] = s1 * 16 + s2;
	}
}



int base64_encode(const unsigned char * bindata, char * base64, int binlength)
{
    int i, j;
    unsigned char current;
	const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (i = 0, j = 0 ; i < binlength ; i += 3)
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j ++] = base64char[(int)current];

        current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30) ;
        if (i + 1 >= binlength)
        {
            base64[j ++] = base64char[(int)current];
            base64[j ++] = '=';
            base64[j ++] = '=';

            break;
        }
        current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char) 0x0F);
        base64[j ++] = base64char[(int)current];

        current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C) ;
        if (i + 2 >= binlength)
        {
            base64[j ++] = base64char[(int)current];
            base64[j ++] = '=';

            break;
        }
        current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char) 0x03);
        base64[j ++] = base64char[(int)current];

        current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F) ;
        base64[j ++] = base64char[(int)current];
    }

    base64[j] = '\0';

    return j;
}

int base64_decode(const char * base64, unsigned char * bindata)
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
	const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    for (i = 0,j = 0; base64[i] != '\0'; i += 4)
    {
        memset(temp, 0xFF, sizeof(temp));
        for (k = 0; k < 64; k ++)
        {
            if (base64char[k] == base64[i])
                temp[0] = k;
        }
        for (k = 0; k < 64; k ++)
        {
            if (base64char[k] == base64[i + 1])
                temp[1] = k;
        }
        for (k = 0; k < 64; k ++)
        {
            if (base64char[k] == base64[i + 2])
                temp[2] = k;
        }
        for (k = 0; k < 64; k ++)
        {
            if (base64char[k] == base64[i + 3])
                temp[3] = k;
        }

        bindata[j ++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
        if ( base64[i + 2] == '=' )
            break;

        bindata[j ++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
                ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
        if ( base64[i + 3] == '=' )
            break;

        bindata[j ++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
                ((unsigned char)(temp[3] & 0x3F));
    }

    return j;
}

unsigned short find_str(unsigned char *s_str, unsigned char *p_str, unsigned short count, unsigned short *seek)
{
	unsigned short _count = 1;
    unsigned short len = 0;
    unsigned char *temp_str = NULL;
    unsigned char *temp_ptr = NULL;
    unsigned char *temp_char = NULL;

	(*seek) = 0;
    if(0 == s_str || 0 == p_str)
        return 0;
    for(temp_str = s_str; *temp_str != '\0'; temp_str++)
    {
        temp_char = temp_str;

        for(temp_ptr = p_str; *temp_ptr != '\0'; temp_ptr++)
        {
            if(*temp_ptr != *temp_char)
            {
                len = 0;
                break;
            }
            temp_char++;
            len++;
        }
        if(*temp_ptr == '\0')
        {
            if(_count == count)
                return len;
            else
            {
                _count++;
                len = 0;
            }
        }
        (*seek) ++;
    }
    return 0;
}

int search_str(unsigned char *source, unsigned char *target)
{
	unsigned short seek = 0;
    unsigned short len;
    len = find_str(source, target, 1, &seek);
    if(len == 0)
        return -1;
    else
        return len;
}

unsigned short get_str1(unsigned char *source, unsigned char *begin, unsigned short count1, unsigned char *end, unsigned short count2, unsigned char *out)
{
	unsigned short i;
    unsigned short len1;
    unsigned short len2;
    unsigned short index1 = 0;
    unsigned short index2 = 0;
    unsigned short length = 0;
    len1 = find_str(source, begin, count1, &index1);
    len2 = find_str(source, end, count2, &index2);
    length = index2 - index1 - len1;
    if((len1 != 0) && (len2 != 0))
    {
        for( i = 0; i < index2 - index1 - len1; i++)
            out[i] = source[index1 + len1 + i];
        out[i] = '\0';
    }
    else
    {
        out[0] = '\0';
    }
    return length;
}

unsigned short get_str2(unsigned char *source, unsigned char *begin, unsigned short count, unsigned short length, unsigned char *out)
{
	unsigned short i = 0;
    unsigned short len1 = 0;
    unsigned short index1 = 0;
    len1 = find_str(source, begin, count, &index1);
    if(len1 != 0)
    {
        for(i = 0; i < length; i++)
            out[i] = source[index1 + len1 + i];
        out[i] = '\0';
    }
    else
    {
        out[0] = '\0';
    }
    return length;
}

unsigned short get_str3(unsigned char *source, unsigned char *out, unsigned short length)
{
	unsigned short i = 0;
    for (i = 0 ; i < length ; i++)
    {
        out[i] = source[i];
    }
    out[i] = '\0';
    return length;
}

/*****************************************************
函数：u16 CRC16(u8 *puchMsgg,u8 usDataLen)
功能：CRC校验用函数
参数：puchMsgg是要进行CRC校验的消息，usDataLen是消息中字节数 mode:0 低字节在前 1:高字节在前
返回：计算出来的CRC校验码。
*****************************************************/
u16 GetCRC16(u8 *data,u16 len)
{
	u16 ax,lsb;
	u8 temp1 = 0;
	u8 temp2 = 0;
	int i,j;

	ax = 0xFFFF;

	for(i = 0; i < len; i ++)
	{
		ax ^= data[i];

		for(j = 0; j < 8; j ++)
		{
			lsb = ax & 0x0001;
			ax = ax >> 1;

			if(lsb != 0)
				ax ^= 0xA001;
		}
	}

	temp1 = (u8)((ax >> 8) & 0x00FF);
	temp2 = (u8)(ax & 0x00FF);

	ax = ((((u16)temp2) << 8) & 0xFF00) + (u16)temp1;

	return ax;
}

//CRC32
u32 CRC32(const u8 *buf, u32 size, u32 temp,u8 flag)
{
	uint32_t i, crc,crc_e;
	
	crc = temp;
	for (i = 0; i < size; i++)
	{
		crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
	}
	
	if(flag != 0)
	{
		crc_e = crc^0xFFFFFFFF;
	}
	else if(flag == 0)
	{
		crc_e = crc;
	}
	return crc_e;
}

//计算校验和
u8 CalCheckSum(u8 *buf, u16 len)
{
	u8 sum = 0;
	u16 i = 0;

	for(i = 0; i < len; i ++)
	{
		sum += *(buf + i);
	}

	return sum;
}

//获取系统时间状态
u8 GetSysTimeState(void)
{
	u8 ret = 0;

	if(calendar.w_year >= 2019)
	{
		ret = 2;
	}

	return ret;
}

//输入日期获得此日期在一年中的第几天 按照闰年计算
u16 get_day_num(u8 m,u8 d)
{
	u8 i = 0;
	u8 x[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};
	u16 s=0;

	for(i = 1; i < m; i ++)
	{
		s += x[i];			//整月的天数
	}

	s += (u16)d;			//日的天数

	return s;				//返回总天数,相对公元1年
}

//已知天数计算日期
void get_date_from_days(u16 days, u8 *m, u8 *d)
{
	u8 i = 0;
	u8 x[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};
	u16 s=0;

	for(i = 0; i < 12; i ++)
	{
		s += x[i];

		if(days > s && days <= s + x[i + 1])
		{
			*m = i + 1;
			*d = days - s;
		}
	}
}

//获取同一年中 两个日期之间差的天数 m1 d1为起始日期
s16 get_dates_diff(u8 m1,u8 d1,u8 m2,u8 d2)
{
	s16 diff = 0;
	u16 sum1;
	u16 sum2;

	sum1 = get_day_num(m1,d1);
	sum2 = get_day_num(m2,d2);

	if(sum2 < sum1)
	{
		diff = -1;
	}
	else
	{
		diff = sum2 - sum1;
	}

	return diff;
}

//闰年判断
u8 leap_year_judge(u16 year)
{
	u16 leap = 0;

	if(year % 400 == 0)
	{
		leap = 1;
	}
    else
    {
        if(year % 4 == 0 && year % 100 != 0)
		{
			leap = 1;
		}
        else
		{
			leap = 0;
		}
	}

	return leap;
}

//闰年判断 返回当前年月日 在一年中的天数
u32 get_days_form_calendar(u16 year,u8 month,u8 date)
{
	u16 i = 0;
	u8 leap = 0;
	u32 days = 0;
	u8 x[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};

	for(i = 2000; i <= year; i ++)
	{
		leap = leap_year_judge(i);

		if(leap == 1)
		{
			days += 366;
		}
		else if(leap == 0)
		{
			days += 365;
		}
	}

	leap = leap_year_judge(year);

	if(leap == 1)
	{
		x[2] = 29;
	}
	else if(leap == 0)
	{
		x[2] = 28;
	}

	for(i = 1; i < month; i ++)
	{
		days += x[i];			//整月的天数
	}

	days += (u16)date;			//日的天数

	return days;
}

//产生一个系统1毫秒滴答时钟.
void SysTick1msAdder(void)
{
	SysTick1ms = (SysTick1ms + 1) & 0xFFFFFFFF;
}

//获取系统1毫秒滴答时钟
u32 GetSysTick1ms(void)
{
	return SysTick1ms;
}

//产生一个系统10毫秒滴答时钟.
void SysTick10msAdder(void)
{
	SysTick10ms = (SysTick10ms + 1) & 0xFFFFFFFF;
}

//获取系统10毫秒滴答时钟
u32 GetSysTick10ms(void)
{
	return SysTick10ms;
}

//产生一个系统100毫秒滴答时钟.
void SysTick100msAdder(void)
{
	SysTick100ms = (SysTick100ms + 1) & 0xFFFFFFFF;
}

//获取系统100毫秒滴答时钟
u32 GetSysTick100ms(void)
{
	return SysTick1ms;
}

void SetSysTick1s(time_t sec)
{
	SysTick1s = sec;
}

//获取系统1秒滴答时钟
time_t GetSysTick1s(void)
{
	time_t sec = 0;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_SYSYICK_1S, portMAX_DELAY);
	}

	sec = SysTick1s;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_SYSYICK_1S);
	}

	return sec;
}

//在FLASH中的指定位置读取一个字节
u8 STMFLASH_ReadByte(u32 faddr)
{
	return *(vu8*)faddr;
}

//按字节读取FLASH指定地址
void STMFLASH_ReadBytes(u32 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	u16 i = 0;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadByte(ReadAddr);
		ReadAddr++;
	}
}

//从EEPROM中读取数据(带CRC16校验码)len包括CRC16校验码
u8 ReadDataFromEepromToMemory(u8 *buf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 ReadCrcCode;
	u16 CalCrcCode = 0;

	for(i = s_add,j = 0; i < s_add + len; i ++, j++)
	{
		*(buf + j) = AT24CXX_ReadOneByte(i);
	}

	ReadCrcCode = (u16)(*(buf + len - 2));
	ReadCrcCode = ReadCrcCode << 8;
	ReadCrcCode = ReadCrcCode | (u16)(*(buf + len - 1));

	CalCrcCode = GetCRC16(buf,len - 2);

	if(ReadCrcCode == CalCrcCode)
	{
		return 1;
	}

	return 0;
}

//向EEPROM中写入数据(带CRC16校验码)len不包括CRC16校验码
void WriteDataFromMemoryToEeprom(u8 *inbuf,u16 s_add, u16 len)
{
	u16 i = 0;
	u16 j = 0;
	u16 CalCrcCode = 0;

	CalCrcCode = GetCRC16(inbuf,len);

	for(i = s_add ,j = 0; i < s_add + len; i ++, j ++)			//写入原始数据
	{
		AT24CXX_WriteOneByte(i,*(inbuf + j));
	}

	AT24CXX_WriteOneByte(s_add + len + 0,(u8)(CalCrcCode >> 8));		//写入CRC
	AT24CXX_WriteOneByte(s_add + len + 1,(u8)(CalCrcCode & 0x00FF));
}

//将内存中的数据拷贝到指定指针所指的内存中
u8 GetMemoryForSpecifyPointer(u8 **str,u16 size, u8 *memory)
{
	u8 ret = 0;
	u8 len = 0;
	u8 new_len = 0;

	if(*str == NULL)
	{
		len = size;

		*str = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}

	if(*str != NULL)
	{
		len = strlen((char *)*str);

		new_len = size;

		if(len == new_len)
		{
			memset(*str,0,new_len + 1);

			memcpy(*str,memory,new_len);

			ret = 1;
		}
		else
		{
			myfree(*str);
			*str = (u8 *)mymalloc(sizeof(u8) * new_len + 1);

			if(*str != NULL)
			{
				memset(*str,0,new_len + 1);

				memcpy(*str,memory,new_len);

				len = new_len;
				new_len = 0;
				ret = 1;
			}
		}
	}

	return ret;
}

//将字符串拷贝到指定地址
u8 CopyStrToPointer(u8 **pointer, u8 *str, u8 len)
{
	u8 ret = 0;

	if(*pointer == NULL)
	{
		*pointer = (u8 *)mymalloc(len + 1);
	}
	else if(*pointer != NULL)
	{
		myfree(*pointer);
		*pointer = (u8 *)mymalloc(sizeof(u8) * len + 1);
	}

	if(*pointer != NULL)
	{
		memset(*pointer,0,len + 1);

		memcpy(*pointer,str,len);

		ret = 1;
	}

	return ret;
}

//终端事件检测配置时间参数
u8 ReadEventDetectTimeConf(void)
{
	u8 ret = 0;
	u8 buf[ER_TIME_CONF_LEN];

	memset(buf,0,ER_TIME_CONF_LEN);

	ret = ReadDataFromEepromToMemory(buf,ER_TIME_CONF_ADD,ER_TIME_CONF_LEN);

	if(ret == 1)
	{
		EventDetectConf.comm_falut_detect_interval 		= *(buf + 0);
		EventDetectConf.router_fault_detect_interval	= *(buf + 1);
		EventDetectConf.turn_on_collect_delay 			= *(buf + 2);
		EventDetectConf.turn_off_collect_delay 			= *(buf + 3);
		EventDetectConf.current_detect_delay 			= *(buf + 4);
	}
	else
	{
		EventDetectConf.comm_falut_detect_interval 		= 3;
		EventDetectConf.router_fault_detect_interval	= 3;
		EventDetectConf.turn_on_collect_delay 			= 1;
		EventDetectConf.turn_off_collect_delay 			= 1;
		EventDetectConf.current_detect_delay 			= 10;
	}

	return ret;
}

//终端事件检测配置阈值参数
u8 ReadEventDetectThreConf(void)
{
	u8 ret = 0;
	u8 buf[ER_THRE_CONF_LEN];

	memset(buf,0,ER_THRE_CONF_LEN);

	ret = ReadDataFromEepromToMemory(buf,ER_THRE_CONF_ADD,ER_THRE_CONF_LEN);

	if(ret == 1)
	{
		EventDetectConf.over_current_ratio 			= *(buf + 0);
		EventDetectConf.over_current_recovery_ratio = *(buf + 1);
		EventDetectConf.low_current_ratio 			= *(buf + 2);
		EventDetectConf.low_current_recovery_ratio 	= *(buf + 3);

		memcpy(EventDetectConf.capacitor_fault_pf_ratio,buf + 4,2);
		memcpy(EventDetectConf.capacitor_fault_recovery_pf_ratio,buf + 6,2);

		EventDetectConf.lamps_over_current_ratio 			= *(buf + 8);
		EventDetectConf.lamps_over_current_recovery_ratio 	= *(buf + 9);
		EventDetectConf.fuse_over_current_ratio 			= *(buf + 10);
		EventDetectConf.fuse_over_current_recovery_ratio 	= *(buf + 11);
		EventDetectConf.leakage_over_current_ratio 			= *(buf + 12);
		EventDetectConf.leakage_over_current_recovery_ratio = *(buf + 13);
		EventDetectConf.leakage_over_voltage_ratio 			= *(buf + 14);
		EventDetectConf.leakage_over_voltage_recovery_ratio = *(buf + 15);
	}
	else
	{
		EventDetectConf.over_current_ratio 			= 10;
		EventDetectConf.over_current_recovery_ratio = 5;
		EventDetectConf.low_current_ratio 			= 10;
		EventDetectConf.low_current_recovery_ratio 	= 5;

		EventDetectConf.capacitor_fault_pf_ratio[0] = 0x13;				//50  单位0.01
		EventDetectConf.capacitor_fault_pf_ratio[1] = 0x88;

		EventDetectConf.capacitor_fault_recovery_pf_ratio[0] = 0x1B;	//70  单位0.01
		EventDetectConf.capacitor_fault_recovery_pf_ratio[1] = 0x58;

		EventDetectConf.lamps_over_current_ratio 			= 50;
		EventDetectConf.lamps_over_current_recovery_ratio 	= 100;
		EventDetectConf.fuse_over_current_ratio 			= 50;
		EventDetectConf.fuse_over_current_recovery_ratio 	= 100;
		EventDetectConf.leakage_over_current_ratio 			= 100;
		EventDetectConf.leakage_over_current_recovery_ratio = 80;
		EventDetectConf.leakage_over_voltage_ratio 			= 60;
		EventDetectConf.leakage_over_voltage_recovery_ratio = 36;
	}

	return ret;
}

//事件记录表
u8 ReadEventRecordList(void)
{
	u8 ret = 0;
	u8 buf[EC1_LABLE_LEN];

	memset(buf,0,EC1_LABLE_LEN);

	ret = ReadDataFromEepromToMemory(buf,E_IMPORTANT_FLAG_ADD,E_IMPORTANT_FLAG_LEN);

	if(ret == 1)
	{
		EventRecordList.important_event_flag = buf[0];
	}
	else
	{
		EventRecordList.important_event_flag = 0;
	}

	ret = ReadDataFromEepromToMemory(buf,EC1_ADD,EC1_LEN);

	if(ret == 1)
	{
		EventRecordList.ec1 = buf[0];
	}
	else
	{
		EventRecordList.ec1 = 0;

		EventRecordList.important_event_flag = 0;

		WriteDataFromMemoryToEeprom(&EventRecordList.important_event_flag,E_IMPORTANT_FLAG_ADD, 1);

		WriteDataFromMemoryToEeprom(&EventRecordList.ec1,EC1_ADD, 1);

		WriteDataFromMemoryToEeprom(EventRecordList.lable1,EC1_LABLE_ADD, EC1_LABLE_LEN - 2);
	}

	ret = ReadDataFromEepromToMemory(buf,EC1_LABLE_ADD,EC1_LABLE_LEN);

	if(ret == 1)
	{
		memcpy(EventRecordList.lable1,buf,EC1_LABLE_LEN - 2);
	}
	else
	{
		EventRecordList.important_event_flag = 0;

		memset(EventRecordList.lable1,0,EC1_LABLE_LEN - 2);

		WriteDataFromMemoryToEeprom(&EventRecordList.important_event_flag,E_IMPORTANT_FLAG_ADD, 1);

		WriteDataFromMemoryToEeprom(EventRecordList.lable1,EC1_LABLE_ADD, EC1_LABLE_LEN - 2);

		WriteDataFromMemoryToEeprom(&EventRecordList.ec1,EC1_ADD, 1);
	}

	return ret;
}

//读取上行通讯口通讯参数
u8 ReadUpCommPortPara(void)
{
	u8 ret = 0;
	u8 buf[RE_UPLOAD_PARA_LEN];

	memset(buf,0,RE_UPLOAD_PARA_LEN);

	ret = ReadDataFromEepromToMemory(buf,RE_UPLOAD_PARA_ADD,RE_UPLOAD_PARA_LEN);

	if(ret == 1)
	{
		RspTimeOut = buf[0];
		ReUpLoadTimes = buf[1];

		if(ReUpLoadTimes > 3)
		{
			ReUpLoadTimes = 3;
		}
	}
	else
	{
		RspTimeOut = 0;
		ReUpLoadTimes = 0;
	}

	return ret;
}

//读取数据上传间隔
u8 ReadDataUploadInterval(void)
{
	u8 ret = 0;
	u8 buf[DATA_UPLOAD_INTER_LEN];

	memset(buf,0,DATA_UPLOAD_INTER_LEN);

	ret = ReadDataFromEepromToMemory(buf,DATA_UPLOAD_INTER_ADD,DATA_UPLOAD_INTER_LEN);

	if(ret == 1)
	{
		DataUploadInterval = (((u16)buf[0]) << 8) + (u16)buf[1];

		if(DataUploadInterval < 10)
		{
			DataUploadInterval = 3600;
		}
	}
	else
	{
		DataUploadInterval = 3600;
	}

	return ret;
}

//读取心跳上传间隔
u8 ReadHeartBeatUploadInterval(void)
{
	u8 ret = 0;
	u8 buf[HEART_BEAT_UPLOAD_INTER_LEN];

	memset(buf,0,HEART_BEAT_UPLOAD_INTER_LEN);

	ret = ReadDataFromEepromToMemory(buf,HEART_BEAT_UPLOAD_INTER_ADD,HEART_BEAT_UPLOAD_INTER_LEN);

	if(ret == 1)
	{
		HeartBeatUploadInterval = (((u16)buf[0]) << 8) + (u16)buf[1];

		if(HeartBeatUploadInterval < 10)
		{
			HeartBeatUploadInterval = 3600;
		}
	}
	else
	{
		HeartBeatUploadInterval = 3600;
	}

	return ret;
}

//读取错峰时间
u8 ReadPeakStaggerTime(void)
{
	u8 ret = 0;
	u8 buf[PEAK_STAGGER_TIME_LEN];

	memset(buf,0,PEAK_STAGGER_TIME_LEN);

	ret = ReadDataFromEepromToMemory(buf,PEAK_STAGGER_TIME_ADD,PEAK_STAGGER_TIME_LEN);

	if(ret == 1)
	{
		RandomPeakStaggerTime = (((u16)buf[0]) << 8) + (u16)buf[1];
		FixedPeakStaggerTime = (((u16)buf[2]) << 8) + (u16)buf[3];
	}
	else
	{
		RandomPeakStaggerTime = 60;
		FixedPeakStaggerTime = 0;
	}

	srand((unsigned)GetSysTick1s());

	return ret;
}

//读取开关灯模式
u8 ReadSwitchMode(void)
{
	u8 ret = 0;
	u8 buf[SWITCH_MODE_LEN];

	memset(buf,0,SWITCH_MODE_LEN);

	ret = ReadDataFromEepromToMemory(buf,SWITCH_MODE_ADD,SWITCH_MODE_LEN);

	if(ret == 1)
	{
		SwitchMode = buf[0];

		if(SwitchMode < CHRONOLOGY_MODE || SwitchMode > ALWAYS_ON)
		{
			SwitchMode = ALWAYS_ON;
		}
	}
	else
	{
		SwitchMode = ALWAYS_ON;
	}

	return ret;
}

//读取光照度阈值
u8 ReadIlluminanceThreshold(void)
{
	u8 ret = 0;
	u8 buf[ILLUM_THER_LEN];

	memset(buf,0,ILLUM_THER_LEN);

	ret = ReadDataFromEepromToMemory(buf,ILLUM_THER_ADD,ILLUM_THER_LEN);

	if(ret == 1)
	{
		IlluminanceThreshold.on = (((u16)buf[0]) << 8) + (u16)buf[1];
		IlluminanceThreshold.off = (((u16)buf[2]) << 8) + (u16)buf[3];
	}
	else
	{
		IlluminanceThreshold.on = 25;
		IlluminanceThreshold.off = 35;
	}

	return ret;
}

//读取电源接口
u8 ReadPowerInterface(void)
{
	u8 ret = 0;
	u8 buf[POWER_INTER_LEN];

	memset(buf,0,POWER_INTER_LEN);

	ret = ReadDataFromEepromToMemory(buf,POWER_INTER_ADD,POWER_INTER_LEN);

	if(ret == 1)
	{
		PowerInterface = buf[0];

		if(PowerInterface > INTFC_DALI)
		{
			PowerInterface = INTFC_0_10V;
		}
	}
	else
	{
		PowerInterface = INTFC_0_10V;
	}

	return ret;
}

//读取亮度
u8 ReadLightLevelPercent(void)
{
	u8 ret = 0;
	u8 buf[POWER_INTER_LEN];

	memset(buf,0,POWER_INTER_LEN);

	ret = ReadDataFromEepromToMemory(buf,POWER_INTER_ADD,POWER_INTER_LEN);

	if(ret == 1)
	{
		DefaultLightLevelPercent = buf[0];

		if(DefaultLightLevelPercent > 100)
		{
			DefaultLightLevelPercent = 100;
		}
	}
	else
	{
		DefaultLightLevelPercent = 100;
	}

	return ret;
}

//读取设备配置信息
u8 ReadDeviceConfigPara(void)
{
	u8 ret = 0;
	u8 buf[CONFIG_INFO_LEN];
	u8 default_mail_add[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};

	memset(buf,0,CONFIG_INFO_LEN);

	ret = ReadDataFromEepromToMemory(buf,CONFIG_INFO_ADD,CONFIG_INFO_LEN);

	if(ret == 1)
	{
		memcpy(DeviceConfigPara.mail_add,buf + 0,8);

		DeviceConfigPara.lamp_type = buf[8];
		DeviceConfigPara.lamp_power = (((u16)buf[9]) << 8) + (u16)buf[10];
		DeviceConfigPara.lamp_pf = (((u16)buf[11]) << 8) + (u16)buf[12];

		memset(DeviceConfigPara.longitude,'0',10);
		memcpy(DeviceConfigPara.longitude,buf + 13,10);

		memset(DeviceConfigPara.latitude,'0',10);
		memcpy(DeviceConfigPara.latitude,buf + 23,10);
	}
	else
	{
		memcpy(DeviceConfigPara.mail_add,default_mail_add,8);

		DeviceConfigPara.lamp_type = 2;
		DeviceConfigPara.lamp_power = 120;
		DeviceConfigPara.lamp_pf = 9500;

		memset(DeviceConfigPara.longitude,'0',10);
		memset(DeviceConfigPara.latitude,'0',10);
	}

	return ret;
}

//读取软件版本
u8 ReadSoftWareVersion(void)
{
	u8 ret = 0;
	u8 buf[SOFT_WARE_VER_LEN];

	memset(buf,0,SOFT_WARE_VER_LEN);

	ret = ReadDataFromEepromToMemory(buf,SOFT_WARE_VER_ADD,SOFT_WARE_VER_LEN);

	if(ret == 1)
	{
		DeviceInfo.software_ver = (((u16)buf[0]) << 8) + (u16)buf[1];

		if(DeviceInfo.software_ver < 101 && DeviceInfo.software_ver > 9999)
		{
			DeviceInfo.software_ver = 101;
		}
	}
	else
	{
		DeviceInfo.software_ver = 101;
	}

	return ret;
}

//读取软件发布日期
u8 ReadSoftWareReleaseDate(void)
{
	u8 ret = 0;
	u8 buf[SOFT_WARE_REL_DATE_LEN];

	memset(buf,0,SOFT_WARE_REL_DATE_LEN);

	ret = ReadDataFromEepromToMemory(buf,SOFT_WARE_REL_DATE_ADD,SOFT_WARE_REL_DATE_LEN);

	if(ret == 1)
	{
		memcpy(DeviceInfo.software_release_date,buf + 0,3);
	}
	else
	{
		DeviceInfo.software_release_date[0] = 19;
		DeviceInfo.software_release_date[1] = 7;
		DeviceInfo.software_release_date[2] = 3;
	}

	return ret;
}

//读取硬件版本
u8 ReadHardWareVersion(void)
{
	u8 ret = 0;
	u8 buf[HARD_WARE_VER_LEN];

	memset(buf,0,HARD_WARE_VER_LEN);

	ret = ReadDataFromEepromToMemory(buf,HARD_WARE_VER_ADD,HARD_WARE_VER_LEN);

	if(ret == 1)
	{
		DeviceInfo.hardware_ver = (((u16)buf[0]) << 8) + (u16)buf[1];

		if(DeviceInfo.hardware_ver < 101 && DeviceInfo.hardware_ver > 9999)
		{
			DeviceInfo.hardware_ver = 101;
		}
	}
	else
	{
		DeviceInfo.hardware_ver = 101;
	}

	return ret;
}

//读取硬件发布日期
u8 ReadHardWareReleaseDate(void)
{
	u8 ret = 0;
	u8 buf[HARD_WARE_REL_DATE_LEN];

	memset(buf,0,HARD_WARE_REL_DATE_LEN);

	ret = ReadDataFromEepromToMemory(buf,HARD_WARE_REL_DATE_ADD,HARD_WARE_REL_DATE_LEN);

	if(ret == 1)
	{
		memcpy(DeviceInfo.hardware_release_date,buf + 0,3);
	}
	else
	{
		DeviceInfo.hardware_release_date[0] = 19;
		DeviceInfo.hardware_release_date[1] = 7;
		DeviceInfo.hardware_release_date[2] = 3;
	}

	return ret;
}

//读取协议版本
u8 ReadProtocolVer(void)
{
	u8 ret = 0;
	u8 buf[PROTOCOL_VER_LEN];

	memset(buf,0,PROTOCOL_VER_LEN);

	ret = ReadDataFromEepromToMemory(buf,PROTOCOL_VER_ADD,PROTOCOL_VER_LEN);

	if(ret == 1)
	{
		DeviceInfo.protocol_ver = (((u16)buf[0]) << 8) + (u16)buf[1];

		if(DeviceInfo.protocol_ver < 101 && DeviceInfo.protocol_ver > 9999)
		{
			DeviceInfo.protocol_ver = 101;
		}
	}
	else
	{
		DeviceInfo.protocol_ver = 101;
	}

	return ret;
}

//FTP固件信息
u8 ReadFrameWareInfo(void)
{
	u8 ret = 0;
	u8 buf[SOFT_WARE_INFO_LEN];

	memset(buf,0,SOFT_WARE_INFO_LEN);

	ret = ReadDataFromEepromToMemory(buf,SOFT_WARE_INFO_ADD,SOFT_WARE_INFO_LEN);

	if(ret == 1)
	{
		FrameWareInfo.version = (((u16)(*(buf + 0))) << 8) + (u16)(*(buf + 1));

		FrameWareInfo.length = (((u32)(*(buf + 2))) << 24) +
							   (((u32)(*(buf + 3))) << 16) +
							   (((u32)(*(buf + 4))) << 8) +
							   (((u32)(*(buf + 5))) << 0);
	}
	else
	{
		FrameWareInfo.version = 101;

		FrameWareInfo.length = 0;
	}

	return ret;
}

//将固件升级状态写入到EEPROM
void WriteFrameWareStateToEeprom(void)
{
	u8 temp_buf[20];

	temp_buf[0]  = FrameWareState.state;
	temp_buf[1]  = (u8)(FrameWareState.total_bags >> 8);
	temp_buf[2]  = (u8)FrameWareState.total_bags;
	temp_buf[3]  = (u8)(FrameWareState.current_bag_cnt >> 8);
	temp_buf[4]  = (u8)FrameWareState.current_bag_cnt;
	temp_buf[5]  = (u8)(FrameWareState.bag_size >> 8);
	temp_buf[6]  = (u8)FrameWareState.bag_size;
	temp_buf[7]  = (u8)(FrameWareState.last_bag_size >> 8);
	temp_buf[8]  = (u8)FrameWareState.last_bag_size;
	temp_buf[9]  = (u8)(FrameWareState.total_size >> 24);
	temp_buf[10] = (u8)(FrameWareState.total_size >> 16);
	temp_buf[11] = (u8)(FrameWareState.total_size >> 8);
	temp_buf[12] = (u8)FrameWareState.total_size;

	WriteDataFromMemoryToEeprom(temp_buf,UPDATE_STATE_ADD,UPDATE_STATE_LEN - 2);
}

//终端软件版本号
u8 UpdateSoftWareVer(void)
{
	u8 ret = 0;
	u8 buf[2];

	DeviceInfo.software_ver = FrameWareInfo.version;

	buf[0] = (u8)(DeviceInfo.software_ver >> 8);
	buf[1] = (u8)(DeviceInfo.software_ver & 0x00FF);

	WriteDataFromMemoryToEeprom(buf,SOFT_WARE_VER_ADD,SOFT_WARE_VER_LEN - 2);

	ret = 1;

	return ret;
}

//终端软件发布日期
u8 UpdateSoftWareReleaseDate(void)
{
	u8 ret = 0;

	DeviceInfo.software_release_date[2] = calendar.w_date;			//日
	DeviceInfo.software_release_date[1] = calendar.w_month;			//月
	DeviceInfo.software_release_date[0] = calendar.w_year - 2000;	//年

	WriteDataFromMemoryToEeprom(DeviceInfo.software_release_date,SOFT_WARE_REL_DATE_ADD,SOFT_WARE_REL_DATE_LEN - 2);

	ret = 1;

	return ret;
}

//读取固件设计状态
u8 ReadFrameWareState(void)
{
	u8 ret = 0;
	u16 page_num = 0;
	u16 i = 0;
	u8 buf[UPDATE_STATE_LEN];

	memset(buf,0,UPDATE_STATE_LEN);

	ret = ReadDataFromEepromToMemory(buf,UPDATE_STATE_ADD,UPDATE_STATE_LEN);

	if(ret == 1)
	{
		FrameWareState.state 			= *(buf + 0);
		FrameWareState.total_bags 		= ((((u16)(*(buf + 1))) << 8) & 0xFF00) +
		                                  (((u16)(*(buf + 2))) & 0x00FF);
		FrameWareState.current_bag_cnt 	= ((((u16)(*(buf + 3))) << 8) & 0xFF00) +
		                                  (((u16)(*(buf + 4))) & 0x00FF);
		FrameWareState.bag_size 		= ((((u16)(*(buf + 5))) << 8) & 0xFF00) +
		                                  (((u16)(*(buf + 6))) & 0x00FF);
		FrameWareState.last_bag_size 	= ((((u16)(*(buf + 7))) << 8) & 0xFF00) +
		                                  (((u16)(*(buf + 8))) & 0x00FF);

		FrameWareState.total_size 		= ((((u32)(*(buf + 9))) << 24) & 0xFF000000) +
								          ((((u32)(*(buf + 10))) << 16) & 0x00FF0000) +
								          ((((u32)(*(buf + 11))) << 8) & 0x0000FF00) +
								          ((((u32)(*(buf + 12))) << 0) & 0x000000FF);

		ret = 1;
	}
	else
	{
		RESET_STATE:
		FrameWareState.state 			= FIRMWARE_FREE;
		FrameWareState.total_bags 		= 0;
		FrameWareState.current_bag_cnt 	= 0;
		FrameWareState.bag_size 		= 0;
		FrameWareState.last_bag_size 	= 0;

		FrameWareState.total_size 		= 0;

		WriteFrameWareStateToEeprom();			//将默认值写入EEPROM
	}

	if(FrameWareState.state == FIRMWARE_DOWNLOADING ||
	   FrameWareState.state == FIRMWARE_DOWNLOAD_WAIT)
	{
		page_num = (FIRMWARE_MAX_FLASH_ADD - FIRMWARE_BUCKUP_FLASH_BASE_ADD) / 2048;	//得到备份区的扇区总数

		FLASH_Unlock();						//解锁FLASH

		for(i = 0; i < page_num; i ++)
		{
			FLASH_ErasePage(i * 2048 + FIRMWARE_BUCKUP_FLASH_BASE_ADD);	//擦除当前FLASH扇区
		}

		FLASH_Lock();						//上锁
	}

	if(FrameWareState.state == FIRMWARE_UPDATE_SUCCESS)
	{
		UpdateSoftWareVer();
		UpdateSoftWareReleaseDate();

		goto RESET_STATE;
	}

	return ret;
}

//读取服务器IP
u8 ReadServerIP(void)
{
	u8 ret = 0;

	u8 buf[SERVER_IP_LEN];

	ret = ReadDataFromEepromToMemory(buf,SERVER_IP_ADD, SERVER_IP_LEN);

	if(ret)
	{
		GetMemoryForSpecifyPointer(&ServerIP,strlen((char *)buf), buf);
	}
	else
	{
		if(ServerIP == NULL)
		{
			ServerIP = (u8 *)mymalloc(sizeof(u8) * 16);
		}

		memset(ServerIP,0,16);

#ifdef RELEASE_VERSION
		sprintf((char *)ServerIP, "117.60.157.137");
#else
		sprintf((char *)ServerIP, "180.101.147.115");
#endif
	}

	return ret;
}

//读取服务器PORT
u8 ReadServerPort(void)
{
	u8 ret = 0;

	u8 buf[SERVER_PORT_LEN];

	ret = ReadDataFromEepromToMemory(buf,SERVER_PORT_ADD, SERVER_PORT_LEN);

	if(ret)
	{
		GetMemoryForSpecifyPointer(&ServerPort,strlen((char *)buf), buf);
	}
	else
	{
		if(ServerPort == NULL)
		{
			ServerPort = (u8 *)mymalloc(sizeof(u8) * 6);
		}

		memset(ServerPort,0,16);

		sprintf((char *)ServerPort, "5683");
	}

	return ret;
}

//控制器开关灯时间数据
u8 ReadLampsSwitchProject(void)
{
	u8 ret = 0;
	u16 i = 0;
	u8 buf[SWITCH_DATE_DAYS_LEN];

	memset(buf,0,SWITCH_DATE_DAYS_LEN);

	ret = ReadDataFromEepromToMemory(buf,SWITCH_DATE_DAYS_ADD,SWITCH_DATE_DAYS_LEN);

	if(ret == 1)
	{
		LampsSwitchProject.start_month = *(buf + 0);
		LampsSwitchProject.start_date  = *(buf + 1);

		LampsSwitchProject.total_days = ((((u16)(*(buf + 2))) << 8) + (u16)(*(buf + 3)));
	}
	else
	{
		LampsSwitchProject.start_month = 0xFF;
		LampsSwitchProject.start_date  = 0xFF;

		LampsSwitchProject.total_days = 0;
	}

	for(i = 0; i < LampsSwitchProject.total_days; i ++)
	{
		memset(buf,0,SWITCH_TIME_LEN);

		ret = ReadDataFromEepromToMemory(buf,SWITCH_TIME_ADD + i * SWITCH_TIME_LEN,SWITCH_TIME_LEN);

		if(ret == 1)
		{
			LampsSwitchProject.switch_time[i].on_time[0]  = *(buf + 0);
			LampsSwitchProject.switch_time[i].on_time[1]  = *(buf + 1);
			LampsSwitchProject.switch_time[i].off_time[0] = *(buf + 2);
			LampsSwitchProject.switch_time[i].off_time[1] = *(buf + 3);
		}
		else
		{
			LampsSwitchProject.switch_time[i].on_time[0]  = 0xFF;
			LampsSwitchProject.switch_time[i].on_time[1]  = 0xFF;
			LampsSwitchProject.switch_time[i].off_time[0] = 0xFF;
			LampsSwitchProject.switch_time[i].off_time[1] = 0xFF;
		}
	}

	return ret;
}


//读取时间策略数组
u8 ReadRegularTimeGroups(void)
{
	u8 ret = 0;
	u16 i = 0;
	u16 j = 0;
	u16 read_crc = 0;
	u16 cal_crc = 0;
	u8 time_group[TIME_STRATEGY_LEN * MAX_GROUP_NUM];
	u8 read_success_buf_flag[MAX_GROUP_NUM];

	RegularTimeWeekDay = (pRegularTime)mymalloc(sizeof(RegularTime_S));
	RegularTimeWeekEnd = (pRegularTime)mymalloc(sizeof(RegularTime_S));
	RegularTimeHoliday = (pRegularTime)mymalloc(sizeof(RegularTime_S));

	RegularTimeWeekDay->number = 0xFF;
	RegularTimeWeekEnd->number = 0xFF;
	RegularTimeHoliday->number = 0xFF;

	RegularTimeWeekDay->prev = NULL;
	RegularTimeWeekEnd->prev = NULL;
	RegularTimeHoliday->prev = NULL;

	RegularTimeWeekDay->next = NULL;
	RegularTimeWeekEnd->next = NULL;
	RegularTimeHoliday->next = NULL;

	memset(time_group,0,TIME_STRATEGY_LEN * MAX_GROUP_NUM);
	memset(read_success_buf_flag,0,MAX_GROUP_NUM);

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		for(j = i * TIME_STRATEGY_LEN; j < i * TIME_STRATEGY_LEN + TIME_STRATEGY_LEN; j ++)
		{
			time_group[j] = AT24CXX_ReadOneByte(TIME_STRATEGY_ADD + j);
		}

		cal_crc = GetCRC16(&time_group[j - TIME_STRATEGY_LEN],TIME_STRATEGY_LEN - 2);
		read_crc = (((u16)time_group[j - 2]) << 8) + (u16)time_group[j - 1];

		if(cal_crc == read_crc)
		{
			read_success_buf_flag[i] = 1;
		}
	}

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		if(read_success_buf_flag[i] == 1)
		{
			pRegularTime tmp_time = NULL;

			tmp_time = (pRegularTime)mymalloc(sizeof(RegularTime_S));

			tmp_time->prev = NULL;
			tmp_time->next = NULL;

			tmp_time->number	= i;
			tmp_time->type 		= time_group[i * TIME_STRATEGY_LEN + 0];
			tmp_time->year 		= time_group[i * TIME_STRATEGY_LEN + 1];
			tmp_time->month 	= time_group[i * TIME_STRATEGY_LEN + 2];
			tmp_time->date 		= time_group[i * TIME_STRATEGY_LEN + 3];
			tmp_time->hour 		= time_group[i * TIME_STRATEGY_LEN + 4];
			tmp_time->minute 	= time_group[i * TIME_STRATEGY_LEN + 5];
			tmp_time->percent 	= time_group[i * TIME_STRATEGY_LEN + 6];

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
	}

	return ret;
}

//在EEPROM中读取运行参数
void ReadParametersFromEEPROM(void)
{
	ReadUpCommPortPara();
	ReadDataUploadInterval();
	ReadHeartBeatUploadInterval();
	ReadPeakStaggerTime();
	ReadSwitchMode();
	ReadIlluminanceThreshold();
	ReadPowerInterface();
	ReadLightLevelPercent();
	ReadDeviceConfigPara();
	ReadSoftWareVersion();
	ReadSoftWareReleaseDate();
	ReadHardWareVersion();
	ReadHardWareReleaseDate();
	ReadProtocolVer();
	ReadFrameWareInfo();
	ReadFrameWareState();
	ReadServerIP();
	ReadServerPort();
	ReadLampsSwitchProject();
	ReadRegularTimeGroups();
	ReadEventDetectTimeConf();
	ReadEventDetectThreConf();
	ReadEventRecordList();
}

//将数据打包成用户数据格式
u16 PackUserData(u8 ctrl_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf)
{
	u16 out_len = 0;

	*(outbuf + 0)  = 0x68;
	memcpy(outbuf + 1,DeviceConfigPara.mail_add,8);
	*(outbuf + 9)  = 0x68;
	*(outbuf + 10)  = (u8)(DeviceInfo.protocol_ver >> 8);
	*(outbuf + 11)  = (u8)(DeviceInfo.protocol_ver & 0x00FF);
	*(outbuf + 12) = ctrl_code;
	*(outbuf + 13) = (u8)(inbuf_len >> 8);
	*(outbuf + 14) = (u8)(inbuf_len & 0x00FF);
	memcpy(outbuf + 15,inbuf,inbuf_len);
	*(outbuf + 15 + inbuf_len) = CalCheckSum(outbuf, 15 + inbuf_len);
	*(outbuf + 15 + inbuf_len + 1) = 0x16;

	out_len = 15 + inbuf_len + 1 + 1;

	return out_len;
}

//将数据打包为命令响应报文
u16 PackCommandRspData(u16 cmd_id,u16 err_code,u8 *inbuf,u16 inbuf_len,u8 *outbuf)
{
	*(outbuf + 0) = 0x82;
	*(outbuf + 1) = (u8)(cmd_id >> 8);
	*(outbuf + 2) = (u8)(cmd_id & 0x00FF);
	*(outbuf + 3) = (u8)(err_code >> 8);
	*(outbuf + 4) = (u8)(err_code & 0x00FF);
	*(outbuf + 5) = (u8)(inbuf_len >> 8);
	*(outbuf + 6) = (u8)(inbuf_len & 0x00FF);

	memcpy(outbuf + 7,inbuf,inbuf_len);

	return inbuf_len + 7;
}

//将数据打包为事件上报格式报文
u16 PackEventUploadData(u8 *inbuf,u16 inbuf_len,u8 *outbuf)
{
	*(outbuf + 0) = 0x83;
	*(outbuf + 1) = (u8)(inbuf_len >> 8);
	*(outbuf + 2) = (u8)(inbuf_len & 0x00FF);

	memcpy(outbuf + 3,inbuf,inbuf_len);

	return inbuf_len + 3;
}

//将传感器数据解包到指定缓冲区
u16 UnPackSensorData(SensorMsg_S *msg,u8 *buf)
{
	u16 i = 0;
	u16 len = 0;

	if(msg != NULL)
	{
		*(buf + i) = (u8)(msg->in_put_current >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_current;
		i ++;

		*(buf + i) = (u8)(msg->in_put_voltage >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_voltage;
		i ++;

		*(buf + i) = (u8)(msg->in_put_freq >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_freq;
		i ++;

		*(buf + i) = (u8)(msg->in_put_power_p >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_power_p;
		i ++;

		*(buf + i) = (u8)(msg->in_put_power_q >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_power_q;
		i ++;

		*(buf + i) = (u8)(msg->in_put_power_s >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_power_s;
		i ++;

		*(buf + i) = (u8)(msg->in_put_pf >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_pf;
		i ++;

		*(buf + i) = (u8)(msg->in_put_energy_p >> 24);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_p >> 16);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_p >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_energy_p;
		i ++;

		*(buf + i) = (u8)(msg->in_put_energy_q >> 24);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_q >> 16);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_q >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_energy_q;
		i ++;

		*(buf + i) = (u8)(msg->in_put_energy_s >> 24);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_s >> 16);
		i ++;
		*(buf + i) = (u8)(msg->in_put_energy_s >> 8);
		i ++;
		*(buf + i) = (u8)msg->in_put_energy_s;
		i ++;

		*(buf + i) = (u8)(msg->out_put_current >> 8);
		i ++;
		*(buf + i) = (u8)msg->out_put_current;
		i ++;

		*(buf + i) = (u8)(msg->out_put_voltage >> 8);
		i ++;
		*(buf + i) = (u8)msg->out_put_voltage;
		i ++;

		*(buf + i) = msg->csq;
		i ++;

		*(buf + i) = msg->temperature;
		i ++;

		*(buf + i) = msg->humidity;
		i ++;

		*(buf + i) = (u8)(msg->illumination >> 8);
		i ++;
		*(buf + i) = (u8)msg->illumination;
		i ++;

		*(buf + i) = msg->light_level;
		i ++;

		*(buf + i) = msg->year;
		i ++;
		*(buf + i) = msg->month;
		i ++;
		*(buf + i) = msg->date;
		i ++;
		*(buf + i) = msg->hour;
		i ++;
		*(buf + i) = msg->minute;
		i ++;
		*(buf + i) = msg->second;
		i ++;

		len = i;
	}

	return len;
}

u8 RegularTimeGroupAdd(u8 type,pRegularTime group_time)
{
	u8 ret = 1;
	pRegularTime tmp_time = NULL;
	pRegularTime main_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	switch(type)
	{
		case TYPE_WEEKDAY:
			main_time = RegularTimeWeekDay;
		break;

		case TYPE_WEEKEND:
			main_time = RegularTimeWeekEnd;
		break;

		case TYPE_HOLIDAY_START:
			main_time = RegularTimeHoliday;

			HolodayRange.year_s  = group_time->year;
			HolodayRange.month_s = group_time->month;
			HolodayRange.date_s  = group_time->date;
			HolodayRange.year_e  = group_time->year;
			HolodayRange.month_e = group_time->month;
			HolodayRange.date_e  = group_time->date;
		break;

		case TYPE_HOLIDAY_END:
			main_time = RegularTimeHoliday;

			HolodayRange.year_e  = group_time->year;
			HolodayRange.month_e = group_time->month;
			HolodayRange.date_e  = group_time->date;
		break;

		default:

		break;
	}

	if(main_time != NULL)
	{
		for(tmp_time = main_time; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(group_time->number == tmp_time->number && tmp_time->number != 0xFF)
			{
				if(tmp_time->next != NULL)
				{
					tmp_time->prev->next = group_time;
					tmp_time->prev->next->next = tmp_time->next;
					tmp_time->next->prev = group_time;
					tmp_time->next->prev->prev = tmp_time->prev;

					myfree(tmp_time);
				}
				else
				{
					tmp_time->prev->next = group_time;
					tmp_time->prev->next->prev = tmp_time->prev;

					myfree(tmp_time);
				}

				break;
			}
			else if(tmp_time->next == NULL)
			{
				tmp_time->next = group_time;
				tmp_time->next->prev = tmp_time;

				break;
			}
		}

		if(group_time->type == TYPE_HOLIDAY_END)
		{
			GET_RANGE:
			group_time->range.year_s  = HolodayRange.year_s;
			group_time->range.month_s = HolodayRange.month_s;
			group_time->range.date_s  = HolodayRange.date_s;

			group_time->range.year_e  = HolodayRange.year_e;
			group_time->range.month_e = HolodayRange.month_e;
			group_time->range.date_e  = HolodayRange.date_e;

			group_time = group_time->prev;

			if(group_time->type == TYPE_HOLIDAY_START)
			{
				goto GET_RANGE;
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

u8 RegularTimeGroupSub(u8 number)
{
	u8 ret = 0;
	pRegularTime tmp_time = NULL;

	if(xSchedulerRunning == 1)
	{
		xSemaphoreTake(xMutex_STRATEGY, portMAX_DELAY);
	}

	if(RegularTimeWeekDay != NULL || RegularTimeWeekDay->next != NULL)
	{
		for(tmp_time = RegularTimeWeekDay->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(tmp_time->number == number)
			{
				if(tmp_time->next != NULL)
				{
					tmp_time->prev->next = tmp_time->next;
					tmp_time->next->prev = tmp_time->prev;
				}
				else
				{
					tmp_time->prev->next = NULL;
				}

				myfree(tmp_time);

				ret = 1;
			}
		}
	}

	if(RegularTimeWeekEnd != NULL || RegularTimeWeekEnd->next != NULL)
	{
		for(tmp_time = RegularTimeWeekEnd->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(tmp_time->number == number)
			{
				if(tmp_time->next != NULL)
				{
					tmp_time->prev->next = tmp_time->next;
					tmp_time->next->prev = tmp_time->prev;
				}
				else
				{
					tmp_time->prev->next = NULL;
				}

				myfree(tmp_time);

				ret = 1;
			}
		}
	}

	if(RegularTimeHoliday != NULL || RegularTimeHoliday->next != NULL)
	{
		for(tmp_time = RegularTimeHoliday->next; tmp_time != NULL; tmp_time = tmp_time->next)
		{
			if(tmp_time->number == number)
			{
				if(tmp_time->next != NULL)
				{
					tmp_time->prev->next = tmp_time->next;
					tmp_time->next->prev = tmp_time->prev;
				}
				else
				{
					tmp_time->prev->next = NULL;
				}

				myfree(tmp_time);

				ret = 1;
			}
		}
	}

	if(xSchedulerRunning == 1)
	{
		xSemaphoreGive(xMutex_STRATEGY);
	}

	return ret;
}

void RemoveAllStrategy(void)
{
	u16 i = 0;

	for(i = 0; i < MAX_GROUP_NUM; i ++)
	{
		RegularTimeGroupSub(i);

		AT24CXX_WriteLenByte(TIME_STRATEGY_ADD + TIME_STRATEGY_LEN * i + (TIME_STRATEGY_LEN - 2),0x0000,2);
	}
}












