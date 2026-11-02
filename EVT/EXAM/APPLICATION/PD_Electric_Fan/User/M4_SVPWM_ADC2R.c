/********************************** (C) COPYRIGHT *******************************
 * File Name          : M4_SVPWM_ADC2R.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2026/10/08
 * Description        : SVPWM parameter calculation, state initialization, related timer and pin configuration,
 *                      ADC parameter calculation, state initialization, related timer and pin configuration,
 *                      zero offset read, ADC trigger time configuration
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Includes------------------------------------------------------------------*/
#include "M0_Control_Library.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* variables ----------------------------------------------------------------*/
SVPWM_2R_Type    SVPWM_Cal_M;     //SVPWM calculation structure
ADCStruc_Type    ADC_M;           //ADC sampling calculation structure
uint16_t AdcTrig;
/*********************************************************************
 * @fn      SVPWM_2R_Paramter_Init
 *
 * @brief   SVPWM parameter initialization
 *
 * @para    SVPWM_Cal: SVPWM calculation structure pointer
 *          Mbase :motor base value parameter structure pointer
 *
 * @return  none
 */
void SVPWM_2R_Paramter_Init(SVPWM_2R_Type *SVPWM_Cal,Motor_Base_Type *Mbase)
{
    SVPWM_Cal->T = ((uint16_t)(TIMER_CLOCK_FREQ_SVPWM/(uint32_t)(FREQ_SVPWM)));
    SVPWM_Cal->HALF_T = SVPWM_Cal->T/2;
    SVPWM_Cal->Quard_T = SVPWM_Cal->T/4;

    SVPWM_Cal->coef1 = _IQ16mpy(_IQ16div((_IQ16(SVPWM_Cal->T))/2,(_IQ16div(DCBUS_RATED_VOLT,Mbase->Vbase))),Three_Q16);
    SVPWM_Cal->coef2 = _IQ16mpy(_IQ16div((_IQ16(SVPWM_Cal->T))/2,(_IQ16div(DCBUS_RATED_VOLT,Mbase->Vbase))),SQRT3_Q16);

    SVPWM_Cal->Maxmodulesqure = MAX_MODULATIONSQUARE;
    SVPWM_Cal->Maxmodule = MAX_MODULATION;

    SVPWM_Cal->OPA_POLL_EN = 0x4101;//4101

//  P4 P5 channel polling
//  SVPWM_Cal->DMA_Trig1 = SVPWM_Cal_M.HALF_T-9.5*(SYSTEM_CLOCK_FREQ/1000000);//Polling interval 2us, switching frequency 15k

//  P3 P4 channel polling
    SVPWM_Cal->DMA_Trig1 = SVPWM_Cal_M.HALF_T-7.5*(SYSTEM_CLOCK_FREQ/1000000);//Polling interval 2us, switching frequency 15k
}

/*********************************************************************
 * @fn      SVPWM_Status_Initial_M
 *
 * @brief   SVPWM status initialization
 *
 * @para    SVPWM_Cal: SVPWM calculation structure pointer
 *
 * @return  none
 */
void SVPWM_2R_Status_Init_M(SVPWM_2R_Type *SVPWM_Cal)
{
    TIM1->CH1CVR = SVPWM_Cal->Quard_T;
    TIM1->CH2CVR = SVPWM_Cal->Quard_T;
    TIM1->CH3CVR = SVPWM_Cal->Quard_T;

    SVPWM_Cal->cc1Load = SVPWM_Cal->Quard_T;
    SVPWM_Cal->cc2Load = SVPWM_Cal->Quard_T;
    SVPWM_Cal->cc3Load = SVPWM_Cal->Quard_T;

    SVPWM_Cal->sector = 1;
    SVPWM_Cal->VoltLimMark = 0;
}

/*********************************************************************
 * @fn      ADC_Parameter_Init
 *
 * @brief   ADC parameter calculation
 *
 * @para    ADCStruc: ADC sampling calculation structure pointer
 *          Mbase :motor base value parameter structure pointer
 *
 * @return  none
 */
