/********************************** (C) COPYRIGHT *******************************
* File Name          : system_ch32f20x.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/08/08
* Description        : CH32F20x Device Peripheral Access Layer System Source File.
*                      For CH32F208 HSE = 32Mhz
*                      For others   HSE = 8Mhz
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*********************************************************************************/
#include "ch32f20x.h"
#include "debug.h"
/*
* Uncomment the line corresponding to the desired System clock (SYSCLK) frequency (after
* reset the HSI is used as SYSCLK source).
* If none of the define below is enabled, the HSI is used as System clock source.
*/

#define SYSCLK_FREQ_HSI    HSI_VALUE





/* Uncomment the following line if you need to relocate your vector Table in Internal SRAM */
/* #define VECT_TAB_SRAM */

/* Vector Table base offset field This value must be a multiple of 0x200 */
#define VECT_TAB_OFFSET  0x0

/* Clock Definitions */

uint32_t SystemCoreClock         = SYSCLK_FREQ_HSI;              /* System Clock Frequency (Core Clock) */


__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/* system_private_function_proto_types */               
static void SetSysClock( void );



/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   Setup the microcontroller system Initialize the Embedded Flash Interface,
 *        the PLL and update the SystemCoreClock variable.
 *
 * @return  none
 */
void SystemInit( void )
{
    RCC->CTLR |= ( uint32_t )0x00000001;

    RCC->CFGR0 &= ( uint32_t )0xF0FF0000;

    RCC->CTLR &= ( uint32_t )0xFEF6FFFF;
    RCC->CTLR &= ( uint32_t )0xFFFBFFFF;
    RCC->CFGR0 &= ( uint32_t )0xFF00FFFF;
#ifdef CH32F20x_D8C
    RCC->CTLR &= ( uint32_t )0xEBFFFFFF;
    RCC->INTR = 0x00FF0000;
    RCC->CFGR2 = 0x00000000;
#else
    RCC->INTR = 0x009F0000;
#endif

    SetSysClock();

#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}

/*********************************************************************
 * @fn      SystemCoreClockUpdate
 *
 * @brief   Update SystemCoreClock variable according to Clock Register Values.
 *
 * @return  none
 */
void SystemCoreClockUpdate( void )
{
    uint32_t tmp = 0, pllmull = 0, pllsource = 0;
	  uint8_t Pll_6_5 = 0;
	
#if defined (CH32F20x_D8C)	
   	uint8_t Pll2mull = 0;
	
#endif	

    tmp = RCC->CFGR0 & RCC_SWS;

    switch( tmp )
    {
        case 0x00:
            SystemCoreClock = HSI_VALUE;
            break;
        case 0x04:
            SystemCoreClock = HSE_VALUE;
            break;
        case 0x08:
            pllmull = RCC->CFGR0 & RCC_PLLMULL;
            pllsource = RCC->CFGR0 & RCC_PLLSRC;
            pllmull = ( pllmull >> 18 ) + 2;

#if defined (CH32F20x_D6) || defined (CH32F20x_D8) || defined (CH32F20x_D8W)
            if( pllmull == 17 )
            {
                pllmull = 18;
            }
#else
            if( pllmull == 2 )
            {
                pllmull = 18;
            }
            if( pllmull == 15 )
            {
                pllmull = 13;  /* *6.5 */
                Pll_6_5 = 1;
            }
            if( pllmull == 16 )
            {
                pllmull = 15;
            }
            if( pllmull == 17 )
            {
                pllmull = 16;
            }
#endif

            if( pllsource == 0x00 )
            {
								if(EXTEN->EXTEN_CTR & EXTEN_PLL_HSI_PRE) SystemCoreClock = HSI_VALUE * pllmull;
								else SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
            }
            else
            {
#if defined (CH32F20x_D8C)						
	          if(RCC->CFGR2 & (1<<16)){ /* PLL2 */
              SystemCoreClock = HSE_VALUE/(((RCC->CFGR2 & 0xF0)>>4) + 1);  /* PREDIV2 */

              Pll2mull = (uint8_t)((RCC->CFGR2 & 0xF00)>>8);

              if(Pll2mull == 0) SystemCoreClock = (SystemCoreClock * 5)>>1;
              else if(Pll2mull == 1) SystemCoreClock = (SystemCoreClock * 25)>>1;
              else if(Pll2mull == 15) SystemCoreClock = SystemCoreClock * 20;
              else  SystemCoreClock = SystemCoreClock * (Pll2mull + 2);

              SystemCoreClock = SystemCoreClock/((RCC->CFGR2 & 0xF) + 1);  /* PREDIV1 */
          }
          else{/* HSE */
              SystemCoreClock = HSE_VALUE/((RCC->CFGR2 & 0xF) + 1);  /* PREDIV1 */
          }

          SystemCoreClock = SystemCoreClock * pllmull;	
#else
							
#if defined (CH32F20x_D8W)
								if((RCC->CFGR0 & (3<<22)) == (3<<22))
								{
									SystemCoreClock = ((HSE_VALUE>>1)) * pllmull;
								}
								else
#endif							
                if( ( RCC->CFGR0 & RCC_PLLXTPRE ) != ( uint32_t )RESET )
                {
#ifdef CH32F20x_D8W
                    SystemCoreClock = ( ( HSE_VALUE >> 2 ) >> 1 ) * pllmull;
#else
                    SystemCoreClock = ( HSE_VALUE >> 1 ) * pllmull;
#endif
                }
                else
                {
#ifdef CH32F20x_D8W
                    SystemCoreClock = ( HSE_VALUE >> 2 ) * pllmull;
#else
                    SystemCoreClock = HSE_VALUE * pllmull;
#endif

                }
#endif								
            }

            if( Pll_6_5 == 1 ) SystemCoreClock = ( SystemCoreClock / 2 );

            break;
        default:
            SystemCoreClock = HSI_VALUE;
            break;
    }

    tmp = AHBPrescTable[( ( RCC->CFGR0 & RCC_HPRE ) >> 4 )];
    SystemCoreClock >>= tmp;
}



/*********************************************************************
 * @fn      SetSysClock
 *
 * @brief   Configures the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers.
 *
 * @return  none
 */
static void SetSysClock( void )
{
    //GPIO_IPD_Unused();

    /* If none of the define above is enabled, the HSI is used as System clock
     * source (default after reset)
    */
}







