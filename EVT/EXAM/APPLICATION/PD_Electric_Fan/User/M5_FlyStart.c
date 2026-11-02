/********************************** (C) COPYRIGHT *******************************
 * File Name          : M5_FlyStart.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/11/04
 * Description        : Flystart status initialization,Flystart Switch control
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
/* Includes -----------------------------------------------------------------*/
#include "M0_Control_Library.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private functions --------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Variables ----------------------------------------------------------------*/
Flystart_type  Flystart_M;
/*********************************************************************
 * @fn      Flystart_Prameter_Define
 *
 * @brief   Flystart parameter definition
 *
 * @para    Flystart: Flystart structure pointer
 *
 * @return  none
 */
void Flystart_Prameter_Define(Flystart_type *Flystart)
{
    Flystart->CalT=FLYSTART_PERIOD_M;

    Flystart->K_WetoPos=(Flystart->CalT * MBase_M.Fn);

    Flystart->Direction=_IQ(0.0);

    Flystart->BEMFDetectSet=FLYSTART_VERIFY_TIME_M*FLYSTART_FREQ_M;

    Flystart->BemfwaitSet=FLYSTART_VERIFY_NUM_M;

    Flystart->KVE=_IQdiv(_IQ16toIQ(MBase_M.Vbase),_IQ16toIQ(MBase_M.Ebase));

    Flystart->StillEMF=FLYSTART_STILL_EMFMIN_M;

    Flystart->Stillveri_cntset = (uint16_t)(FLYSTART_STILL_VERIFY_TIME_M * FLYSTART_FREQ_M);

    Flystart->Bemf_Threshold=FLYSTART_BEMF_THRESHOLD_M;
    Flystart->Vq_DirDetectionSet = VQ_ZERODETECTION_EFFSET;

    Flystart->Vq_DirPosiEffSet_thresh  = VQ_ZERO_EFFSET;
    Flystart->Vq_DirNegEffSet_thresh  = VQ_ZERO_EFFSET;

    Flystart->Vq_NegEffSet_thresh  =  VQ_AMPLITUDE_EFFSET;
    Flystart->Vq_PosiEffSet_thresh = VQ_AMPLITUDE_EFFSET;

    Flystart->Pll_Bandwidth=FLYSTARTPLL_BANDWIDTH_M;
    Flystart->Speedboundary = FLYSTART_SPEED_BOUNDARY_M ;

    Flystart->Stillcurrthresh = FLYSTART_STILL_CURR_M;
    Flystart->Stillcurrcntset = (uint16_t)(FLYSTART_STILLTIME * FLYSTART_FREQ_M);
    Flystart->Breakcntset = (uint16_t)(FLYSTART_BREAKTIME * FLYSTART_FREQ_M);

    Flystart->Speed_Upper = FLYSTART_SPEED_MAX_M;
    Flystart->Speed_Lower = FLYSTART_SPEED_MIN_M;

    Flystart->Vq_LPF.Input_Coef = _IQdiv(FLYSTART_EMFLPF_WcT_M,(FLYSTART_EMFLPF_WcT_M + UNIT_Q24));
    Flystart->Vq_LPF.Output_Coef =  _IQdiv(UNIT_Q24,(FLYSTART_EMFLPF_WcT_M + UNIT_Q24));

    Flystart->Vd_LPF.Input_Coef = _IQdiv(FLYSTART_EMFLPF_WcT_M,(FLYSTART_EMFLPF_WcT_M + UNIT_Q24));
    Flystart->Vd_LPF.Output_Coef =  _IQdiv(UNIT_Q24,(FLYSTART_EMFLPF_WcT_M + UNIT_Q24));

    Flystart->Angspeed_LPF.Input_Coef = _IQdiv(FLYSTART_SPEED_LPF_WcT_M,(FLYSTART_SPEED_LPF_WcT_M + UNIT_Q24));
    Flystart->Angspeed_LPF.Output_Coef = _IQdiv(UNIT_Q24,(FLYSTART_SPEED_LPF_WcT_M + UNIT_Q24));
}