void ADC_Parameter_Init (ADCStruc_Type *ADCStruc,Motor_Base_Type *Mbase)
{
    ADCStruc->RatedCurr_ConvInv = _IQ16mpyIQX(_IQ16div(_IQ16(128),_IQ16(32767)),\
                            16,_IQ16div(CURR_SAMPLE_RANGE_ADC,Mbase->Ibase),16);

    ADCStruc->DCVoltRatedADCInv =_IQ16div(_IQ16(1.0),(_IQ16div(_IQ16mpy(DCVOLT_CONV_RATIO,DCBUS_RATED_VOLT),_IQ16(3.3))*4096));

    ADCStruc->DCVoltAverSizeInv1 = _IQ(1.0)/DCVOLT_AVER_SIZE;
    ADCStruc->DCVoltAverSizeInv2 = _IQ(1.0) - ADCStruc->DCVoltAverSizeInv1;

}
/*********************************************************************
 * @fn      SVPWM_2R_Timer_Configure
 *
 * @brief   SVPWM timer and PWM ports initialization
 *
 * @return  none
 */
void SVPWM_2R_Timer_Configure(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure={0};
    TIM_TimeBaseInitTypeDef TIM3_TimeBaseInitStructure= {0};
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_OCInitTypeDef TIM3_OCInitStructure= {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure={0};
    GPIO_InitTypeDef GPIO_InitStructure={0};
    DMA_InitTypeDef DMA1_InitStructure = {0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOB\
                          | RCC_PB2Periph_AFIO | RCC_PB2Periph_TIM1, ENABLE);

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_TIM2|RCC_PB1Periph_TIM3, ENABLE);
    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10  ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
//    GPIO_ResetBits(GPIOB,GPIO_Pin_12);

    TIM_DeInit(TIM1);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Prescaler = TIMER_PRESCALER_SVPWM;

    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;

    TIM_TimeBaseStructure.TIM_Period = SVPWM_Cal_M.HALF_T-1;

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseStructure.TIM_RepetitionCounter = TIMER_UPDATE_RATE_SVPWM;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;

    TIM_OCInitStructure.TIM_Pulse = SVPWM_Cal_M.HALF_T/2;

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //Positive logic
//    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;    //Negative logic

    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;

    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;   //Positive logic
//    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;    //Negative logic

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_Pulse = SVPWM_Cal_M.HALF_T-2;

    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;

    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;

    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;

    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;

    TIM_BDTRInitStructure.TIM_DeadTime = DEADTIME_CONFIGURE;

    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;

//    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;

    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;

    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);

    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
    TIM_SetCounter(TIM1,0);
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_ITConfig(TIM1, TIM_IT_CC4,DISABLE);

    DMA_DeInit(DMA1_Channel6);
    DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA1_InitStructure.DMA_BufferSize = 1;
    DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA1_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA1_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(OPA->CFGR1);
    DMA1_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&(SVPWM_Cal_M.OPA_POLL_EN);
    DMA_Init(DMA1_Channel6, &DMA1_InitStructure);


    DMA_DeInit(DMA1_Channel2);
    DMA1_InitStructure.DMA_PeripheralBaseAddr = (u32)&TIM3->CH4CVR;
    DMA1_InitStructure.DMA_MemoryBaseAddr = (u32)&AdcTrig;
    DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA1_InitStructure.DMA_BufferSize = 1;
    DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA1_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA1_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA1_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA1_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA1_InitStructure);


    DMA_Cmd(DMA1_Channel6, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    TIM_DeInit(TIM3);
    TIM_TimeBaseStructInit(&TIM3_TimeBaseInitStructure);
    TIM3_TimeBaseInitStructure.TIM_Prescaler = TIMER_PRESCALER_SVPWM;
    TIM3_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM3_TimeBaseInitStructure.TIM_Period = SVPWM_Cal_M.T-1;
    TIM_TimeBaseInit(TIM3, &TIM3_TimeBaseInitStructure);

    TIM_OCStructInit(&TIM3_OCInitStructure);
    TIM3_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM3_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM3_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM3_OCInitStructure);
    TIM_OC3Init(TIM3, &TIM3_OCInitStructure);
    TIM_OC4Init(TIM3, &TIM3_OCInitStructure);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);


    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR0);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);

    TIM_SetCounter(TIM3,0);
}

