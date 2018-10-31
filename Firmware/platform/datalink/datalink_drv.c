#include "common.h"
#include "bsp.h"
#include "bsp_os.h"
#include "uart_drv.h"
#include "string.h"
#include "datalink_drv.h"
#include "gpio_drv.h"
#include "led_drv.h"
#include "pwm_drv.h"

static u8 csq_cmd[]="AT*CSQ?\r\n";
static u8 pack[14]={0};
static u8 button_output[13] = {0};

dataLink_t datalink;

extern FIFO_T stFiFo;

void get_rssi(void)
{
	uart_drv_data_send(csq_cmd,sizeof(csq_cmd));
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

bool datalink_received(void)
{
	u8 read;
	u8 position,temp;
	bool ret= FALSE;
	u8 csq_value[10]="RSSI=";

	if(Fifo_DataLen(&stFiFo) >= 16)
	{
		if(Fifo_Read(&stFiFo,&read) == TRUE && read == 0xAA)
		{
			for(position=0;position<14;position++)//提取数据段
			{
				Fifo_Read(&stFiFo,&temp);
				if(temp == 0xF5)
					temp = 0x0A;
				pack[position] = temp;	//接收到完整数据
				MSG("0x%x,",pack[position]);
			}
			MSG("\r\n");
			if(Fifo_Read(&stFiFo,&read) == TRUE && read == 0x55)
				ret=TRUE;
		}else if(read == '+'){
			if(Fifo_Read(&stFiFo,&read) == TRUE && read == 'C'){
				if(Fifo_Read(&stFiFo,&read) == TRUE && read == 'S'){
					if(Fifo_Read(&stFiFo,&read) == TRUE && read == 'Q'){
						if(Fifo_Read(&stFiFo,&read) == TRUE && read == ':'){
							//MSG("we got a rssi value\r\n");
							if(Fifo_Read(&stFiFo,&read) == TRUE)
								csq_value[5] = read;
							if(Fifo_Read(&stFiFo,&read) == TRUE && read >= '0' && read <= '9'){
								csq_value[6] = read;
							}
							//send rssi value(RSSI=xx) to transmitter
							uart_drv_data_send(csq_value,7);
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
	
	if(pack[13] == checkSum(pack,13))
	{
		//MSG("we got a correct packet\r\n");
		//if(hover_set == FALSE)
		if(datalink.link_connected == FALSE)
			datalink.link_connected = TRUE;
		if(datalink.linked == FALSE)
			datalink.linked = TRUE;
		//get timeout stamp
		datalink.connected_time = datalink.connected_tick +10;
		
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