/*********************************************************************
 * @fn      Flystart_Status_Init
 *
 * @brief   Flystart status initialization
 *
 * @para    Flystart: Flystart structure pointer
 *
 * @return  none
 */
void Flystart_Status_Init(Flystart_type *Flystart,int8_t Direction)
{
    Flystart->Speedinst=0;
    Flystart->Speed_Filter=0;

    Flystart->Direction=_IQ(0.0);

    Flystart->RAngle_new_phase=0;
    Flystart->EAngle_new_phase=0;

    Flystart->Vd=0;
    Flystart->Vq=0;
    Flystart->Vd_Filter=0;
    Flystart->Vq_Filter=0;

    Flystart->Vs=0;

    Flystart->Vd_LPF.Out_New=0;
    Flystart->Vd_LPF.Out_Pre=0;

    Flystart->Vq_LPF.Out_New=0;
    Flystart->Vq_LPF.Out_Pre=0;

    Flystart->Angspeed_LPF.Out_New=0;
    Flystart->Angspeed_LPF.Out_Pre=0;

    Flystart->BemfSpeed_PLL.Error_New=0;
    Flystart->BemfSpeed_PLL.Error_Prev=0;
    Flystart->BemfSpeed_PLL.Output_New=0;
    Flystart->BemfSpeed_PLL.Output_Prev=0;

    Flystart->EMFP2R.cd=0;
    Flystart->EMFP2R.cq=0;

    Flystart->TriangleR.Cos=0;
    Flystart->TriangleR.Sin=0;

    Flystart->TriangleE.Cos=0;
    Flystart->TriangleE.Sin=0;

    Flystart->Breakcnt=0;
    Flystart->Stillcurrcnt=0;
    Flystart->Stillveri_cnt=0;
    Flystart->Breakcnt=0;

    Flystart->Vq_DirPosiEffcnt=0;
    Flystart->Vq_DirNegEffcnt=0;
    Flystart->Vq_DirDetectionCnt=0;
    Flystart->Vq_PosiEffcnt=0;
    Flystart->Vq_NegEffcnt=0;

    Flystart->BEMFDetectcnt=0;
    Flystart->BemfwaitCnt=0;

    Flystart->Vefify_status=STILL;
    Flystart->Vq_AmplitudeMark=FALSE;
    Flystart->Vq_PosiMark=FALSE;
    Flystart->Vq_NegMark=FALSE;
    Flystart->Vq_DirMark=FALSE;
    Flystart->Process_mark=FALSE;
    Flystart->Switch_mark=FALSE;


    if(Direction==1)
    {
        Flystart->BemfSpeed_PLL.Upper_Lim = Flystart->Speed_Upper;
        Flystart->BemfSpeed_PLL.Lower_Lim = Flystart->Speed_Lower;
    }
    else if(Direction==-1)
    {
        Flystart->BemfSpeed_PLL.Upper_Lim = Flystart->Speed_Lower;
        Flystart->BemfSpeed_PLL.Lower_Lim = Flystart->Speed_Upper;
    }

}

/*********************************************************************
 * @fn      Flystart_Switch
 *
 * @brief   Flystart Switch control
 *
 * @para    Flystart: Flystart structure pointer
 *          SpeedRamp:Acceleration and deceleration control structure pointer
 *
 * @return  none
 */
