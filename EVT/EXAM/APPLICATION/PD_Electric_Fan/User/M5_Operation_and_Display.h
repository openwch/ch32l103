/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_Operation_and_Display.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Definition of relevant operation parameters such as buttons and displays
                        Definition of acceleration and deceleration parameters

*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __MOTOR_OPERATION_AND_DISPLAY_H
#define __MOTOR_OPERATION_AND_DISPLAY_H

/* Includes -----------------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/
typedef enum {PRESSUP = 0, PRESSDOWN = 1} PressStatus;
typedef struct
{
    GPIO_TypeDef *Group;
    uint16_t     Pin;
    uint16_t Effectivetime;
    uint16_t Effectivecnt;
    uint16_t Ineffectivetime;
    uint16_t Ineffectivecnt;
    Truth_Verify_Type Status;
    PressStatus         KeyPress;
} Keyrecover_Type;

typedef struct
{
    Keyrecover_Type RUN;
    Keyrecover_Type Speedadjust;
    Keyrecover_Type oscillate;
    Keyrecover_Type Timeoff;
} Key_Manager_Type;//Recoverable key management structure


typedef struct
{
    uint16_t Effectivecntset1;
    uint16_t Effectivecntset2;
    uint16_t Effectivecnt;
    uint8_t  BuzzMark;
    Truth_Verify_Type  Effective;
    Truth_Verify_Type  VdcFirstPoweron;
    Truth_Verify_Type  VdcPoweron;
} Buzzctrl_Type;

typedef struct
{
    uint16_t StopwaitCnt;
    uint16_t StopwaitSet;

    uint16_t StartbuttonSet;
    uint16_t SpeedbuttonCnt;
    uint16_t SpeedbuttonSetdown;

    uint16_t FaultHoldingSet;
    uint16_t FaultHoldingCnt;
    Truth_Verify_Type Startmark;
    uint8_t Checkmode;
} Operation_Type;

typedef struct
{
    _iq24 Preramp;                  //Pre-ramp reference (unit-rad/s_per)/(unit-W/s_per)
    _iq24 Aftramp;                  //Post-ramp reference (unit-rad/s_per)/(unit-W/s_per)
    _iq24 AccStep;                  //Acceleration step (unit-rad/s^2_per)/(unit-W/s^2_per)
    _iq24 DecStep;                  //Deceleration step (unit-rad/s^2_per)/(unit-W/s^2_per)
    uint8_t  Point_adncnt;
    int8_t   Direction;

    Truth_Verify_Type Startfinish;
    Truth_Verify_Type Decemark;
}Rampctr_Type;
/* Exported constants -------------------------------------------------------*/

#define SPEED_LOOP 0
#define POWER_LOOP 1
#define CLOSE_LOOP  SPEED_LOOP

#define STARTUP_DIRECTION       1                       // Running direction (1: forward, -1: reverse)
#define MAX_ANGSPEED_POS_M      _IQ(1.0)                // Maximum forward speed (unit-per)
#define MAX_ANGSPEED_NEG_M      _IQ(-1.0)               // Maximum reverse speed (unit-per)
#define MIN_ANGSPEED_POS_M      _IQ(0.025)              // Minimum forward speed (unit-per)
#define MIN_ANGSPEED_NEG_M      _IQ(-0.025)             // Minimum reverse speed (unit-per)
#define SPEED_RAMP_FREQ_M       SPEED_LOOP_CAL_FREQ     // Acceleration and deceleration processing frequency (unit-Hz)
#define ACCE_TIME_M             5                       // Acceleration time (unit-s)
#define DECE_TIME_M             5                       // Deceleration time (unit-s)

#define SPEEDPOINT1             _IQ(0.30)
#define SPEEDPOINT2             _IQ(0.50)
#define SPEEDPOINT3             _IQ(0.75)


#define POWERPOINT1             _IQ(0.30)
#define POWERPOINT2             _IQ(0.50)
#define POWERPOINT3             _IQ(0.7)

#define POINT_DEFAULT      1
#define POINT_RANK         3

#define FAST_DECE_SPEED         _IQ(0.25)              // Fast deceleration speed target (unit-per)
#define BREAKSPEED              _IQ(0.10)              // Speed trigger stator short-circuit brake (unit-per)

#define FAST_DECE_POWER         _IQ(0.15)               // Fast deceleration power target (unit-per)
#define BREAKPOWER              _IQ(0.10)               // power trigger stator short-circuit brake (unit-per)

