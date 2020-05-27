#include "STD_Types.h"
#include "USART_interface.h"
#include "LIN.h"

static u8 signal4[SIGNAL4_LENGTH]={0,1,2};
static u8 signal5[SIGNAL5_LENGTH]={3,4};
static u8 signal6[SIGNAL6_LENGTH]={5};
static u8 signal7[SIGNAL7_LENGTH]={6,7,8,9,10,11,12};

u16 scalerSignals[NUMBER_OF_SCALER_SIGNALS]={1,2,3,4};
u8* const byteArraySignals[NUMBER_OF_BYTE_ARRAY_SIGNALS]={signal4,signal5,signal6,signal7};

static const LIN_signalConfigType frame_0_0_signals[NUMBER_OF_FRAME_0_0_SIGNALS]=
{
		{
				.ID=		SIGNAL0_ID,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	1,
				.offset=	24
		},
		{
				.ID=		SIGNAL4_ID,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	SIGNAL4_LENGTH,
				.offset=	0
		}
};

static const LIN_signalConfigType frame_0_1_signals[NUMBER_OF_FRAME_0_1_SIGNALS]=
{
		{
				.ID=		SIGNAL1_ID,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	2,
				.offset=	16
		},
		{
				.ID=		SIGNAL5_ID,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	SIGNAL5_LENGTH,
				.offset=	0
		}
};

static const LIN_signalConfigType frame_1_0_signals[NUMBER_OF_FRAME_1_0_SIGNALS]=
{
		{
				.ID=		SIGNAL2_ID,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	2,
				.offset=	8
		},
		{
				.ID=		SIGNAL6_ID,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	SIGNAL6_LENGTH,
				.offset=	0
		}
};

static const LIN_signalConfigType frame_1_1_signals[NUMBER_OF_FRAME_1_1_SIGNALS]=
{
		{
				.ID=		SIGNAL3_ID,
				.type=		LIN_SIGNAL_TYPE_SCALER,
				.length=	3,
				.offset=	0
		},
		{
				.ID=		SIGNAL7_ID,
				.type=		LIN_SIGNAL_TYPE_BYTE_ARRAY,
				.length=	SIGNAL7_LENGTH,
				.offset=	1
		}
};

static const LIN_frameConfigType cluster0Frames[NUMBER_OF_CLUSTER_0_FRAMES]=
{
		{
				.signals=				frame_0_0_signals,
				.periodicity=			0,
				.ID=					0,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		NUMBER_OF_FRAME_0_0_SIGNALS,
				.frameSlotTimeTicks=	FRAME_0_0_TIME_SLOT,
				.frameSize=				FRAME_0_0_SIZE,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			0
		},
		{
				.signals=				frame_0_1_signals,
				.periodicity=			0,
				.ID=					1,
				.type=					LIN_FRAME_TYPE_EVENT_TRIG,
				.numberOfSignals=		NUMBER_OF_FRAME_0_1_SIGNALS,
				.frameSlotTimeTicks=	FRAME_0_1_TIME_SLOT,
				.frameSize=				FRAME_0_1_SIZE,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			0
		}
};

static const LIN_frameConfigType cluster1Frames[NUMBER_OF_CLUSTER_1_FRAMES]=
{
		{
				.signals=				frame_1_0_signals,
				.periodicity=			0,
				.ID=					2,
				.type=					LIN_FRAME_TYPE_UNCONDITIONAL,
				.numberOfSignals=		NUMBER_OF_FRAME_1_0_SIGNALS,
				.frameSlotTimeTicks=	FRAME_1_0_TIME_SLOT,
				.frameSize=				FRAME_1_0_SIZE,
				.direction=				LIN_FRAME_DIRECTION_SUBSCRIBER,
				.firstDelay=			0
		},
		{
				.signals=				frame_1_1_signals,
				.periodicity=			0,
				.ID=					3,
				.type=					LIN_FRAME_TYPE_SPORADIC,
				.numberOfSignals=		NUMBER_OF_FRAME_1_1_SIGNALS,
				.frameSlotTimeTicks=	FRAME_1_1_TIME_SLOT,
				.frameSize=				FRAME_1_1_SIZE,
				.direction=				LIN_FRAME_DIRECTION_PUBLISHER,
				.firstDelay=			0
		}
};

const LIN_clusterConfigType clusters[NUMBER_OF_CLUSTERS]=
{
		{
				.frames=			cluster0Frames,
				.bitRate=			USART_BAUDRATE_19200,
				.UART_ID=			USART_ID_USART1,
				.UART_asyncMode=	USART_DMA_ASYNC_MODE,
				.masterTaskControl=	LIN_CLUSTER_MASTER_TASK_CONTROL_ENABLE,
				.numberOfFrames=	NUMBER_OF_CLUSTER_0_FRAMES
		},
		{
				.frames=			cluster1Frames,
				.bitRate=			USART_BAUDRATE_19200,
				.UART_ID=			USART_ID_USART3,
				.UART_asyncMode=	USART_INTERRUPT_ASYNC_MODE,
				.masterTaskControl=	LIN_CLUSTER_MASTER_TASK_CONTROL_DISABLE,
				.numberOfFrames=	NUMBER_OF_CLUSTER_1_FRAMES
		}
};