/*********************************************************************
 * @fn      ADC_OPA_Configure
 *
 * @brief   ADC and OPA configuration
 *
 * @para    ADCStruc: ADC sampling calculation structure pointer
 *
 * @return  none
 */
void ADC_OPA_Configure(ADCStruc_Type *ADCStruc)
{
    ADC_InitTypeDef ADC_InitStructure={0};
    GPIO_InitTypeDef GPIO_InitStructure={0};
//    OPA_InitTypeDef  OPA_InitStructure = {0};
    CMP_InitTypeDef CMP_InitTypeDef={0};

    OPCM_Unlock();
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);//ADC clock is PCLK2 divided by 2, 24M

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB|RCC_PB2Periph_GPIOC\
                          |RCC_PB2Periph_AFIO | RCC_PB2Periph_ADC1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    CMP_InitTypeDef.CMP_NUM = CMP3;
    CMP_InitTypeDef.Mode = OUT_IO1;
    CMP_InitTypeDef.NSEL = CMP_CHN1;
    CMP_InitTypeDef.PSEL = CMP_CHP2;
    OPA_CMP_Init(&CMP_InitTypeDef);
    OPA_CMP_Cmd(CMP3,ENABLE);

    // PA0:OPA_P4 PA4:OPA_O3 PA5:OPA_N3 PA7:OPA_P3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1,ADC_Channel_3, 1, ADC_SampleTime_CyclesMode3);

    ADC_Cmd(ADC1, ENABLE);

    ADC_DutyDelayCmd(ADC1,ENABLE);
    ADC_Sample_ModeConfig(ADC1,ADC_Sample_Over_1M_Mode);

}

/******************************************************************************
* Function Name  : ADC_Trigger
* Description    : ADC trigger timing calculation
* Input          : SVPWM calculation structure pointer, ADC register pointer, timer pointer
* Output         : None
* Return         : None
******************************************************************************/
void ADC_Trigger(SVPWM_2R_Type *SVPWM_Cal)
{
    TIM3->CH4CVR = SVPWM_Cal->HALF_T-2;
    TIM3->CH3CVR = SVPWM_Cal->HALF_T-2;

    AdcTrig=SVPWM_Cal->HALF_T-2+2*(SYSTEM_CLOCK_FREQ/1000000);
}

/******************************************************************************
* Function Name  : DCBUS_Volt_Cal
* Description    : Bus voltage calculation
* Input          : ADC calculation structure pointer, ADC register pointer
* Output         : None
* Return         : None
******************************************************************************/
void DCBUS_Volt_Cal(ADCStruc_Type *ADCStruc,ADC_TypeDef *ADCx)
{
    if((ADCx->STATR & ADC_FLAG_EOC) == ADC_FLAG_EOC)
   {
        ADCStruc->DCVoltPres = _IQ16toIQ(_IQ16MPY_MACRO(_IQ16(ADCx->RDATAR>>4),ADCStruc->DCVoltRatedADCInv));
//        ADCStruc->DCVoltPres = _IQ(1.0);
        ADCStruc->DCVoltAver = _IQMPY_MACRO(ADCStruc->DCVoltAver,ADCStruc->DCVoltAverSizeInv2)\
                                + _IQMPY_MACRO(ADCStruc->DCVoltPres,ADCStruc->DCVoltAverSizeInv1);

        ADCStruc->DCVoltcali= (_IQdiv(_IQ(1.0),ADCStruc->DCVoltAver));

   }
}
/*********************************************************************
 * @fn      ADC_1R_Offset_Reading
 *
 * @brief   Current sample static offset calculation
 *
 * @para    ADCStruc: ADC sampling calculation structure pointer
 *          ADCx:ADC register pointer
 *          ADC_num: ADC number
 *
 * @return  none
 */
