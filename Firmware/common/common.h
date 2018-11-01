#ifndef _COMMON_H_
#define _COMMON_H_

//generic
#include  <stdio.h>
#include  <stdarg.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>

//os relative
#include  <cpu.h>
#include  <cpu_core.h>
#include  <os.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>

//debug
#include "debug_msg.h"

//platform
#include "stm32f4xx.h"

enum SystemState{
	SYSTEM_INIT=0,		//³õÊ¼»¯×´Ì¬
	SYSTEM_FLYING,		//·ÉÐÐ×´Ì¬
	SYSTEM_HOVER,		//ÐüÍ£×´Ì¬
	SYSTEM_BACK,		//·µº½×´Ì¬
	SYSTEM_ERROR		//´íÎó×´Ì¬
};


//misc
#define  STATIC     static
#endif