void Flystart_Switch(Flystart_type *Flystart,Rampctr_Type *Ramp_M)
{
    if(Flystart->Switch_mark == FALSE)
    {
        if(Ramp_M->Direction ==1)
        {
            switch (Flystart->Vefify_status)
            {
                case STILL:
                case POSI_LOW:
                case NEG_HIGH:
                case NEG_LOW:
                    MLBG_M.Cal_Struc.Inimark = FALSE;
                    Flystart->Switch_mark = TRUE;
                    SVPWM_2R_Status_Init_M(&SVPWM_Cal_M);//zero voltage output
                    SVPWM_Lowside_Effective(&SVPWM_Cal_M);//all low-side switches on

                    Mpid_M.dCurr.Kp=Mpid_M.dCurr.Kp>>1;
                    Mpid_M.dCurr.Ki=Mpid_M.dCurr.Ki>>1;

                    Mpid_M.qCurr.Kp=Mpid_M.qCurr.Kp>>1;
                    Mpid_M.qCurr.Ki=Mpid_M.qCurr.Ki>>1;
                    break;
                case POSI_HIGH:
                    MLBG_M.Cal_Struc.Inimark=TRUE;
                    Flystart_CtrLoop_Inital(Flystart,&Mpid_M,&MStruc_M,&MLBG_M,&DCurrProcess_M);//loop initialization
                    Ramp_M->Aftramp = Flystart->Speed_Filter;
                    Flystart->Switch_mark = TRUE;

                    Mpid_M.dCurr.Kp=Mpid_M.dCurr.Kp>>1;
                    Mpid_M.dCurr.Ki=Mpid_M.dCurr.Ki>>1;

                    Mpid_M.qCurr.Kp=Mpid_M.qCurr.Kp>>1;
                    Mpid_M.qCurr.Ki=Mpid_M.qCurr.Ki>>1;
                    RunningStatus_M = RUN;//enter running
                    break;
            }
        }
        else if (Ramp_M->Direction ==-1)
        {
            switch (Flystart->Vefify_status)
            {
                case STILL:
                case POSI_HIGH:
                case POSI_LOW:
                case NEG_LOW:
                    Flystart->Switch_mark = TRUE;
                    MLBG_M.Cal_Struc.Inimark = FALSE;
                    SVPWM_2R_Status_Init_M(&SVPWM_Cal_M);//zero voltage output
                    SVPWM_Lowside_Effective(&SVPWM_Cal_M);

                    //PI restore
                    Mpid_M.dCurr.Kp=Mpid_M.dCurr.Kp>>1;
                    Mpid_M.dCurr.Ki=Mpid_M.dCurr.Ki>>1;

                    Mpid_M.qCurr.Kp=Mpid_M.qCurr.Kp>>1;
                    Mpid_M.qCurr.Ki=Mpid_M.qCurr.Ki>>1;
                    break;

                case NEG_HIGH:
                    Flystart_CtrLoop_Inital(Flystart,&Mpid_M,&MStruc_M,&MLBG_M,&DCurrProcess_M);//loop initialization
#if (CLOSE_LOOP==SPEED_LOOP)
                    Ramp_M->Aftramp = Flystart->Speed_Filter;
#elif (CLOSE_LOOP==POWER_LOOP)
                    Ramp_M->Aftramp = _IQabs(Flystart->Speed_Filter);
#endif
                    Flystart->Switch_mark = TRUE;
                    RunningStatus_M = RUN;//enter running

                    //PI restore
                    Mpid_M.dCurr.Kp=Mpid_M.dCurr.Kp>>1;
                    Mpid_M.dCurr.Ki=Mpid_M.dCurr.Ki>>1;

                    Mpid_M.qCurr.Kp=Mpid_M.qCurr.Kp>>1;
                    Mpid_M.qCurr.Ki=Mpid_M.qCurr.Ki>>1;
                    break;
            }
        }
    }
    else
    {
        if(Ramp_M->Direction ==1)
        {
            if((_IQabs(MStruc_M.C3S.PhaseA)<=Flystart->Stillcurrthresh)&&\
               (_IQabs(MStruc_M.C3S.PhaseB)<=Flystart->Stillcurrthresh)&&\
                (_IQabs(MStruc_M.C3S.PhaseC)<=Flystart->Stillcurrthresh))
            {
                if((Flystart->Breakcnt <U16_MAX)&&((Flystart->Vefify_status==POSI_LOW)||\
                   (Flystart->Vefify_status==NEG_LOW)||(Flystart->Vefify_status==NEG_HIGH)))
                 Flystart->Breakcnt ++;

                if((Flystart->Stillcurrcnt <U16_MAX)&&(Flystart->Vefify_status==STILL))
                 Flystart->Stillcurrcnt ++;
            }
            switch (Flystart->Vefify_status)//enable PWM for braking first; start positioning and startup after braking
            {
               case STILL:
                   if(Flystart->Stillcurrcnt >=Flystart->Stillcurrcntset)
                   {
                       PID_Output_Init(&Mpid_M,Ramp_M->Direction);//PID regulator output initialization
                       Motor_Status_Initial_M(&MStruc_M);//motor status initialization
                       LBG_Status_Init(&MLBG_M,&LBG_Status_M,Ramp_M->Direction);//LBG observer initialization

                       MStruc_M.CRef2R.cd=DAXIS_CURR_REF_INIT1_M;
                       MStruc_M.CRef2R.cq=QAXIS_CURR_REF_INIT1_M*Ramp_M->Direction;
                       MLBG_M.Cal_Struc.Inimark=TRUE;
                       RunningStatus_M = RUN;

                   }
                   break;
               case POSI_LOW:
               case NEG_HIGH:
               case NEG_LOW:
                    if(Flystart->Breakcnt >=Flystart->Breakcntset)
                    {
                        PID_Output_Init(&Mpid_M,Ramp_M->Direction);//PID regulator output initialization
                        Motor_Status_Initial_M(&MStruc_M);//motor status initialization
                        LBG_Status_Init(&MLBG_M,&LBG_Status_M,Ramp_M->Direction);//LBG observer initialization
                        MStruc_M.CRef2R.cd=DAXIS_CURR_REF_INIT1_M;
                        MStruc_M.CRef2R.cq=QAXIS_CURR_REF_INIT1_M*Ramp_M->Direction;
                        MLBG_M.Cal_Struc.Inimark=TRUE;
                        RunningStatus_M = RUN;


                   }
                    break;

             }
         }
         else if (Ramp_M->Direction ==-1)
          {
             if((_IQabs(MStruc_M.C3S.PhaseA)<=Flystart->Stillcurrthresh)&&\
                (_IQabs(MStruc_M.C3S.PhaseB)<=Flystart->Stillcurrthresh)&&\
                 (_IQabs(MStruc_M.C3S.PhaseC)<=Flystart->Stillcurrthresh))
             {
                 if((Flystart->Breakcnt <U16_MAX)&&((Flystart->Vefify_status==POSI_LOW)||\
                    (Flystart->Vefify_status==NEG_LOW)||(Flystart->Vefify_status==POSI_HIGH)))
                  Flystart->Breakcnt ++;

                 if((Flystart->Stillcurrcnt <U16_MAX)&&(Flystart->Vefify_status==STILL))
                  Flystart->Stillcurrcnt ++;
             }
             switch (Flystart->Vefify_status)//enable PWM for braking first; start positioning and startup after braking
             {
                case STILL:
                    if(Flystart->Stillcurrcnt >=Flystart->Stillcurrcntset)
                    {
                        PID_Output_Init(&Mpid_M,Ramp_M->Direction);//PID regulator output initialization
                        Motor_Status_Initial_M(&MStruc_M);//motor status initialization
                        LBG_Status_Init(&MLBG_M,&LBG_Status_M,Ramp_M->Direction);//LBG observer initialization
                        MStruc_M.CRef2R.cd=DAXIS_CURR_REF_INIT1_M;
                        MStruc_M.CRef2R.cq=QAXIS_CURR_REF_INIT1_M*Ramp_M->Direction;
                        MLBG_M.Cal_Struc.Inimark=TRUE;
                        RunningStatus_M = RUN;
                    }
                    break;
                case POSI_HIGH:
                case POSI_LOW:
                case NEG_LOW:
                     if(Flystart->Breakcnt >=Flystart->Breakcntset)
                     {
                         PID_Output_Init(&Mpid_M,Ramp_M->Direction);//PID regulator output initialization
                         Motor_Status_Initial_M(&MStruc_M);//motor status initialization
                         LBG_Status_Init(&MLBG_M,&LBG_Status_M,Ramp_M->Direction);//LBG observer initialization
                         MStruc_M.CRef2R.cd = DAXIS_CURR_REF_INIT1_M;
                         MStruc_M.CRef2R.cq = QAXIS_CURR_REF_INIT1_M*Ramp_M->Direction;
                         MLBG_M.Cal_Struc.Inimark=TRUE;
                         RunningStatus_M = RUN;
                     }
                     break;
              }
          }
       }
}