/********************************** (C) COPYRIGHT *******************************
 * File Name          : Internal_Flash.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2026/08/19
 * Description        : Internal Flash program
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32l103.h"
#include "Internal_Flash.h"
#include "ch32l103_flash.h"

/*********************************************************************
 * @fn      FLASH_BufLoad
 *
 * @brief   Flash Buffer load(4Byte).
 *
 * @param   Address - specifies the address to be programmed.
 *          Data0 - specifies the data0 to be programmed.
 *
 * @return  none
 */
void IFlash_Prog_512(uint32_t address,uint32_t *pbuf)
{
    uint8_t i, j;
    if (address < IFLASH_UDISK_START_ADDR || (address + 511) > IFLASH_UDISK_END_ADDR )
    {
        printf("Error Address %x\n",address);
        return;
    }
    address &= 0x00FFFFFF;
    address |= 0x08000000;
    NVIC_DisableIRQ(USBFS_IRQn);

    FLASH_ROM_ERASE(address, 512);
    FLASH_ROM_WRITE(address, pbuf, 512);

    NVIC_EnableIRQ(USBFS_IRQn);
}

