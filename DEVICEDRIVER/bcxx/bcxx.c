#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "at_cmd.h"
#include "utils.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "bcxx.h"
#include "fifo.h"
#include "common.h"
#include "uart4.h"


unsigned char   bcxx_init_ok;
unsigned char	bcxx_busy = 0;
char *bcxx_imei = NULL;


void bcxx_hard_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	BCXX_PWREN_LOW;
	BCXX_RST_LOW;
}

void bcxx_hard_reset(void)
{
	BCXX_PWREN_LOW;						//关闭电源
	delay_ms(300);
	BCXX_PWREN_HIGH;					//打开电源

	delay_ms(100);

	BCXX_RST_HIGH;						//硬件复位
	delay_ms(300);
	BCXX_RST_LOW;

	bcxx_init_ok = 1;
}

u32 ip_SendData(int8_t *buf, uint32_t len)
{
     SentData((char *)buf,"OK",100);
     return len;
}

void netif_rx(uint8_t *buf,uint16_t *read)
{
	uint8_t ptr[1024] = {0};

	*read = fifo_get(dl_buf_id,ptr);

	if(*read != 0)
	{
		if(strstr((const char *)ptr, "+NNMI:") != NULL)
		{
			memcpy(buf,ptr,*read);
		}
		else
		{
			*read = 0;
		}
	}
}

void bcxx_soft_init(void)
{
	RE_INIT:

	bcxx_hard_reset();

	nbiot_sleep(8000);

	if(bcxx_set_NATSPEED(115200) != 1)
		goto RE_INIT;

	if(bcxx_get_AT_CGSN() != 1)
		goto RE_INIT;

	if(bcxx_get_AT_NCCID() != 1)
		goto RE_INIT;

	if(bcxx_get_AT_CIMI() != 1)
		goto RE_INIT;
	
//	if(bcxx_set_AT_CGDCONT("nb.developer") != 1)
//		goto RE_INIT;

	printf("got imei\r\n");

	if(bcxx_set_AT_CFUN(0) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_NBAND(DeviceInfo.imsi) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_NCDP((char *)ServerIP,(char *)ServerPort) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CELL_RESELECTION() != 1)
		goto RE_INIT;

	if(bcxx_set_AT_NRB() != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CFUN(1) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CEDRXS(0) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CPSMS(0) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CSCON(0) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CEREG(4) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_QREGSWT(1) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_NNMI(1) != 1)
		goto RE_INIT;

	if(bcxx_set_AT_CGATT(1) != 1)
		goto RE_INIT;

	nbiot_sleep(10000);

//	printf("bc95 init sucess\r\n");
}

//设定模块波特率
unsigned char bcxx_set_NATSPEED(u32 baud_rate)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

	if(ret == 1)				//波特率默认9600
	{
		memset(cmd_tx_buf,0,64);

		sprintf(cmd_tx_buf,"AT+NATSPEED=%d,30,1,2,1,0,0\r\n",baud_rate);

		ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

		if(ret == 1)
		{
			UART4_Init(baud_rate);

			memset(cmd_tx_buf,0,64);

			sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

			ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
		}
	}
	else if(ret == 0)
	{
		UART4_Init(baud_rate);

		memset(cmd_tx_buf,0,64);

		sprintf(cmd_tx_buf,"AT+NATSPEED?\r\n");

		ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);
	}

    return ret;
}

unsigned char bcxx_set_AT_CFUN(char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CFUN=%d\r\n", cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_90S);

    return ret;
}

//设置频带
unsigned char bcxx_set_AT_NBAND(unsigned char *imsi)
{
	unsigned char ret = 0;
	unsigned char operators_code = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	if(imsi == NULL)
	{
		return ret;
	}

	operators_code = (*(imsi + 3) - 0x30) * 10 + *(imsi + 4) - 0x30;

	if(operators_code == 0 ||
	   operators_code == 2 ||
	   operators_code == 4 ||
	   operators_code == 7 ||
	   operators_code == 1 ||
	   operators_code == 6 ||
	   operators_code == 9)
	{
		NB_ModulePara.band = 8;
	}
	else if(operators_code == 3 ||
	        operators_code == 5 ||
	        operators_code == 11)
	{
		NB_ModulePara.band = 5;
	}
	else
	{
		NB_ModulePara.band = 8;
	}

	sprintf(cmd_tx_buf,"AT+NBAND=%d\r\n",NB_ModulePara.band);

	ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_90S);

    return ret;
}

//获取IMEI号
unsigned char bcxx_get_AT_CGSN(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+CGSN=1\r\n", "+CGSN", 100,0,TIMEOUT_1S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "CGSN:", 1, "\r\n", 2, (unsigned char *)buf);

		if(strlen(buf) == IMEI_LEN)
		{
			GetMemoryForSpecifyPointer(&DeviceInfo.imei,IMEI_LEN,(u8 *)buf);

			ret = 1;
		}
    }

    return ret;
}