void ADC_Offset_Reading(ADC_TypeDef *ADCx, ADCStruc_Type *ADCStruc,u_int8_t ADC_num)
{
    uint16_t i;
    OPA_InitTypeDef  OPA_InitStructure = {0};
    OPA_Cmd(OPA1,DISABLE);
    OPA_StructInit(&OPA_InitStructure);
    OPA_InitStructure.OPA_NUM = OPA1;
    OPA_InitStructure.NSEL = CHN3;
    OPA_InitStructure.PSEL = CHP3;
    OPA_InitStructure.Mode = OUT_IO_OUT3;
    OPA_Init(&OPA_InitStructure);
    OPA_Cmd(OPA1,ENABLE);

    Time_Wait(4);

    ADCStruc->OffsetValue1 = 0;
    ADCStruc->OffsetValue2 = 0;

    ADC_ITConfig(ADCx, ADC_IT_JEOC, DISABLE);

    ADC_ExternalTrigInjectedConvConfig(ADCx, ADC_ExternalTrigInjecConv_None);

    ADC_ExternalTrigInjectedConvCmd(ADCx,ENABLE);

    ADC_InjectedSequencerLengthConfig(ADCx,1);

    ADC_InjectedChannelConfig(ADCx, CURR_CHANNEL_PHASEA_M,1,SAMPLING_TIMER_CLOCK);

    ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);

    ADC_SoftwareStartInjectedConvCmd(ADCx,ENABLE);

    for(i=0; i <CURR_OFFSET_READ_NUM_ADC; i++)
    {
         while(!ADC_GetFlagStatus(ADCx,ADC_FLAG_JEOC));
         ADCStruc->OffsetValue1 += (ADC_GetInjectedConversionValue(ADCx,ADC_InjectedChannel_1)>>3);
         ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);
         ADC_SoftwareStartInjectedConvCmd(ADCx,ENABLE);
    }

    while(!ADC_GetFlagStatus(ADCx,ADC_FLAG_JEOC));
    ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);

    OPA_Cmd(OPA1,DISABLE);
    OPA_StructInit(&OPA_InitStructure);
    OPA_InitStructure.OPA_NUM = OPA1;
    OPA_InitStructure.NSEL = CHN3;
    OPA_InitStructure.PSEL = CHP4;
    OPA_InitStructure.Mode = OUT_IO_OUT3;
    OPA_Init(&OPA_InitStructure);
    OPA_Cmd(OPA1,ENABLE);
    Time_Wait(4);
//    ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);

    ADC_SoftwareStartInjectedConvCmd(ADCx,ENABLE);

       for(i=0; i <CURR_OFFSET_READ_NUM_ADC; i++)
       {
            while(!ADC_GetFlagStatus(ADCx,ADC_FLAG_JEOC));
            ADCStruc->OffsetValue2 += (ADC_GetInjectedConversionValue(ADCx,ADC_InjectedChannel_1)>>3);
            ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);
            ADC_SoftwareStartInjectedConvCmd(ADCx,ENABLE);
       }
       while(!ADC_GetFlagStatus(ADCx,ADC_FLAG_JEOC));

       ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);

       OPA_Cmd(OPA1,DISABLE);
       OPA_StructInit(&OPA_InitStructure);
       OPA_InitStructure.OPA_NUM = OPA1;
       OPA_InitStructure.NSEL = CHN3;
       OPA_InitStructure.PSEL = CHP0;
       OPA_InitStructure.Mode = OUT_IO_OUT3;
       OPA_InitStructure.POLL_NUM = CHP_POLL_NUM_5;
       OPA_InitStructure.PSEL_POLL = CHP_OPA1_OFF;
       OPA_InitStructure.OPA_POLL_Interval = 1;
       OPA_InitStructure.CNT_IE = CNT_IE_OFF;
       OPA_Init(&OPA_InitStructure);
       OPA_Cmd(OPA1,ENABLE);
       Time_Wait(4);

       ADC_InjectedSequencerLengthConfig(ADC1, 2);
       ADC_InjectedChannelConfig(ADC1, CURR_CHANNEL_PHASEA_M, 1, SAMPLING_TIMER_CLOCK);
       ADC_InjectedChannelConfig(ADC1, CURR_CHANNEL_PHASEA_M, 2, SAMPLING_TIMER_CLOCK);
       ADC_DiscModeChannelCountConfig(ADC1, 1);
       ADC_InjectedDiscModeCmd(ADC1, ENABLE);
       ADC_ClearFlag(ADCx, ADC_FLAG_JEOC);
}

