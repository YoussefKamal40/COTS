#include "STD_Types.h"
#include "USART_interface.h"
#include "LIN.h"

/*
	Mo ID4 uncond 1 byte bi2->sL0 bi3~bi4->sL1
	Mi ID30 uncond 4 byte bi0~15->mL2 bi16~32->mL3
	Mo ID23 sporadic 4 byte by0~2->mSw0sL4 by3->mSw1sL5
	Mi ID40 eventTrig 5 byte by0->sSw2mL6 by1~4->sSw3mL7
*/

/*
 * THeader_Nominal= 34 * TBit
 * TResponse_Nominal= 10 * (NData + 1) * TBit
 * TFrame_Nominal= THeader_Nominal + TResponse_Nominal
 * TFrame_Nominal= (34+10*(NData+1))*TBit
 * TFrame_Maximum=1.4*((34+10*(NData+1))*TBit)
 * TFrame_Maximum=1.4*((34+10*(NData+1))/BIT_RATE)
 * TFrame_Slot > jitter + TFrame_Maximum
 * */

static u8 mSw0Signal[3];
static u8 mSw1Signal[1];
static u8 mSw2Signal[1];
static u8 mSw3Signal[4];
static u8 sSw0Signal[3];
static u8 sSw1Signal[1];
static u8 sSw2Signal[1];
static u8 sSw3Signal[4];

u16 scalerSignals[NUMBER_OF_SCALER_SIGNALS]=
{
		mSw0Signal,
		mSw1Signal,
		mSw2Signal,
		mSw3Signal,
		sSw0Signal,
		sSw1Signal,
		sSw2Signal,
		sSw3Signal
};

u8* const byteArraySignals[NUMBER_OF_BYTE_ARRAY_SIGNALS]={};

static const LIN_signalConfigType mSw2Sw3Signals[2]=
{
		{
				.ID=		SIGNAL_SW2_M,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	1,
				.offset=	0
		},
		{
				.ID=		SIGNAL_SW3_M,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	4,
				.offset=	1
		}
};

static const LIN_signalConfigType sSw2Sw3Signals[2]=
{
		{
				.ID=		SIGNAL_SW2_S,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	1,
				.offset=	0
		},
		{
				.ID=		SIGNAL_SW3_S,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	4,
				.offset=	1
		}
};

static const LIN_signalConfigType mSw0Sw1Signals[2]=
{
		{
				.ID=		SIGNAL_SW0_M,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	3,
				.offset=	0
		},
		{
				.ID=		SIGNAL_SW1_M,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	1,
				.offset=	3
		}
};

static const LIN_signalConfigType sSw0Sw1Signals[2]=
{
		{
				.ID=		SIGNAL_SW0_S,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	3,
				.offset=	0
		},
		{
				.ID=		SIGNAL_SW1_S,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	1,
				.offset=	3
		}
};

static const LIN_signalConfigType sL2L3Signals[2]=
{
		{
				.ID=		SIGNAL_LED2_S,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	16,
				.offset=	0
		},
		{
				.ID=		SIGNAL_LED3_S,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	16,
				.offset=	16
		}
};

static const LIN_signalConfigType mL2L3Signals[2]=
{
		{
				.ID=		SIGNAL_LED2_M,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	16,
				.offset=	0
		},
		{
				.ID=		SIGNAL_LED3_M,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	16,
				.offset=	16
		}
};

static const LIN_signalConfigType mL0L1Signals[2]=
{
		{
				.ID=		SIGNAL_LED0_M,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	1,
				.offset=	2
		},
		{
				.ID=		SIGNAL_LED1_M,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	2,
				.offset=	3
		}
};

static const LIN_signalConfigType sL0L1Signals[2]=
{
		{
				.ID=		SIGNAL_LED0_S,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	1,
				.offset=	2
		},
		{
				.ID=		SIGNAL_LED1_S,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	2,
				.offset=	3
		}
};

static const LIN_frameConfigType cluster0Frames[4]=
{
		{/*output frame for led0 and led1 ID=4*/
				.signals=				mL0L1Signals,
				.periodicity=			10,
				.ID=					4,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*3.9375 + jitter=5ms*/
				.frameSize=				1,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			0
		},
		{/*input frame for led2 and led3 ID=30*/
				.signals=				mL2L3Signals,
				.periodicity=			25,
				.ID=					30,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.125 + jitter = 7ms*/
				.frameSize=				4,
				.direction=				LIN_FRAME_DIRECTION_SUBSCRIBER,
				.firstDelay=			2
		},
		{/*output frame for sw0 and sw1*/
				.signals=				mSw0Sw1Signals,
				.periodicity=			5,
				.ID=					23,
				.type=					LIN_FRAME_TYPE_SPORADIC,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.125 + jitter = 7ms*/
				.frameSize=				4,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			4
		},
		{/*input frame for sw2 and sw3*/
				.signals=				mSw2Sw3Signals,
				.periodicity=			5,
				.ID=					40,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.854 + jitter = 8ms*/
				.frameSize=				5,
				.direction=				LIN_FRAME_DIRECTION_SUBSCRIBER,
				.firstDelay=			4
		}
};

static const LIN_frameConfigType cluster1Frames[4]=
{
		{/*input frame for led0 and led1 ID=4*/
				.signals=				sL0L1Signals,
				.periodicity=			10,
				.ID=					4,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*3.9375 + jitter = 5ms*/
				.frameSize=				1,
				.direction=				LIN_FRAME_DIRECTION_SUBSCRIBER,
				.firstDelay=			0
		},
		{/*output frame for led2 and led3 ID=30*/
				.signals=				sL2L3Signals,
				.periodicity=			25,
				.ID=					30,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.125 + jitter = 7ms*/
				.frameSize=				4,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			2
		},
		{/*input frame for sw0 and sw1*/
				.signals=				sSw0Sw1Signals,
				.periodicity=			5,
				.ID=					23,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.125 + jitter = 7ms*/
				.frameSize=				4,
				.direction=				LIN_FRAME_DIRECTION_SUBSCRIBER,
				.firstDelay=			4
		},
		{/*output frame for sw2 and sw3*/
				.signals=				sSw2Sw3Signals,
				.periodicity=			5,
				.ID=					40,
				.type=					LIN_FRAME_TYPE_EVENT_TRIG,
				.numberOfSignals=		2,
				.frameSlotTimeTicks=	1,/*6.854 + jitter = 8ms*/
				.frameSize=				5,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			4
		}
};

const LIN_clusterConfigType clusters[NUMBER_OF_CLUSTERS]=
{
		{/*Master Node*/
				.frames=			cluster0Frames,
				.bitRate=			USART_BAUDRATE_19200,
				.UART_ID=			USART_ID_USART1,
				.UART_asyncMode=	USART_INTERRUPT_ASYNC_MODE,
				.masterTaskControl=	LIN_CLUSTER_MASTER_TASK_CONTROL_ENABLE,
				.numberOfFrames=	4
		},
		{/*Slave Node*/
				.frames=			cluster1Frames,
				.bitRate=			USART_BAUDRATE_19200,
				.UART_ID=			USART_ID_USART3,
				.UART_asyncMode=	USART_INTERRUPT_ASYNC_MODE,
				.masterTaskControl=	LIN_CLUSTER_MASTER_TASK_CONTROL_DISABLE,
				.numberOfFrames=	4
		}
};
