/*
 * libCH32L103_MOTOR.h
 *
 *  Created on: May 20, 2024
 *      Author: zcc
 */

#ifndef USBPD_LIBMOTOR_1V1_1_H_
#define USBPD_LIBMOTOR_1V1_1_H_

#include "ch32l103_usbpd.h"

typedef struct {
    union {
        u16 Data;
        struct {
            u16 MsgType:5;
            u16 PortDataRole:1;
            u16 SpecRevision:2;
            u16 PortPwrRole:1;
            u16 MsgID:3;
            u16 NDO:3;
            u16 Extended:1;
        };
    };
} st_Prot_Header;




typedef struct {
    union {
        u32 Data;
        struct {
            u32 MaxCurrent:10;
            u32 Current:10;
            u32 Reserved:2;
            u32 EPRMode:1;
            u32 UnchunkedExtended:1;
            u32 NoUSBSuspend:1;
            u32 USBComm:1;
            u32 CapbilityMismatch:1;
            u32 GiveBack:1;
            u32 ObjectPos:4;
        };
    };
} st_Request_Fixed;



typedef struct {

    void (*pDevChk)();
    void (*pUserUnattached)();      //Call the user program when the connection succeeds
    void (*pUserAttached)();        //Call the user program when the connection is removed

    u8 DevRole;     //Sink   Src   DRP   DRP.TrySink   DRP.TrySrc
    u8 DevStat;     //0:Sink   1:Src

    u8 ConnectStat;


    u16 Cnt;
    u16 TryCnt;
    u16 Timeout;

    uint16_t *PortCC;       //Port control register of the CC in use
    uint32_t GpioCC;        //GPIO of the CC in use

    uint16_t *PortVconn;        //Port control register of the CC in use
    uint32_t GpioVconn;     //GPIO of the CC in use

} st_PD_DEVICE;



typedef struct {

                //  SOP     SOP'    SOP"    HRST    CRST
    u8 RxSop;   //  01      10      11      10      11
    u8 TxSop;   //  0x00    0x50    0x44    0xFF    0x73
    u8 Rxfact;

    u8 TxMsgID;
    u8 TxEmarkMsgID;
    u8 RxMsgID;
    u8 RxEmarkMsgID;
    u8 RetryCnt;
    u8 PDExist;             //Do not check MsgID for the first packet message
    u8 PDVconnExist;
    u16 IdleCnt;                //PD Idle count



    st_Prot_Header Header;      //Default value of the current Header, including the DRP state

    u8 WaitTxGcrc;
    u8 WaitRxGcrc;

    u8 WaitMsgTx;       //Flag for waiting to send a message
    u16 MsgTxCnt;       //Message send timing in ms

    u8 WaitMsgRx;       //Flag for waiting to receive a message
    u16 MsgRxCnt;       //Message receive timeout in ms

    u8 VoltChanging;        //Voltage adjustment flag
    u8 tSrcTransition;      //Waiting to start voltage adjustment    25-35
    u16 tPSTransition;      //Voltage adjustment phase      450-550

    void (*pRxFinish)();        //Receive success: the message is received and GoodCRC has been replied
    void (*pRxTimeout)();       //Receive timeout: the message was not received within the timing time, and the type of the received message is not determined
    void (*pTxFinish)();        //Send success: the message was sent successfully and GoodCRC has been received
    void (*pTxTimeout)();       //Send timeout: the correct GoodCRC was not received within the retry count
    void (*pRxHRST)();          //HRST received, and HRST sent

    u8 RxFinish;
    u8 RxHRST;
    u8 TxFinish;


    u32 rxSrcCap[7];
    u8  rxSrcCapCnt;
    st_Request_Fixed savedRequest;

    u8 RequestType;
    u8 PPS_success;
} st_PD_PHY;


extern st_PD_PHY PD_PHY;
extern st_PD_DEVICE PD_DEVICE;



void pProt_TX_Request(void);
void PD_Lib_Check_Version(void);
void Set_DevChk(u8 Role);
void PD_Init(void);
void pProt_TX_PS_RDY(void);
void PD_TX_GCRC(void);
void PD_TX_MSG(void);

#endif /* USBPD_LIBMOTOR_1V1_1_H_ */
