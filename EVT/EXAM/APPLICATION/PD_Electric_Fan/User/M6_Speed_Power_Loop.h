/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __M6_SPEEDLOOP_H
#define __M6_SPEEDLOOP_H

/* Includes -----------------------------------------------------------------*/

/* Exported constants -------------------------------------------------------*/
//Timer clock frequency used by the speed loop (unit-Hz)
#define TIMER_CLOCK_FREQ_SPEED      96000000uL

//Speed loop timer interrupt frequency (unit-Hz)
#define TIMER_FREQ_SPEED_LOOP       2000

//SYS timer period value
#define TIMER_PERIDO_SYSTICK     TIMER_CLOCK_FREQ_SPEED / (u32)(TIMER_FREQ_SPEED_LOOP*8)

//Speed loop calculation frequency (unit-Hz)
#define SPEED_LOOP_CAL_FREQ         2000

//Speed loop calculation period (unit-s)
#define SPEED_LOOP_CAL_PERIOD      _IQ(1.0)/SPEED_LOOP_CAL_FREQ

//Speed loop PID regulator calculation period (x+1)*500usec
#define SPEED_SAMPLE_PID_PERIOD     TIMER_FREQ_SPEED_LOOP/SPEED_LOOP_CAL_FREQ

//PD calculation period
#define PD_PERIOD                        2

/* Exported variables -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/

/* Exported functions -------------------------------------------------------*/
//Speed processing timer initialization
void Systick_Init(void);

//Speed processing timer enable
void SYS_TIM_Cmd(FunctionalState NewState);

#endif
