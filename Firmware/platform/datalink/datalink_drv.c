#include "common.h"
#include "bsp.h"
#include "bsp_os.h"
#include "uart_drv.h"
#include "string.h"
#include "datalink_drv.h"
#include "gpio_drv.h"
#include "led_drv.h"
#include "pwm_drv.h"
#include "button_drv.h"

static u8 csq_cmd[]="AT*CSQ?\r\n";
static u8 pack[16]={0};
static u8 button_output[13] = {0};

u8 module_index=0;

#define PWM_DEFAULT_VALUE	2048
#define RX_HEAD	0xA5
#define RX_TAIL	0x5A

#define DATALINK1	0
#define DATALINK2	1

dataLink_t datalink;

extern FIFO_T stFiFo1;
extern FIFO_T stFiFo3;

enum SystemState systemState;

void datalink_init(u8 index)
{
	u8 i=0;

	systemState = SYSTEM_INIT;
	
	datalink.index = index;
	datalink.link_connected = FALSE;
	datalink.linked = FALSE;
	datalink.protected = FALSE;
	datalink.hover_set = FALSE;
	datalink.hover_state = FALSE;
	datalink.returned = FALSE;
	datalink.connected_time = 0;
	datalink.hover_time = 0;
	datalink.connected_tick = 0;
	datalink.hover_recovery = 0;
	datalink.back_recovery = 0;
	
	for(i=0;i<4;i++)
	{
		datalink.hover_pwm_value[i] = PWM_DEFAULT_VALUE;
	}

	systemState = SYSTEM_FLYING;
}

void get_rssi(uint8_t which)
{
	if(which%2 == 0)
	{
		uart_drv_dbg_send(csq_cmd,sizeof(csq_cmd));
	}else{
		uart_drv_data_send(csq_cmd,sizeof(csq_cmd));
	}
}

u8 getLedStatus(void)
{
	u8 ret=0;

	if(button_value_get(BUTTON_SRC_POWER_CTRL1) == 1){
		ret |= POWER_LED;
	}else{
		ret &= ~POWER_LED;
	}
	if(button_value_get(BUTTON_SRC_RETURN_LED) == 1){
		ret |= RETURN_LED;
	}else{
		ret &= ~RETURN_LED;
	}
	return ret;
}

void unpackDataLink(void)
{
//	u8 i;

	button_output[0] =pack[10]&SHUTTLE_ADJ2;
	button_output[1] =pack[10]&SHUTTLE_ADJ1;
	button_output[2] =pack[10]&SHUTTLE_BUTTON;
	button_output[3] =pack[10]&PHOTO_BUTTON;
	button_output[4] =pack[10]&PLAYBACK_BUTTON;
	button_output[5] =pack[10]&MODE_SET1;
	button_output[6] =pack[10]&MODE_SET2;
	button_output[7] =pack[10]&VIDEO_BUTTON;

	button_output[8] =pack[11]&POWER_BUTTON;
	button_output[9] =pack[11]&RETURN_RESERVE2;
	button_output[10] =pack[11]&RETURN_TOGGLE;
	button_output[11] =pack[11]&RETURN_RESERVE1;
	button_output[12] =pack[11]&RETURN_BUTTON;

	//for(i=0;i<13;i++)
	//	MSG("%d,",button_output[i]);
	//MSG("\r\n");
	
}

void gpioSetOutput(u8 *gpio,u8 size)
{
	u8 i=0;
	while(size --){
		if(*(gpio++)){
			gpio_value_set(i);
		}else{
			gpio_value_reset(i);
		}
		if(i<GPIO_SRC_NUM)
			i++;
	}
}

static u8 rssi1=0;
static u8 rssi2=0;

static u8 checksenddata(u8 *data,u8 size)
{
	u8 ret;
	u16 sum=0;

	while(size--)
	{
		sum += *(data++);
	}

	ret = sum %256;

	return ret;
}

void datalink_send(u8 which)
{
	u8 send[8]={0};
	static u8 number=0;

	send[0] = RX_HEAD;
	send[1] = number++;
	send[2] = getLedStatus();	//获取led返回状态
	if(which == 0)
	{	
		send[3] = rssi1;
		send[4] = 0;
	}else{
		send[3] = 0;
		send[4] = rssi2;
	}
	send[5] = 0x00;
	send[6] = checksenddata(send,5);
	send[7] = RX_TAIL;

	if(which == 1)
	{
		uart_drv_data_send(send,8);
	}else{
		uart_drv_dbg_send(send,8);
	}
}



