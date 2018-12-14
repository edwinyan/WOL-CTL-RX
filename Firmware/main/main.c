#include "common.h"
#include "bsp.h"
#include "bsp_os.h"
#include "led_drv.h"
#include "tc_common.h"
#include "uart_drv.h"
#include "datalink_drv.h"

OS_MUTEX	TX_MUTEX;		//uart tx mutex
OS_MUTEX	RX_MUTEX;		//uart rx mutex

OS_MUTEX	FIFO_MUTEX;

//FIFO_T stFiFo;

/*----------------------------------------------------------------------------*/
//macro and variables
#define  APP_CFG_TASK_START_STK_SIZE			256u
#define  APP_CFG_TASK_START_PRIO				2u

#define  APP_RX_TASK_STK_SIZE					256u
#define  APP_RX_TASK_PRIO                       3u

//#define  APP_UART_TASK_STK_SIZE                 256u
//#define  APP_UART_TASK_PRIO                     4u

#define  APP_DATALINK_TASK_STK_SIZE             256u
#define  APP_DATALINK_TASK_PRIO                 5u

#define  APP_RSSI_TASK_STK_SIZE                 256u
#define  APP_RSSI_TASK_PRIO                     15u

static  OS_TCB   app_task_start_tcb;
static  CPU_STK  app_task_start_stk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   app_rx_task_tcb;
static  CPU_STK  app_rx_task_stk[APP_RX_TASK_STK_SIZE];

//static  OS_TCB   app_uart_task_tcb;
//static  CPU_STK  app_uart_task_stk[APP_UART_TASK_STK_SIZE];

static  OS_TCB   app_datalink_task_tcb;
static  CPU_STK  app_datalink_task_stk[APP_DATALINK_TASK_STK_SIZE];

static  OS_TCB   app_rssi_task_tcb;
static  CPU_STK  app_rssi_task_stk[APP_RSSI_TASK_STK_SIZE];

