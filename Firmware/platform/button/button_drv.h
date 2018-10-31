#ifndef _BUTTON_DRV_H_
#define _BUTTON_DRV_H_

typedef enum{
    BUTTON_SRC_START = 0,
    BUTTON_SRC_POWER_CTRL2 = BUTTON_SRC_START,    //¶ÔÂë°´¼ü
    BUTTON_SRC_POWER_CTRL1,
	BUTTON_SRC_RETURN_LED,
	
	BUTTON_SRC_NUM
}button_src_enum;

void button_drv_init(void);
u8 button_value_get(button_src_enum src);

#endif