bool datalink_received(void)
{
	u8 read;
	u8 position,temp;
	bool ret= FALSE;
	//static u8 which=0;
	//u8 csq_value[10]="RSSI=";

	if(Fifo_DataLen(&stFiFo1) >= 18)
	{
		if(Fifo_Read(&stFiFo1,&read) == TRUE && read == 0xAA)
		{
			for(position=0;position<16;position++)//提取数据段
			{
				Fifo_Read(&stFiFo1,&temp);
				pack[position] = temp;	//接收到完整数据
				MSG("0x%x,",pack[position]);
			}
			//MSG("\r\n");
			if(Fifo_Read(&stFiFo1,&read) == TRUE && read == 0x55)
				ret=TRUE;
		}else if(read == '+'){
			if(Fifo_Read(&stFiFo1,&read) == TRUE && read == 'C'){
				if(Fifo_Read(&stFiFo1,&read) == TRUE && read == 'S'){
					if(Fifo_Read(&stFiFo1,&read) == TRUE && read == 'Q'){
						if(Fifo_Read(&stFiFo1,&read) == TRUE && read == ':'){
							//MSG("we got a rssi value\r\n");
							if(Fifo_Read(&stFiFo1,&read) == TRUE)
								//csq_value[5] = read;
								rssi1 = read-'0';	//转化成10进制数
							if(Fifo_Read(&stFiFo1,&read) == TRUE && read >= '0' && read <= '9'){
								//csq_value[6] = read;
								rssi1 = rssi1*10+read-'0';	//转化成10进制数
							}
							//send rssi value(RSSI=xx) to transmitter
							//uart_drv_data_send(csq_value,7);
							datalink_send(DATALINK1);
						}
					}
				}
			}
		}
	}

	if(Fifo_DataLen(&stFiFo3) >= 18)
	{
		if(Fifo_Read(&stFiFo3,&read) == TRUE && read == 0xAA)
		{
			for(position=0;position<16;position++)//提取数据段
			{
				Fifo_Read(&stFiFo3,&temp);
				pack[position] = temp;	//接收到完整数据
				MSG("0x%x,",pack[position]);
			}
			//MSG("\r\n");
			if(Fifo_Read(&stFiFo3,&read) == TRUE && read == 0x55)
				ret=TRUE;
		}else if(read == '+'){
			if(Fifo_Read(&stFiFo3,&read) == TRUE && read == 'C'){
				if(Fifo_Read(&stFiFo3,&read) == TRUE && read == 'S'){
					if(Fifo_Read(&stFiFo3,&read) == TRUE && read == 'Q'){
						if(Fifo_Read(&stFiFo3,&read) == TRUE && read == ':'){
							//MSG("we got a rssi value\r\n");
							if(Fifo_Read(&stFiFo3,&read) == TRUE)
								//csq_value[5] = read;
								rssi2 = read-'0';	//转化成10进制数
							if(Fifo_Read(&stFiFo3,&read) == TRUE && read >= '0' && read <= '9'){
								//csq_value[6] = read;
								rssi2 = rssi2*10+read-'0';	//转化成10进制数
							}
							//send rssi value(RSSI=xx) to transmitter
							//uart_drv_data_send(csq_value,7);
							datalink_send(DATALINK2);
						}
					}
				}
			}
		}
	}
	return ret;
}

static u8 checkSum(u8 *data,u8 size)
{
	u8 ret;
	u16 sum=0;

	while(size--)
	{
		sum += *(data++);
	}

	ret = sum %256;

	return ret;
}

