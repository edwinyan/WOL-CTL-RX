#ifndef _DATALINK_DRV_H_
#define _DATALINK_DRV_H_

typedef struct {
	u8 index;
	u32 connected_time;		//连接时间戳
	bool link_connected; 	//连接状态
	bool linked;			//是否连接成功过
	bool protected;			//是否触发失控保护
	bool hover_set;			//失控保护中位值保存状态位
	bool hover_state;		//悬停状态标志位
	u32 hover_time;			//悬停时间
	u8 hover_recovery;		//悬停恢复时间
	bool returned;			//返航标志位
	u8 back_recovery;		//返航恢复时间
	u16 hover_pwm_value[4];	//失控保护中位值
	u32 connected_tick;		//连接有效期标志位
}dataLink_t;

//output status of LB2 controller
#define SHUTTLE_ADJ2		0x01
#define SHUTTLE_ADJ1		0x02
#define	SHUTTLE_BUTTON		0x04
#define PHOTO_BUTTON		0x08
#define	PLAYBACK_BUTTON		0x10
#define MODE_SET1			0x20
#define MODE_SET2			0x40
#define VIDEO_BUTTON		0x80

#define POWER_BUTTON		0x01
#define RETURN_RESERVE2		0x02
#define RETURN_TOGGLE		0x04
#define RETURN_RESERVE1		0x08
#define RETURN_BUTTON		0x10

#define RETURN_LED			0x01
#define POWER_LED			0x02

void datalink_init(u8 index);
void data_unpack(void);
bool datalink_received(void);
void get_rssi(uint8_t which);
void datalink_state(void);
void handler_protected(void);

#endif
