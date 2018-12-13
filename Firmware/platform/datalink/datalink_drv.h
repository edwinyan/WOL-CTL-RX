#ifndef _DATALINK_DRV_H_
#define _DATALINK_DRV_H_

typedef struct {
	u8 index;
	u32 connected_time;		//����ʱ���
	bool link_connected; 	//����״̬
	bool linked;			//�Ƿ����ӳɹ���
	bool protected;			//�Ƿ񴥷�ʧ�ر���
	bool hover_set;			//ʧ�ر�����λֵ����״̬λ
	bool hover_state;		//��ͣ״̬��־λ
	u32 hover_time;			//��ͣʱ��
	u8 hover_recovery;		//��ͣ�ָ�ʱ��
	bool returned;			//������־λ
	u8 back_recovery;		//�����ָ�ʱ��
	u16 hover_pwm_value[4];	//ʧ�ر�����λֵ
	u32 connected_tick;		//������Ч�ڱ�־λ
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