void data_unpack(void)
{
	u16 pwm[5] = {0};
	
	if(pack[15] == checkSum(pack,15))
	{
		//MSG("we got a correct packet\r\n");
		pack[13] = module_index;
		if(datalink.link_connected == FALSE)
			datalink.link_connected = TRUE;
		if(datalink.linked == FALSE)
			datalink.linked = TRUE;
		//get timeout stamp
		datalink.connected_time = datalink.connected_tick +15;
		
		pwm[PWM_CHANNEL_CAMERA] = (pack[0]<<8) + pack[1];
		pwm[PWM_CHANNEL_JS_R1] = (pack[2]<<8) + pack[3];
		pwm[PWM_CHANNEL_JS_R2] = (pack[4]<<8) + pack[5];
		pwm[PWM_CHANNEL_JS_L1] = (pack[6]<<8) + pack[7];
		pwm[PWM_CHANNEL_JS_L2] = (pack[8]<<8) + pack[9];

		if(datalink.hover_set == FALSE)
		{
			datalink.hover_pwm_value[0] = pwm[PWM_CHANNEL_JS_R1];
			datalink.hover_pwm_value[1] = pwm[PWM_CHANNEL_JS_R2];
			datalink.hover_pwm_value[2] = pwm[PWM_CHANNEL_JS_L1];
			datalink.hover_pwm_value[3] = pwm[PWM_CHANNEL_JS_L2];

			datalink.hover_set=TRUE;
		}

		//MSG("---%d,%d,%d,%d,%d---\r\n",pwm[PWM_CHANNEL_CAMERA],pwm[PWM_CHANNEL_JS_R1],pwm[PWM_CHANNEL_JS_R2],pwm[PWM_CHANNEL_JS_L1],pwm[PWM_CHANNEL_JS_L2]);
		TIM_SetCompare1(TIM5,pwm[PWM_CHANNEL_CAMERA]);
		if(datalink.hover_state == FALSE)
		{
			TIM_SetCompare1(TIM3,pwm[PWM_CHANNEL_JS_R1]);
			TIM_SetCompare2(TIM3,pwm[PWM_CHANNEL_JS_R2]);
			TIM_SetCompare3(TIM3,pwm[PWM_CHANNEL_JS_L1]);
			TIM_SetCompare4(TIM3,pwm[PWM_CHANNEL_JS_L2]);
		}
		unpackDataLink();
		gpioSetOutput(button_output,13);
		LED_B_ON;
	}
}

void handler_protected(void)
{
	switch(systemState)
	{
		case SYSTEM_FLYING:
			if(datalink.protected)		//触发了失控保护
			{
				systemState = SYSTEM_HOVER;
				MSG("enter hover state\r\n");
			}
			break;
		case SYSTEM_HOVER:
			if(datalink.hover_state == FALSE)
			{
				datalink.hover_state=TRUE;
				datalink.hover_time = datalink.connected_tick + 6000;	//设置悬停时间
				//设置摇杆中位值，保持悬停状态
				TIM_SetCompare1(TIM3,datalink.hover_pwm_value[0]);
				TIM_SetCompare2(TIM3,datalink.hover_pwm_value[1]);
				TIM_SetCompare3(TIM3,datalink.hover_pwm_value[2]);
				TIM_SetCompare4(TIM3,datalink.hover_pwm_value[3]);
				MSG("linkquality too bad,hover the craft for 1 min\r\n");
			}else{
				if(datalink.hover_time < datalink.connected_tick)	//等待时间结束
				{
					systemState = SYSTEM_BACK;
					datalink.hover_state =FALSE;
					MSG("hover action done,go to back action\r\n");
				}
				//BSP_OS_TimeDly(100);
				//数据链路从悬停状态下恢复
				if(datalink.protected == FALSE)
				{
					datalink.hover_recovery++;
					if(datalink.hover_recovery > 33)	//恢复链路1s
					{
						//数据链路恢复稳定，切换到正常飞行模式
						systemState = SYSTEM_FLYING;
						//清除标志位
						datalink.hover_state =FALSE;
						datalink.hover_time =0;
						datalink.hover_recovery=0;
						MSG("recovery from hover to flying state\r\n");
					}
				}else{
					datalink.hover_recovery =0;
				}					
				//MSG("wait for hover action done\r\n");
				
				
			}
			break;
		case SYSTEM_BACK:
			if(datalink.returned == FALSE)
			{
				datalink.returned =TRUE;
				MSG("linkquality too bad,do return action\r\n");
				//do_return_action();		//产生返航动作
			}else{
				if(datalink.protected == FALSE)
				{
					datalink.back_recovery++;
					if(datalink.back_recovery > 33)	//稳定恢复链路1s
					{
						//数据链路恢复稳定，切换到正常飞行模式
						systemState = SYSTEM_FLYING;
						datalink.returned =FALSE;
						datalink.back_recovery =0;
						MSG("recovery from back to flying state\r\n");
					}
				}else{
					datalink.back_recovery =0;
				}
			}
			break;
		default:
			break;
	}
}

void datalink_state(void)
{
	if(datalink.connected_time < datalink.connected_tick && datalink.link_connected){
		datalink.link_connected = FALSE;
		//MSG("disconnected step1\r\n");
	}
	if(datalink.link_connected == FALSE && datalink.linked){
		//500ms没有收到数据包，触发失控保护
		datalink.protected = TRUE;
		//MSG("disconnected step2\r\n");
	}
	if(datalink.protected && datalink.link_connected){
		datalink.protected = FALSE;
		//MSG("disconnected recovery\r\n");
	}
	datalink.connected_tick++;
}