//获取ICCID
unsigned char bcxx_get_AT_NCCID(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+NCCID\r\n", "OK", 100,0,TIMEOUT_5S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "NCCID:", 1, "\r\n", 2, (unsigned char *)buf);

		if(strlen(buf) == ICCID19_LEN)
		{
			GetMemoryForSpecifyPointer(&DeviceInfo.iccid,ICCID19_LEN,(u8 *)buf);

			ret = 1;
		}
		else if(strlen(buf) == ICCID20_LEN)
		{
			GetMemoryForSpecifyPointer(&DeviceInfo.iccid,ICCID20_LEN,(u8 *)buf);

			ret = 1;
		}
    }

    return ret;
}

//获取IMSI
unsigned char bcxx_get_AT_CIMI(void)
{
	unsigned char ret = 0;
	char buf[32];

    if(SendCmd("AT+CIMI\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
    {
		memset(buf,0,32);

		get_str1((unsigned char *)result_ptr->data, "\r\n", 1, "\r\n", 2, (unsigned char *)buf);

		if(strlen(buf) == IMSI_LEN)
		{
			GetMemoryForSpecifyPointer(&DeviceInfo.imsi,IMSI_LEN,(u8 *)buf);

			ret = 1;
		}
    }

    return ret;
}

//设置APN
unsigned char bcxx_set_AT_CGDCONT(u8 *apn)
{
	unsigned char ret = 0;
	
	char cmd_tx_buf[32];

	memset(cmd_tx_buf,0,32);

//	sprintf(cmd_tx_buf,"AT+CGDCONT?\r\n");
	sprintf(cmd_tx_buf,"AT+CGDCONT=0,\"IPV4V6\",\"%s\"\r\n",apn);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//打开小区重选功能
unsigned char bcxx_set_AT_CELL_RESELECTION(void)
{
	unsigned char ret = 0;

    ret = SendCmd("AT+NCONFIG=CELL_RESELECTION,TRUE\r\n", "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//重启模块
unsigned char bcxx_set_AT_NRB(void)
{
	unsigned char ret = 0;

	SendCmd("AT+NRB\r\n", "OK", 1000,0,TIMEOUT_10S);

	ret = SendCmd("AT\r\n", "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置IOT平台IP和端口 HUAWEI IOT
unsigned char bcxx_set_AT_NCDP(char *addr, char *port)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NCDP=%s,%s\r\n",addr,port);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置连接状态自动回显
unsigned char bcxx_set_AT_CSCON(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CSCON=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//EPS Network Registration Status
unsigned char bcxx_set_AT_CEREG(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CEREG=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置网络数据接收模式HUAWEI IOT
unsigned char bcxx_set_AT_NNMI(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NNMI=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置网络数据接收模式 TCP/IP
unsigned char bcxx_set_AT_NSONMI(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NSONMI=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

unsigned char bcxx_set_AT_CGATT(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CGATT=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_2S);

    return ret;
}

unsigned char bcxx_set_AT_QREGSWT(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+QREGSWT=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_2S);

    return ret;
}

//设置eDRX开关
unsigned char bcxx_set_AT_CEDRXS(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CEDRXS=%d,5\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//设置PMS开关
unsigned char bcxx_set_AT_CPSMS(unsigned char cmd)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+CPSMS=%d\r\n",cmd);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//向IOT平台发送数据 HUAWEI IOT
unsigned char bcxx_set_AT_NMGS(unsigned int len,char *buf)
{
	unsigned char ret = 0;
	char cmd_tx_buf[NET_BUF_MAX_LEN];

	memset(cmd_tx_buf,0,NET_BUF_MAX_LEN);

	sprintf(cmd_tx_buf,"AT+NMGS=%d,%s\r\n",len,buf);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//向IOT平台发送数据 HUAWEI IOT
unsigned char bcxx_set_AT_QLWULDATA(unsigned int len,char *buf)
{
	unsigned char ret = 0;
	char cmd_tx_buf[NET_BUF_MAX_LEN];

	memset(cmd_tx_buf,0,NET_BUF_MAX_LEN);

	sprintf(cmd_tx_buf,"AT+QLWULDATA=%d,%s\r\n",len,buf);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//获取本地IP地址
unsigned char bcxx_get_AT_CGPADDR(char **ip)
{
	unsigned char ret = 0;
	unsigned char len = 0;
	unsigned char new_len = 0;
	unsigned char msg[20];

    if(SendCmd("AT+CGPADDR\r\n", ",", 100,20,TIMEOUT_1S) == 1)
    {
		memset(msg,0,20);

		get_str1((unsigned char *)result_ptr->data, "+CGPADDR:0,", 1, "\r\nOK", 1, (unsigned char *)msg);

		new_len = strlen((char *)msg);

		if(new_len != 0)
		{
			if(*ip == NULL)
			{
				*ip = (char *)mymalloc(sizeof(u8) * len + 1);
			}

			if(*ip != NULL)
			{
				len = strlen((char *)*ip);

				if(len == new_len)
				{
					memset(*ip,0,new_len + 1);
					memcpy(*ip,msg,new_len);
					ret = 1;
				}
				else
				{
					myfree(*ip);
					*ip = (char *)mymalloc(sizeof(u8) * new_len + 1);
					if(ip != NULL)
					{
						memset(*ip,0,new_len + 1);
						memcpy(*ip,msg,new_len);
						len = new_len;
						new_len = 0;
						ret = 1;
					}
				}
			}
		}
    }

    return ret;
}

//新建一个SOCKET
unsigned char bcxx_set_AT_NSOCR(char *type, char *protocol,char *port)
{
	unsigned char ret = 255;
	unsigned char buf[3] = {0,0,0};
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NSOCR=%s,%s,%s,2\r\n",type,protocol,port);

    if(SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S) == 1)
    {
		get_str1((unsigned char *)result_ptr->data, "\r\n", 1, "\r\n", 2, (unsigned char *)buf);
		if(strlen((char *)buf) == 1)
		{
			ret = buf[0] - 0x30;
		}
    }

    return ret;
}

//关闭一个SOCKET
unsigned char bcxx_set_AT_NSOCL(unsigned char socket)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NSOCL=%d\r\n",socket);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//向UDP服务器发送数据并等待响应数据
unsigned char bcxx_set_AT_NSOFT(unsigned char socket, char *ip,char *port,unsigned int len,char *inbuf)
{
	unsigned char ret = 0;
	char cmd_tx_buf[256];

	memset(cmd_tx_buf,0,256);

	sprintf(cmd_tx_buf,"AT+NSOST=%d,%s,%s,%d,%s\r\n",socket,ip,port,len,inbuf);

    ret = SendCmd(cmd_tx_buf, "+NSOSTR:", 100,0,TIMEOUT_60S);

    return ret;
}

//建立一个TCP连接
unsigned char bcxx_set_AT_NSOCO(unsigned char socket, char *ip,char *port)
{
	unsigned char ret = 0;
	char cmd_tx_buf[64];

	memset(cmd_tx_buf,0,64);

	sprintf(cmd_tx_buf,"AT+NSOCO=%d,%s,%s\r\n",socket,ip,port);

    ret = SendCmd(cmd_tx_buf, "OK", 100,0,TIMEOUT_1S);

    return ret;
}

//通过TCP连接发送数据
unsigned char bcxx_set_AT_NSOSD(unsigned char socket, unsigned int len,char *inbuf)
{
	unsigned char ret = 0;
	char cmd_tx_buf[512];

	memset(cmd_tx_buf,0,512);

	sprintf(cmd_tx_buf,"AT+NSOSD=%d,%d,%s,0x100,100\r\n",socket,len,inbuf);

    ret = SendCmd(cmd_tx_buf, "+NSOSTR:", 100,0,TIMEOUT_60S);

    return ret;
}


//获取信号强度
unsigned char bcxx_get_AT_CSQ(unsigned char *csq)
{
	u8 ret = 0;
	u8 i = 0;
	char *msg = NULL;
	char tmp[10];

	if(SendCmd("AT+CSQ\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
	{
		msg = strstr((char *)result_ptr->data,"+CSQ:");

		if(msg == NULL)
			return 0;

		memset(tmp,0,10);

		msg = msg + 5;

		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';

		*csq = nbiot_atoi(tmp,strlen(tmp));

		if(*csq == 0 || *csq >= 99)
		{
			*csq = 0;
			ret = 0;
		}
	}

	return ret;
}

//获取UE模块状态
/*
	rsrp	参考信号接收功率
	rssi	接收信号强度等级
	snr		信噪比
	pci		物理小区标识
	rsrq	参考信号接收质量
	ecl		信号覆盖等级
*/
unsigned char bcxx_get_AT_NUESTATS(signed short *rsrp,
                                   signed short *rssi,
                                   signed short *snr,
                                   signed short *pci,
                                   signed short *rsrq)
{
	u8 ret = 0;
	u8 i = 0;
	char *msg = NULL;
	char tmp[10];

	if(SendCmd("AT+NUESTATS=CELL\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
	{
		msg = strstr((char *)result_ptr->data,":CELL,");

		if(msg == NULL)
			return 0;

		memset(tmp,0,10);

		msg = msg + 6;

		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';
		*pci = nbiot_atoi(tmp,strlen(tmp));

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';
		*rsrp = nbiot_atoi(tmp,strlen(tmp));

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';
		*rsrq = nbiot_atoi(tmp,strlen(tmp));

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != ',')
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';
		*rssi = nbiot_atoi(tmp,strlen(tmp));

		i = 0;
		msg = msg + 1;
		memset(tmp,0,10);
		while(*msg != 0x0D)
		tmp[i ++] = *(msg ++);
		tmp[i] = '\0';
		*snr = nbiot_atoi(tmp,strlen(tmp));
	}

	return ret;
}

//从模块获取时间
unsigned char bcxx_get_AT_CCLK(char *buf)
{
	unsigned char ret = 0;

    if(SendCmd("AT+CCLK?\r\n", "OK", 100,0,TIMEOUT_1S) == 1)
    {
        if(search_str((unsigned char *)result_ptr->data, "+CCLK:") != -1)
		{
			get_str1((unsigned char *)result_ptr->data, "+CCLK:", 1, "\r\n\r\nOK", 1, (unsigned char *)buf);

			ret = 1;
		}
    }

    return ret;
}



