#define BREAKTIME               1.0                     // stator short-circuit brake last time(unit-s)

#define START_FINISH_SPEED      (FAST_DECE_SPEED +_IQ(0.05))//Speed threshold of start finish mark

#define KEY_CHECK_FREQ           SPEED_LOOP_CAL_FREQ    // Frequency of key detection calculation
#define SRKEY_EFFECTIVE_TIME     0.05
#define SRKEY_INEFFECTIVE_TIME   0.05

#define OPERATION_CAL_FREQ      SPEED_LOOP_CAL_FREQ     // Operation execution frequency(unit-Hz)
#define STOP_WAITING_TIME       0.75                     // Wait time before restart (unit-s)
#define FAULT_HOLDING_TIME      0.75                     // Fault status last time before reset(unit-s)

#define POWERVERY_FREQ              SPEED_LOOP_CAL_FREQ // Power-on check frequency (unit-Hz)
#define POWERON_VOLT                _IQ(0.75)           // Power-on voltage threshold (unit-Vper)
#define POWERON_TIME                0.1                 // Power-on verification effective time (unit-s)
#define POWEROFF_VOLT               _IQ(0.65)           // Power-off voltage threshold(unit-Vper)
#define POWERVERY_DCVOLTAVERSIZE    4                   // Power-on check voltage average calculation length

//Buzzer control parameters
#define BUZZ_PROCESS_FREQ           SPEED_LOOP_CAL_FREQ //Determination frequency (unit-Hz)
#define BUZZ_EFFECTIVE_TIME1        0.3                 //Effective time 1 (unit-s)
#define BUZZ_EFFECTIVE_TIME2        0.6                 //Effective time 2 (unit-s)

#define BUZZ_TIMER_PRESCALER      1                     //Buzzer timer prescaler
//Buzzer timer clock frequency (unit-Hz)
#define BUZZ_TIMER_CLOCK_FREQ      (SYSTEM_CLOCK_FREQ/(BUZZ_TIMER_PRESCALER+1))
#define BUZZ_TIMER_FREQ           4000                  //Buzzer toggle frequency (unit-Hz)
//Buzzer timer period
#define BUZZ_TIMER_PERIOD         BUZZ_TIMER_CLOCK_FREQ/BUZZ_TIMER_FREQ


/* Exported variables -------------------------------------------------------*/
extern Rampctr_Type         Ramp_M;
extern Operation_Type       Operation_M;
extern Powercontrol_Type    Powerctr_M;
extern Key_Manager_Type     Key_M;
extern Buzzctrl_Type        BUZZ_M;

/* Exported macro -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------ */
void Motor_Operation_Process(void);

void rampctr_Parameter_Define(Rampctr_Type *Ramp_M);

void rampctr_Initial(Rampctr_Type *Ramp_M);

void Operation_Parameter_Define(Operation_Type *Operation);

void Operation_Status_Init(Operation_Type *Operation);

void Ramp_Set(Rampctr_Type *Speed_Ramp);

void KeyandLED_IOInit(void);
void KeyandLED_Parameter_Define(Key_Manager_Type *KeyM);
void KeyandLED_Status_Init(Key_Manager_Type *KeyM);
void Skeycheck_Recover(Keyrecover_Type *Keyrec);
void KeyCheck_Process(Key_Manager_Type *KeyM, Rampctr_Type *Speed_Ramp);

void Poweroperation_Initial(Powercontrol_Type *PowerOper);

void Poweroperation_Process(Rampctr_Type *SpeedRamp,ADCStruc_Type *ADCStruc,Powercontrol_Type *PowerOper,\
                            Runningstatus_Type *Runningstatus,_iq24 Speedact);

void Break_Verify(Rampctr_Type *SpeedRamp, Runningstatus_Type *Runningstatus);

void Speed_Ramp_Process(Rampctr_Type *Speed_Ramp);

void Power_Ramp_Process(Rampctr_Type *Power_Ramp);

void Time_Delay_Process(void);

void Time_Wait(uint16_t);

void Time_Delay_Set(uint16_t Delayset, uint16_t *Delaycounter);

Truth_Verify_Type Time_Delay_Elapse(uint16_t Delaycounter);

void BUZZ_TIMER_Init(void);
void BUZZ_Parameter_define(Buzzctrl_Type *BUZZCtr);
void BUZZ_PROCESS(Buzzctrl_Type *BUZZCtr);
void PD_Seek(void);
void PD_Process(void);
#endif