/*----------------------------------------------------------------------------*/
//local function
STATIC void app_rfm_rx_task(void *p_arg)
{
	OS_ERR      err;

	(void)p_arg;
	
	//MSG("Creating Application Tasks: %d\r\n",__FPU_USED);

	while (DEF_TRUE) 
    {   
		OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}
#if 0
STATIC void app_uart_task(void *p_arg)
{
	OS_ERR      err;
	u8 data=0;

	(void)p_arg;
	
	Fifo_Init(&stFiFo);

	while (DEF_TRUE) 
    {   
		//MSG("-----------uart-----------@%d\r\n",OSTimeGet(&err));
        #if 1
		if(1 == uart_drv_data_recv(&data,1))
		{
			Fifo_Write(&stFiFo,data);
			//MSG("%c",data);
		}
		//MSG("\r\n");
		#endif
		OSTimeDlyHMSM(0, 0, 0, 3, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}
#endif

STATIC void app_datalink_task(void *p_arg)
{
	OS_ERR      err;

	(void)p_arg;

	datalink_init(0);

	while (DEF_TRUE) 
    {   
		LED_B_OFF;
		if(datalink_received())
		{
			data_unpack();
		}

		handler_protected();
		
		OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

STATIC void app_rssi_task(void *p_arg)
{
	OS_ERR      err;
	static uint8_t which=0;
	(void)p_arg;

	while (DEF_TRUE) 
    {   
		get_rssi(which++);
		//MSG("which=%d\r\n",which);
		OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

STATIC void app_task_create(void)
{

	OS_ERR      err;
#if 1	
    OSTaskCreate((OS_TCB       *)&app_rx_task_tcb,              
                 (CPU_CHAR     *)"App rmf rx Task",
                 (OS_TASK_PTR   )app_rfm_rx_task, 
                 (void         *)0,
                 (OS_PRIO       )APP_RX_TASK_PRIO,
                 (CPU_STK      *)&app_rx_task_stk[0],
                 (CPU_STK_SIZE  )app_rx_task_stk[APP_RX_TASK_STK_SIZE / 10],
                 (CPU_STK_SIZE  )APP_RX_TASK_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
#endif

#if 0
	OSTaskCreate((OS_TCB	   *)&app_uart_task_tcb,			  
				 (CPU_CHAR	   *)"App uart Task",
				 (OS_TASK_PTR	)app_uart_task, 
				 (void		   *)0,
				 (OS_PRIO		)APP_UART_TASK_PRIO,
				 (CPU_STK	   *)&app_uart_task_stk[0],
				 (CPU_STK_SIZE	)app_uart_task_stk[APP_UART_TASK_STK_SIZE / 10],
				 (CPU_STK_SIZE	)APP_UART_TASK_STK_SIZE,
				 (OS_MSG_QTY	)0,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err);
#endif
#if 1

	OSTaskCreate((OS_TCB	   *)&app_datalink_task_tcb,			  
				 (CPU_CHAR	   *)"App datalink Task",
				 (OS_TASK_PTR	)app_datalink_task, 
				 (void		   *)0,
				 (OS_PRIO		)APP_DATALINK_TASK_PRIO,
				 (CPU_STK	   *)&app_datalink_task_stk[0],
				 (CPU_STK_SIZE	)app_datalink_task_stk[APP_DATALINK_TASK_STK_SIZE / 10],
				 (CPU_STK_SIZE	)APP_DATALINK_TASK_STK_SIZE,
				 (OS_MSG_QTY	)0,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err);
#endif

	OSTaskCreate((OS_TCB	   *)&app_rssi_task_tcb,			  
				 (CPU_CHAR	   *)"App rssi Task",
				 (OS_TASK_PTR	)app_rssi_task, 
				 (void		   *)0,
				 (OS_PRIO		)APP_RSSI_TASK_PRIO,
				 (CPU_STK	   *)&app_rssi_task_stk[0],
				 (CPU_STK_SIZE	)app_rssi_task_stk[APP_RSSI_TASK_STK_SIZE / 10],
				 (CPU_STK_SIZE	)APP_RSSI_TASK_STK_SIZE,
				 (OS_MSG_QTY	)0,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err);



}

STATIC void app_task_start(void *p_arg)
{
    OS_ERR      err;
	//u32 i =0;


   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */
    
    BSP_Tick_Init();                                            /* Start Tick Initialization                            */
    
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */

    BSP_Peripheral_Init();

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
    MSG("Creating Application Tasks: %d\r\n",__FPU_USED);
	OSMutexCreate((OS_MUTEX*	)&FIFO_MUTEX,
				  (CPU_CHAR*	)"UART FIFO_MUTEX",
				  (OS_ERR*		)&err);
	OSMutexCreate((OS_MUTEX*	)&TX_MUTEX,
				  (CPU_CHAR*	)"UART TX_MUTEX",
				  (OS_ERR*		)&err);
	OSMutexCreate((OS_MUTEX*	)&RX_MUTEX,
				  (CPU_CHAR*	)"UART RX_MUTEX",
				  (OS_ERR*		)&err);

    app_task_create();                                            

    while (DEF_TRUE) 
    {   
        //tc_run_all();
        //MSG("----------------loop-----------------\r\n");
        //LED_B_ON;
		//OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_STRICT, &err);
		//LED_B_OFF;
		LED_R_TOGGLE;
		datalink_state();
		OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}


/*----------------------------------------------------------------------------*/
int main(void)
{
    OS_ERR err;

    //BSP_IntDisAll();
    OSInit(&err);

    OSTaskCreate((OS_TCB       *)&app_task_start_tcb,              
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )app_task_start, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&app_task_start_stk[0],
                 (CPU_STK_SIZE  )app_task_start_stk[APP_CFG_TASK_START_STK_SIZE / 10],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

    OSStart(&err);                                              
    
    (void)&err;
    
    return 0;
}

