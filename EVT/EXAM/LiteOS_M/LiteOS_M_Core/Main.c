/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "debug.h"
#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_compiler.h"
#include "los_exc_info.h"

__attribute__((aligned (8))) UINT8 g_memStart[LOSCFG_SYS_HEAP_SIZE];


/*********************************************************************
 * @fn      taskSampleEntry2
 *
 * @brief   taskSampleEntry2 program.
 *
 * @return  none
 */
VOID taskSampleEntry2(VOID)
{
		while(1) {
			LOS_TaskDelay(2000);
			printf("taskSampleEntry2 running\r\n");
		}
}

/*********************************************************************
 * @fn      taskSampleEntry1
 *
 * @brief   taskSampleEntry1 program.
 *
 * @return  none
 */
VOID taskSampleEntry1(VOID)
{
		while(1) {
			LOS_TaskDelay(1000);
			printf("taskSampleEntry1 running\r\n");
		}
}


/*********************************************************************
 * @fn      taskSample
 *
 * @brief   taskSample program.
 *
 * @return  none
 */
UINT32 taskSample(VOID)
{
		UINT32  uwRet;
		UINT32 taskID1,taskID2;
		TSK_INIT_PARAM_S stTask={0};
		stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)taskSampleEntry1;
		stTask.uwStackSize  = 0X500;
		stTask.pcName       = "taskSampleEntry1";
		stTask.usTaskPrio   = 6;/* high priority */
		uwRet = LOS_TaskCreate(&taskID1, &stTask);
		if (uwRet != LOS_OK) {
				printf("create task1 failed\n");
		}

		stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)taskSampleEntry2;
		stTask.uwStackSize  = 0X500;
		stTask.pcName       = "taskSampleEntry2";
		stTask.usTaskPrio   = 7;/* low priority */
		uwRet = LOS_TaskCreate(&taskID2, &stTask);
		if (uwRet != LOS_OK) {
				printf("create task2 failed\n");
		}
		return LOS_OK;
}


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	unsigned int ret;
	SystemCoreClockUpdate();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );	

	ret = LOS_KernelInit();
	taskSample();
	if (ret == LOS_OK)
	{
		LOS_Start();
	}
	printf("should not run at here!\r\n");

	while(1)
	{
	
	}
}