/*********************************************************************
 * @fn      ADC_1R_Start
 *
 * @brief   ADC conversion enable
 *
 * @param   NewState - ENABLE or DISABLE.
 *
 * @return  none
 */
void ADC_Start (FunctionalState cmd)
{
    if (cmd == ENABLE)
    {
        ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T3_CC4);

        TIM3->CH1CVR = SVPWM_Cal_M.DMA_Trig1;

        TIM_DMACmd(TIM3,TIM_DMA_CC1,ENABLE);
        TIM_DMACmd(TIM3,TIM_DMA_CC3,ENABLE);

        ADC1->STATR = ~(uint32_t)ADC_FLAG_JEOC;

        ADC1->CTLR1 |= (1<<7);

    }
    else
    {
        ADC_InjectedDiscModeCmd(ADC1, DISABLE);
        ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);

        ADC1->CTLR1 &= ~(1<<7);

        ADC1->STATR = ~(uint32_t)ADC_FLAG_JEOC;

        TIM_DMACmd(TIM3,TIM_DMA_CC1,DISABLE);

        TIM_DMACmd(TIM3,TIM_DMA_CC3,DISABLE);

        OPA->CFGR1 &= ~(uint16_t)(1<<0);
        ADC_ExternalTrigInjectedConvCmd(ADC1,DISABLE);

        TIM_CtrlPWMOutputs(TIM1, DISABLE);

        ADC_InjectedDiscModeCmd(ADC1, DISABLE);
    }
}

/*********************************************************************
 * @fn      SVPWM_Lowside_Effective
 *
 * @brief   All low side MOS on, effective immediately
 *
 * @para    SVPWM_Cal: SVPWM calculation structure pointer
 *
 * @return  none
 */
void SVPWM_Lowside_Effective(SVPWM_2R_Type *SVPWM_Cal)
{
    SVPWM_Cal->cc1Load = 0;
    SVPWM_Cal->cc2Load = 0;
    SVPWM_Cal->cc3Load = 0;
}

/*********************************************************************
 * @fn      SVPWM_Zerovolt_Output
 *
 * @brief   0 voltage output, effective immediately
 *
 * @para    SVPWM_Cal: SVPWM calculation structure pointer
 *
 * @return  none
 */
void SVPWM_Zerovolt_Output(SVPWM_2R_Type *SVPWM_Cal)
{
    SVPWM_Cal->cc1Load = SVPWM_Cal->Quard_T;
    SVPWM_Cal->cc2Load = SVPWM_Cal->Quard_T;
    SVPWM_Cal->cc3Load = SVPWM_Cal->Quard_T;
}

/*********************************************************************
 * @fn      SVPWM_Update
 *
 * @brief   Time comparison value updated
 *
 * @para    SVPWM_Cal: SVPWM calculation structure pointer
 *
 * @return  none
 */
void SVPWM_Update (SVPWM_2R_Type *SVPWM_Cal)
{
    TIM1->CH1CVR = SVPWM_Cal->cc1Load;
    TIM1->CH2CVR = SVPWM_Cal->cc2Load;
    TIM1->CH3CVR = SVPWM_Cal->cc3Load;
}


