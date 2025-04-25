/********************************** (C) COPYRIGHT  *******************************
* File Name          : ch32f20x_dbgmcu.c
* Author             : WCH
* Version            : V1.0.1
* Date               : 2025/04/09
* Description        : This file provides all the DBGMCU firmware functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32f20x_dbgmcu.h"

#define IDCODE_DEVID_MASK    ((uint32_t)0x0000FFFF)

/*********************************************************************
 * @fn      DBGMCU_GetREVID
 *
 * @brief   Returns the device revision identifier.
 *
 * @return  Revision identifier.
 */
uint32_t DBGMCU_GetREVID( void )
{
    return( DBGMCU->IDCODE & IDCODE_DEVID_MASK );
}

/*********************************************************************
 * @fn      DBGMCU_GetDEVID
 *
 * @brief   Returns the device identifier.
 *
 * @return  Device identifier.
 */
uint32_t DBGMCU_GetDEVID( void )
{
    return( DBGMCU->IDCODE >> 16 );
}

/*********************************************************************
 * @fn      DBGMCU_Config
 *
 * @brief   Configures the specified peripheral and low power mode behavior
 *        when the MCU under Debug mode.
 *
 * @param    DBGMCU_Periph - specifies the peripheral and low power mode.
 *                    DBGMCU_SLEEP- Keep debugger connection during SLEEP mode
 *                    DBGMCU_STOP - Keep debugger connection during STOP mode
 *                    DBGMCU_STANDBY - Keep debugger connection during STANDBY mode
 *                    DBGMCU_IWDG_STOP - Debug IWDG stopped when Core is halted
 *                    DBGMCU_WWDG_STOP -  Debug WWDG stopped when Core is halted
 *                    DBGMCU_TIM1_STOP - TIM1 counter stopped when Core is halted
 *                    DBGMCU_TIM2_STOP - TIM2 counter stopped when Core is halted
 *                    DBGMCU_TIM3_STOP - TIM3 counter stopped when Core is halted
 *                    DBGMCU_TIM4_STOP - TIM4 counter stopped when Core is halted
 *                    DBGMCU_CAN1_STOP - Debug CAN2 stopped when Core is halted
 *                    DBGMCU_I2C1_SMBUS_TIMEOUT - I2C1 SMBUS timeout mode stopped when Core is halted
 *                    DBGMCU_I2C2_SMBUS_TIMEOUT - I2C2 SMBUS timeout mode stopped when Core is halted
 *                    DBGMCU_TIM5_STOP - TIM5 counter stopped when Core is halted
 *                    DBGMCU_TIM6_STOP - TIM6 counter stopped when Core is halted
 *                    DBGMCU_TIM7_STOP - TIM7 counter stopped when Core is halted
 *                    DBGMCU_TIM8_STOP - TIM8 counter stopped when Core is halted
 *                    DBGMCU_CAN2_STOP - Debug CAN2 stopped when Core is halted
 *                    DBGMCU_TIM9_STOP - TIM9 counter stopped when Core is halted
 *                    DBGMCU_TIM10_STOP - TIM10 counter stopped when Core is halted
 *                  NewState - ENABLE or DISABLE.
 *
 * @return  none
 */
void DBGMCU_Config( uint32_t DBGMCU_Periph, FunctionalState NewState )
{
    if( NewState != DISABLE )
    {
        DBGMCU->CFGR |= DBGMCU_Periph;
    }
    else
    {
        DBGMCU->CFGR &= ~DBGMCU_Periph;
    }
}
/*********************************************************************
 * @fn      DBGMCU_GetCHIPID
 *
 * @brief   Returns the CHIP identifier.
 *
 * @return Device identifier.
 *          ChipID List-
 *          CH32F207VCT6-0x207004x8
 *          CH32F205RBT6-0x205004x8
 *          CH32F203VCT6-0x203004x4
 *          CH32F203RCT6-0x203104x4
 *          CH32F203CBT6-0x203204x4
 *          CH32F203RBT6-0x203304x4
 *          CH32F203C8U6-0x203004x0
 *          CH32F203C8T6-0x203104x0
 *          CH32F203K8T6-0x203204x0
 *          CH32F203C6T6-0x203304x0
 *          CH32F208WBU6-0x208004xC
 *          CH32F208RBT6-0x208104xC
 */
uint32_t DBGMCU_GetCHIPID( void )
{
    return( *( uint32_t * )0x1FFFF704 );
}













