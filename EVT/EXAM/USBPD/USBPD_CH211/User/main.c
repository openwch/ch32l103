/********************************** (C) COPYRIGHT *******************************
 * File Name		  : main.c
 * Author			 : WCH
 * Version			: V1.0.0
 * Date			   :
 * Description		: Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*******************************************************************************
 * This routine is used to demonstrate the CH211 Demo board.Users can select
 * the role of the Demo board through the Set_DevChk(), thereby verifying
 * the related functions of the PD and CH211 chip.

 * Schematic Path: EVT\EXAM\USBPD\USBPD_CH211\SCH\USBPD_CH211\USBPD_CH211.SchDoc

*******************************************************************************/




#include "string.h"
#include "debug.h"


#include "PD_DEF.h"


/*********************************************************************
 * @fn      LED_Init
 *
 * @brief   Initializes the LED.
 *
 * @return  none
 */
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	GPIO_SetBits(GPIOA, GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4 );		//LED RGB
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

}

/*********************************************************************
 * @fn      ADC_Function_Init
 *
 * @brief   Initializes the ADC.
 *
 * @return  none
 */
void ADC_Function_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef  ADC_InitStructure = {0};

	//VBUS ADC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_CyclesMode2);

	ADC_Cmd(ADC1, ENABLE);
}

void TIM_Init(void)
{
	NVIC_InitTypeDef        NVIC_InitStructure = {0};

	RCC_PB1PeriphClockCmd(RCC_PB1Periph_LPTIM, ENABLE);
	//Timer 1ms
	LPTIM->CFGR |= LPTIM_CountSource_Internal; 
	LPTIM->CFGR |= LPTIM_InClockSource_HSI;     
	LPTIM->CFGR |= LPTIM_TClockPrescaler_DIV32; 
	LPTIM->ARR = PD_SYS_TIMER;                 

	NVIC_InitStructure.NVIC_IRQChannel = LPTIM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	LPTIM->CR |= LPTIM_CR_ENABLE;        
	LPTIM->CR |= LPTIM_CR_CNTSTRT;          
	LPTIM->ICR |= LPTIM_IT_ARRM;       
	LPTIM->IER |= LPTIM_IT_ARRM;         
}


int main(void)
{

	SystemCoreClockUpdate();
	USART_Printf_Init(460800);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );


	Delay_Init();
	Delay_Ms(500);

	LED_Init();
	I2C_PWR_Init();							//I2C and CH211 initialization
	PD_Init();      					//PD initialization
	ADC_Function_Init();	   //VBUS voltage detection setting
	TIM_Init();
	Set_DevChk( DevRole_Sink );				//Start detecting device plug and unplug


	while (1) {

		if ( PD_DEVICE.ConnectStat == 0 )
		{
			GPIO_ResetBits(GPIOA, LED_B );
            Delay_Ms(500);
			GPIO_SetBits(GPIOA, LED_B );
            Delay_Ms(500);
		}
		else
		{
			GPIO_SetBits(GPIOA, LED_B );
            Delay_Ms(500);
        }

	}
}












