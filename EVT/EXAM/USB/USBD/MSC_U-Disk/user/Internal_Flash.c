/********************************** (C) COPYRIGHT *******************************
 * File Name          : Internal_Flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : Internal Flash program
 *********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32f20x.h"
#include "Internal_Flash.h"
#include "ch32f20x_flash.h"
/*********************************************************************
 * @fn      IFlash_Prog_512
 *
 * @brief   Internal Flash Fast Program 512.
 *
 * @return  none
 */
void IFlash_Prog_512(uint32_t address,uint32_t *pbuf)
{
    if (address < IFLASH_UDISK_START_ADDR || (address + 511) > IFLASH_UDISK_END_ADDR)
    {
        return;
    }
    address &= 0x00FFFFFF;
    address |= 0x08000000;
    NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    FLASH_Unlock_Fast();
    FLASH_ErasePage_Fast(address);
    FLASH_ProgramPage_Fast(address, pbuf);
    FLASH_ErasePage_Fast(address + INTERNAL_FLASH_PAGE_SIZE);
    FLASH_ProgramPage_Fast(address+ INTERNAL_FLASH_PAGE_SIZE, pbuf + INTERNAL_FLASH_PAGE_SIZE/4);
    FLASH_Lock_Fast();
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
}
